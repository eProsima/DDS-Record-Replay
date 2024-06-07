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

#include <gtest/gtest.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <cpp_utils/ros2_mangling.hpp>

#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>
#include <ddspipe_yaml/Yaml.hpp>

#include <ddsrecorder_yaml/recorder/YamlReaderConfiguration.hpp>

#if FASTRTPS_VERSION_MAJOR < 2 || (FASTRTPS_VERSION_MAJOR == 2 && FASTRTPS_VERSION_MINOR < 13)
    #include "../resources/types/hello_world/v1/HelloWorld.h"
    #include "../resources/types/hello_world/v1/HelloWorldPubSubTypes.h"
#else
    #include "../resources/types/hello_world/v2/HelloWorld.h"
    #include "../resources/types/hello_world/v2/HelloWorldPubSubTypes.h"
#endif // if FASTRTPS_VERSION_MAJOR < 2 || (FASTRTPS_VERSION_MAJOR == 2 && FASTRTPS_VERSION_MINOR < 13)

#include <tool/DdsRecorder.hpp>

#include "constants.hpp"

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

class FileCreationTest : public testing::Test
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
        fastdds::dds::TypeSupport type(new HelloWorldPubSubType());
        type.register_type(participant_);

        // Create the publisher
        publisher_ = participant_->create_publisher(fastdds::dds::PUBLISHER_QOS_DEFAULT, nullptr);

        ASSERT_NE(publisher_, nullptr);

        // Create the RecorderConfiguration
        Yaml yml;
        configuration_ = std::make_unique<ddsrecorder::yaml::RecorderConfiguration>(yml);
        configuration_->simple_configuration->domain = test::DOMAIN;
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

protected:

    std::vector<std::shared_ptr<fastrtps::rtps::SerializedPayload_t>> record_messages_(
            const std::string& file_name,
            const unsigned int messages1,
            const DdsRecorderState state1 = DdsRecorderState::RUNNING,
            const unsigned int messages2 = 0,
            const DdsRecorderState state2 = DdsRecorderState::RUNNING,
            const unsigned int wait = 0,
            const EventKind event = EventKind::NO_EVENT)
    {
        // Create the Recorder
        std::shared_ptr<ddsrecorder::participants::FileTracker> file_tracker;
        auto recorder = std::make_unique<ddsrecorder::recorder::DdsRecorder>(*configuration_, state1, file_tracker, file_name);

        // Create the topic
        const auto topic_name = (configuration_->ros2_types) ? test::ROS2_TOPIC_NAME : test::TOPIC_NAME;
        topic_ = participant_->create_topic(topic_name, "HelloWorld", fastdds::dds::TOPIC_QOS_DEFAULT);

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
        sent_messages.insert(sent_messages.end(), sent_messages_after_transition.begin(), sent_messages_after_transition.end());

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

    std::vector<std::shared_ptr<fastrtps::rtps::SerializedPayload_t>> send_messages_(
            const unsigned int number_of_messages)
    {
        // Create the DataWriter
        create_datawriter_();

        // Wait for the DataReader to match the DataWriter
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Send the messages
        std::vector<std::shared_ptr<fastrtps::rtps::SerializedPayload_t>> sent_messages;

        for (std::uint32_t i = 0; i < number_of_messages; i++)
        {
            // Create the message
            HelloWorld hello;
            hello.index(i);

            // Send the message
            writer_->write(&hello);

            // Serialize the message
            HelloWorldPubSubType pubsubType;
            const auto payload_size = pubsubType.getSerializedSizeProvider(&hello)();
            auto payload = std::make_shared<fastrtps::rtps::SerializedPayload_t>(payload_size);
            pubsubType.serialize(&hello, payload.get());

            // Store the serialized message
            sent_messages.push_back(payload);

            // Wait for the message to be sent
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // Delete the DataWriter
        delete_datawriter_();

        return sent_messages;
    }

    void create_datawriter_()
    {
        // Configure the DataWriter's QoS to ensure that the DDS Recorder receives all the msgs
        fastdds::dds::DataWriterQos wqos = fastdds::dds::DATAWRITER_QOS_DEFAULT;
        wqos.reliability().kind = fastdds::dds::RELIABLE_RELIABILITY_QOS;
        wqos.durability().kind = fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
        wqos.history().kind = fastdds::dds::KEEP_ALL_HISTORY_QOS;

        // Create the writer
        writer_ = publisher_->create_datawriter(topic_, wqos);

        ASSERT_NE(writer_, nullptr);
    }

    void delete_datawriter_()
    {
        if (writer_ != nullptr)
        {
            publisher_->delete_datawriter(writer_);
        }
    }

    std::filesystem::path get_output_file_path_(
            const std::string& output_file_name)
    {
        const auto file_path = std::filesystem::current_path() / output_file_name;

        paths_.push_back(file_path);

        return file_path;
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
    fastdds::dds::Publisher* publisher_ = nullptr;
    fastdds::dds::Topic* topic_ = nullptr;
    fastdds::dds::DataWriter* writer_ = nullptr;

    std::vector<std::filesystem::path> paths_;

    std::unique_ptr<ddsrecorder::yaml::RecorderConfiguration> configuration_;
};
