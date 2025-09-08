// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include <cpp_utils/ros2_mangling.hpp>

#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>
#include <ddspipe_yaml/Yaml.hpp>

#include <ddsrecorder_yaml/recorder/YamlReaderConfiguration.hpp>

#include <tool/DdsRecorder.hpp>

#include "constants.hpp"

#include "../resources/types/hello_world/HelloWorld.hpp"
#include "../resources/types/hello_world/HelloWorldPubSubTypes.hpp"

using namespace eprosima;
using DdsRecorderState = ddsrecorder::recorder::DdsRecorderStateCode;

enum class EventKind
{
    NO_EVENT,
    EVENT,
    EVENT_START,
    EVENT_STOP,
    EVENT_SUSPEND,
};

class FileCreationTest : public testing::Test,
    public fastdds::dds::DataWriterListener
{
public:

    void SetUp() override
    {
        // Create the participant
        fastdds::dds::DomainParticipantQos pqos;
        pqos.name(test::PARTICIPANT_ID);

        participant_ = fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(test::DOMAIN, pqos);

        ASSERT_NE(participant_, nullptr);

        // Register the type
        type_support_ = fastdds::dds::TypeSupport(new HelloWorldPubSubType());
        participant_->register_type(type_support_);

        // Create the publisher
        publisher_ = participant_->create_publisher(fastdds::dds::PUBLISHER_QOS_DEFAULT, nullptr);

        ASSERT_NE(publisher_, nullptr);

        // Create the RecorderConfiguration
        Yaml yml;
        configuration_ = std::make_unique<ddsrecorder::yaml::RecorderConfiguration>(yml);
        configuration_->dds_configuration->domain = test::DOMAIN;

        // Create the topic
        create_topic_();

        // Create the DataWriter
        create_datawriter_();
    }

    void TearDown() override
    {
        // Delete the participant
        if (participant_ != nullptr)
        {
            participant_->delete_contained_entities();
            fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
        }

        // Remove the output files
        for (const auto& path : paths_)
        {
            delete_file_(path);
        }
    }

    void on_publication_matched(
            fastdds::dds::DataWriter* /*writer*/,
            const fastdds::dds::PublicationMatchedStatus& info) override
    {
        if (info.current_count_change > 0)
        {
            {
                std::lock_guard<std::mutex> lock(mtx_);
                matched_ = true;
            }
            cv_.notify_one();
        }
        else if (info.current_count == 0)
        {
            {
                std::lock_guard<std::mutex> lock(mtx_);
                matched_ = false;
            }
        }
    }

protected:

    std::vector<HelloWorld> record_messages_(
            const std::string& file_name,
            const unsigned int messages1,
            const DdsRecorderState state1 = DdsRecorderState::RUNNING,
            const unsigned int messages2 = 0,
            const DdsRecorderState state2 = DdsRecorderState::RUNNING,
            const unsigned int wait = 0,
            const EventKind event = EventKind::NO_EVENT)
    {
        // Create the Recorder
        auto recorder = std::make_unique<ddsrecorder::recorder::DdsRecorder>(*configuration_, state1, file_name);

        // Send messages
        auto sent_messages = send_messages_(messages1);

        if (state1 != state2)
        {
            // Change the Recorder's state
            switch (state2)
            {
                case DdsRecorderState::RUNNING:
                    recorder->start();
                    break;
                case DdsRecorderState::PAUSED:
                    recorder->pause();
                    break;
                case DdsRecorderState::SUSPENDED:
                    recorder->suspend();
                    break;
                case DdsRecorderState::STOPPED:
                    recorder->stop();
                    break;
                default:
                    break;
            }
        }

        // Wait for the event window
        std::this_thread::sleep_for(std::chrono::seconds(wait));

        // Send more messages
        const auto sent_messages_after_transition = send_messages_(messages2);
        sent_messages.insert(sent_messages.end(),
                sent_messages_after_transition.begin(), sent_messages_after_transition.end());

        if (event != EventKind::NO_EVENT && state2 == DdsRecorderState::PAUSED)
        {
            recorder->trigger_event();

            switch (event)
            {
                case EventKind::EVENT_START:
                    recorder->start();
                    break;

                case EventKind::EVENT_SUSPEND:
                    recorder->suspend();
                    break;

                case EventKind::EVENT_STOP:
                    recorder->stop();
                    break;

                default:
                    break;
            }
        }

        return sent_messages;
    }

    std::vector<HelloWorld> send_messages_(
            const unsigned int number_of_messages)
    {
        // Create the DataWriter
        create_datawriter_();

        // Wait for the DataWriter to match the DataReader
        wait_for_matching();

        // Send the messages
        std::vector<HelloWorld> sent_messages;

        for (std::uint32_t i = 0; i < number_of_messages; i++)
        {
            // Create the message
            HelloWorld hello;
            hello.index(i);
            hello.message("Hello World!");

            // Send the message
            writer_->write(&hello);

            // Store the message
            sent_messages.push_back(hello);

            // Wait for the message to be sent
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // Delete the DataWriter
        delete_datawriter_();

        return sent_messages;
    }

    std::shared_ptr<fastdds::rtps::SerializedPayload_t> to_cdr(
            const HelloWorld& message)
    {
        HelloWorldPubSubType pubsubType;
        const auto payload_size = pubsubType.calculate_serialized_size(&message,
                        fastdds::dds::DEFAULT_DATA_REPRESENTATION);
        auto payload = std::make_shared<fastdds::rtps::SerializedPayload_t>(payload_size);
        pubsubType.serialize(&message, *payload, fastdds::dds::DEFAULT_DATA_REPRESENTATION);

        return payload;
    }

    std::string to_json(
            const HelloWorld& message)
    {
        // Get type object
        fastdds::dds::xtypes::TypeObjectPair type_objects;
        fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
            type_support_->get_name(),
            type_objects);

        // Build dynamic type
        fastdds::dds::DynamicType::_ref_type dyn_type =
                fastdds::dds::DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
            type_objects.complete_type_object)->build();

        // Build dynamic data
        fastdds::dds::DynamicData::_ref_type dyn_data =
                fastdds::dds::DynamicDataFactory::get_instance()->create_data(dyn_type);

        // Transform the message into DynamicData
        const auto payload_size = type_support_->calculate_serialized_size(&message,
                        fastdds::dds::DEFAULT_DATA_REPRESENTATION);
        auto payload = std::make_shared<fastdds::rtps::SerializedPayload_t>(payload_size);
        type_support_->serialize(&message, *payload, fastdds::dds::DEFAULT_DATA_REPRESENTATION);

        fastdds::dds::TypeSupport dyn_type_support(new fastdds::dds::DynamicPubSubType(dyn_type));
        dyn_type_support->deserialize(*payload, &dyn_data);

        // Serialize DynamicType into its IDL representation
        std::stringstream data_json;
        json_serialize(dyn_data, fastdds::dds::DynamicDataJsonFormat::EPROSIMA, data_json);

        return data_json.str();
    }

    void recreate_datawriter_()
    {
        // Delete the existing DataWriter
        delete_datawriter_();

        // delete the topic
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }

        // Create a new topic
        create_topic_();

        // Create a new DataWriter
        create_datawriter_();
    }

    void create_topic_()
    {
        const auto topic_name = (configuration_->ros2_types) ? test::ROS2_TOPIC_NAME : test::TOPIC_NAME;
        topic_ = participant_->create_topic(topic_name, "HelloWorld", fastdds::dds::TOPIC_QOS_DEFAULT);
    }

    void create_datawriter_()
    {
        // Configure the DataWriter's QoS to ensure that the DDS Recorder receives all the msgs
        fastdds::dds::DataWriterQos wqos = fastdds::dds::DATAWRITER_QOS_DEFAULT;
        wqos.reliability().kind = fastdds::dds::RELIABLE_RELIABILITY_QOS;
        wqos.durability().kind = fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
        wqos.history().kind = fastdds::dds::KEEP_ALL_HISTORY_QOS;

        // Create the writer
        writer_ = publisher_->create_datawriter(topic_, wqos, this);

        ASSERT_NE(writer_, nullptr);
    }

    void delete_datawriter_()
    {
        if (writer_ != nullptr)
        {
            publisher_->delete_datawriter(writer_);
        }
    }

    void wait_for_matching(
            std::chrono::seconds timeout = std::chrono::seconds(2))
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait_for(lock, timeout, [this]()
                {
                    return matched_;
                });

        if (!matched_)
        {
            FAIL() << "DataWriter did not match any DataReader within the timeout.";
        }
    }

    std::string get_output_file_path_(
            const std::string& output_file_name)
    {
        const auto file_path = std::filesystem::current_path() / output_file_name;

        paths_.push_back(file_path);

        return file_path.string();
    }

    bool delete_file_(
            const std::filesystem::path& file_path)
    {
        if (std::filesystem::exists(file_path) && !std::filesystem::remove(file_path))
        {
            return false;
        }

        const auto file_path_tmp = file_path.string() + ".tmp~";

        if (std::filesystem::exists(file_path_tmp) && !std::filesystem::remove(file_path_tmp))
        {
            return false;
        }

        return true;
    }

    fastdds::dds::DomainParticipant* participant_ = nullptr;
    fastdds::dds::TypeSupport type_support_;
    fastdds::dds::Publisher* publisher_ = nullptr;
    fastdds::dds::Topic* topic_ = nullptr;
    fastdds::dds::DataWriter* writer_ = nullptr;

    std::vector<std::filesystem::path> paths_;

    std::unique_ptr<ddsrecorder::yaml::RecorderConfiguration> configuration_;

    bool matched_{false};
    std::mutex mtx_;
    std::condition_variable cv_;
};
