// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp>
#include <fastdds/rtps/writer/WriterDiscoveryStatus.hpp>
#include <ddspipe_yaml/Yaml.hpp>

#include <ddsrecorder_yaml/replayer/YamlReaderConfiguration.hpp>

#include <tool/DdsReplayer.hpp>

#include "../../resources/constants.hpp"

using namespace eprosima;

namespace {

constexpr unsigned int MESSAGES_PER_PARTITION = 50;

/**
 * @brief Dynamic subscriber that counts samples received through one DDS partition.
 *
 * The supplied MCAP contains a HelloWorld type with four members. The existing statically typed
 * test type has a different schema, so this listener obtains the type from the replayer's type
 * information and creates a DynamicData reader.
 */
class PartitionSubscriber : public fastdds::dds::DomainParticipantListener
{
public:

    PartitionSubscriber(
            const std::string& partition,
            const std::string& topic_name,
            const std::uint32_t domain)
        : topic_name_(topic_name)
    {
        fastdds::dds::DomainParticipantQos participant_qos;
        participant_qos.name(("McapPartitionSubscriber_" + partition).c_str());

        fastdds::dds::StatusMask mask;
        mask << fastdds::dds::StatusMask::data_available();
        mask << fastdds::dds::StatusMask::subscription_matched();

        participant_ = fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
            domain, participant_qos, this, mask);
        if (participant_ == nullptr)
        {
            throw std::runtime_error("Failed to create partition subscriber participant");
        }

        fastdds::dds::SubscriberQos subscriber_qos = fastdds::dds::SUBSCRIBER_QOS_DEFAULT;
        subscriber_qos.partition().push_back(partition.c_str());
        subscriber_ = participant_->create_subscriber(subscriber_qos, nullptr);
        if (subscriber_ == nullptr)
        {
            throw std::runtime_error("Failed to create partition subscriber");
        }
    }

    ~PartitionSubscriber() override
    {
        if (participant_ != nullptr)
        {
            participant_->delete_contained_entities();
            fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
        }
    }

    void on_data_writer_discovery(
            fastdds::dds::DomainParticipant*,
            fastdds::rtps::WriterDiscoveryStatus,
            const fastdds::dds::PublicationBuiltinTopicData& info,
            bool&) override
    {
        {
            std::lock_guard<std::mutex> lock(reader_mutex_);
            if (info.topic_name.to_string() != topic_name_ || reader_ != nullptr)
            {
                return;
            }
        }

        const auto type_identifier = info.type_information.type_information.complete().typeid_with_size().type_id();
        fastdds::dds::xtypes::TypeObject type_object;
        if (fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
                    type_identifier, type_object) != fastdds::dds::RETCODE_OK)
        {
            return;
        }

        auto dynamic_type = fastdds::dds::DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
            type_object)->build();
        if (!dynamic_type)
        {
            return;
        }

        fastdds::dds::TypeSupport type(new fastdds::dds::DynamicPubSubType(dynamic_type));
        if (type.register_type(participant_) != fastdds::dds::RETCODE_OK)
        {
            return;
        }

        topic_ = participant_->create_topic(
            topic_name_, dynamic_type->get_name().to_string(), fastdds::dds::TOPIC_QOS_DEFAULT);
        if (topic_ == nullptr)
        {
            return;
        }

        fastdds::dds::DataReaderQos reader_qos = fastdds::dds::DATAREADER_QOS_DEFAULT;
        reader_qos.reliability().kind = fastdds::dds::RELIABLE_RELIABILITY_QOS;
        reader_qos.durability().kind = fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
        reader_qos.history().kind = fastdds::dds::KEEP_ALL_HISTORY_QOS;

        dynamic_type_ = dynamic_type;
        auto reader = subscriber_->create_datareader(topic_, reader_qos, this);
        {
            std::lock_guard<std::mutex> lock(reader_mutex_);
            reader_ = reader;
        }
        reader_cv_.notify_all();
    }

    void on_data_available(
            fastdds::dds::DataReader* reader) override
    {
        auto data = fastdds::dds::DynamicDataFactory::get_instance()->create_data(dynamic_type_);
        fastdds::dds::SampleInfo info;

        while (reader->take_next_sample(&data, &info) == fastdds::dds::RETCODE_OK)
        {
            if (!info.valid_data)
            {
                continue;
            }

            std::uint32_t index = 0;
            if (data->get_uint32_value(index, data->get_member_id_by_name("index")) ==
                    fastdds::dds::RETCODE_OK)
            {
                std::lock_guard<std::mutex> lock(messages_mutex_);
                indexes_.push_back(index);
                messages_cv_.notify_all();
            }
        }
    }

    bool wait_for_reader(
            const std::chrono::seconds timeout)
    {
        std::unique_lock<std::mutex> lock(reader_mutex_);
        return reader_cv_.wait_for(lock, timeout, [this]()
                       {
                           return reader_ != nullptr;
                       });
    }

    bool wait_for_messages(
            const std::size_t expected,
            const std::chrono::seconds timeout) const
    {
        std::unique_lock<std::mutex> lock(messages_mutex_);
        return messages_cv_.wait_for(lock, timeout, [this, expected]()
                       {
                           return indexes_.size() >= expected;
                       });
    }

    std::vector<std::uint32_t> indexes() const
    {
        std::lock_guard<std::mutex> lock(messages_mutex_);
        return indexes_;
    }

private:

    fastdds::dds::DomainParticipant* participant_{nullptr};
    fastdds::dds::Subscriber* subscriber_{nullptr};
    fastdds::dds::Topic* topic_{nullptr};
    fastdds::dds::DataReader* reader_{nullptr};
    fastdds::dds::DynamicType::_ref_type dynamic_type_;
    std::string topic_name_;

    mutable std::mutex reader_mutex_;
    mutable std::condition_variable reader_cv_;
    mutable std::mutex messages_mutex_;
    mutable std::condition_variable messages_cv_;
    std::vector<std::uint32_t> indexes_;
};

} // namespace

/**
 * Verify that the replayer changes the output partition when the MCAP metadata changes for the
 * same writer GUID.
 *
 * CASES:
 *  - Verify that the first 50 samples are received in partition A.
 *  - Verify that the following 50 samples are received in partition B.
 */
TEST(McapPartitionFileReadTest, partition_switch)
{
    Yaml yml;
    ddsrecorder::yaml::ReplayerConfiguration configuration(yml);
    configuration.replayer_configuration->domain.domain_id = test::DOMAIN;
    configuration.replay_types = true;

    const std::string input_file = "../../resources/recordings/basic/one_writer_change_partition.mcap";
    PartitionSubscriber partition_a("A", "HelloWorldTopic", test::DOMAIN);
    PartitionSubscriber partition_b("B", "HelloWorldTopic", test::DOMAIN);
    ddsrecorder::replayer::DdsReplayer replayer(configuration, input_file);

    std::thread replay_thread([&replayer]()
            {
                replayer.process_file();
            });

    const bool partition_a_reader_ready = partition_a.wait_for_reader(std::chrono::seconds(5));
    const bool partition_b_reader_ready = partition_b.wait_for_reader(std::chrono::seconds(5));
    replay_thread.join();
    replayer.stop();

    ASSERT_TRUE(partition_a_reader_ready);
    ASSERT_TRUE(partition_b_reader_ready);
    ASSERT_TRUE(partition_a.wait_for_messages(
                MESSAGES_PER_PARTITION, std::chrono::seconds(5)));
    ASSERT_TRUE(partition_b.wait_for_messages(
                MESSAGES_PER_PARTITION, std::chrono::seconds(5)));

    const auto partition_a_indexes = partition_a.indexes();
    const auto partition_b_indexes = partition_b.indexes();
    ASSERT_EQ(partition_a_indexes.size(), MESSAGES_PER_PARTITION);
    ASSERT_EQ(partition_b_indexes.size(), MESSAGES_PER_PARTITION);

    for (unsigned int i = 0; i < MESSAGES_PER_PARTITION; ++i)
    {
        EXPECT_EQ(partition_a_indexes[i], i + 1);
        EXPECT_EQ(partition_b_indexes[i], MESSAGES_PER_PARTITION + i + 1);
    }
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
