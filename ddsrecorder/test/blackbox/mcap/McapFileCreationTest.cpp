// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include <iostream>
#include <thread>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include <cpp_utils/ros2_mangling.hpp>

#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>
#include <ddsrecorder_yaml/recorder/yaml_configuration_tags.hpp>
#include <ddsrecorder_yaml/recorder/YamlReaderConfiguration.hpp>

#include <tool/DdsRecorder.hpp>

#include <mcap/reader.hpp>

#include "../../resources/types/hello_world/HelloWorld.hpp"
#include "../../resources/types/hello_world/HelloWorldPubSubTypes.hpp"
#include "../../resources/types/hello_world/HelloWorldTypeObjectSupport.hpp"

using namespace eprosima::ddspipe;
using namespace eprosima::ddsrecorder;
using namespace eprosima::ddsrecorder::recorder;
using namespace eprosima::fastdds::dds;

using DdsRecorderState = eprosima::ddsrecorder::recorder::DdsRecorderStateCode;

enum class DataTypeKind
{
    HELLO_WORLD,
};

enum class EventKind
{
    NO_EVENT,
    EVENT,
    EVENT_START,
    EVENT_STOP,
    EVENT_SUSPEND,
};

namespace test {

// Publisher

const unsigned int DOMAIN = 222;

const std::string dds_topic_name = "TypeIntrospectionTopic";
const std::string dds_type_name = "HelloWorld";

const std::string ros2_topic_name = "rt/hello";
// Note: The ros2_type_name will not be used, as it's not possible to set the type name
// In previous versions, it was possible to set it directly, but now generating a new
// type just to change the name would be overkill.
const std::string ros2_type_name = "std_msgs::msg::dds_::String_";

const unsigned int n_msgs = 3;
const std::string send_message = "Hello World";
const unsigned int index = 6;
const unsigned int downsampling = 3;

eprosima::fastdds::dds::DataWriter* writer_;
eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicType>::ref_type dynamic_type_;

} // test

std::unique_ptr<DdsRecorder> create_recorder(
        const std::string file_name,
        const int downsampling,
        DdsRecorderState recorder_state = DdsRecorderState::RUNNING,
        const unsigned int event_window = 20,
        const bool ros2_types = false)
{
    YAML::Node yml;

    eprosima::ddsrecorder::yaml::RecorderConfiguration configuration(yml);
    configuration.topic_qos.downsampling = downsampling;
    // Set default value for downsampling
    // TODO: Change mechanism setting topic qos' default values from specs
    eprosima::ddspipe::core::types::TopicQoS::default_topic_qos.set_value(configuration.topic_qos);
    configuration.event_window = event_window;
    eprosima::ddspipe::core::types::DomainId domainId;
    domainId.domain_id = test::DOMAIN;
    configuration.simple_configuration->domain = domainId;
    configuration.ros2_types = ros2_types;

    std::shared_ptr<eprosima::ddsrecorder::participants::FileTracker> file_tracker;

    return std::make_unique<DdsRecorder>(
        configuration,
        recorder_state,
        file_tracker,
        file_name);
}

void create_publisher(
        const std::string topic_name,
        const std::string type_name,
        const unsigned int domain)
{
    eprosima::fastdds::dds::DomainParticipantQos pqos;
    pqos.name("TypeIntrospectionExample_Participant_Publisher");

    // Create the Participant
    eprosima::fastdds::dds::DomainParticipant* participant_ =
            DomainParticipantFactory::get_instance()->create_participant(domain, pqos);

    // Register the type
    // NOTE: the sheer use of this type support is to register the type object representation
    eprosima::fastdds::dds::TypeSupport type(new HelloWorldPubSubType());
    type->register_type_object_representation();

    eprosima::fastdds::dds::xtypes::TypeObjectPair dyn_type_objects;
    if (eprosima::fastdds::dds::RETCODE_OK !=
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
                type_name,
                dyn_type_objects))
    {
        EPROSIMA_LOG_WARNING(DDSRECORDER_TEST, "Failed to get TypeObjects for type with name " << type_name);
        return;
    }

    test::dynamic_type_ = eprosima::fastdds::dds::DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
        dyn_type_objects.complete_type_object)->build();

    // Create the Publisher
    eprosima::fastdds::dds::Publisher* publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

    // Register the (dynamic) type support in the Participant
    eprosima::fastdds::dds::TypeSupport dyn_type_support(new eprosima::fastdds::dds::DynamicPubSubType(test::
                    dynamic_type_));
    participant_->register_type(dyn_type_support);

    // Create the DDS Topic
    eprosima::fastdds::dds::Topic* topic_ = participant_->create_topic(topic_name, type_name,
                    TOPIC_QOS_DEFAULT);

    // Create the DDS DataWriter
    test::writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, nullptr);
}

eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicData>::ref_type send_sample(
        const unsigned int index = 1,
        const unsigned int time_sleep = 100)
{
    if (test::dynamic_type_ == nullptr)
    {
        EPROSIMA_LOG_WARNING(DDSRECORDER_TEST, "Dynamic type is null. Cannot send sample of index: " << index);
        return nullptr;
    }

    // Create and initialize new dynamic data
    auto dynamic_data_ = eprosima::fastdds::dds::DynamicDataFactory::get_instance()->create_data(test::dynamic_type_);

    // Set index
    dynamic_data_->set_uint32_value(dynamic_data_->get_member_id_by_name("index"), index);
    // Set message
    dynamic_data_->set_string_value(dynamic_data_->get_member_id_by_name("message"), test::send_message);
    test::writer_->write(&dynamic_data_);

    EPROSIMA_LOG_INFO(DDSRECORDER_TEST, "Message published.");

    std::this_thread::sleep_for(std::chrono::milliseconds(time_sleep));

    return dynamic_data_;
}

eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicData>::ref_type record(
        const std::string file_name,
        const unsigned int num_msgs = 1,
        const unsigned int downsampling = 1,
        const bool ros2_types = false)
{
    eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicData>::ref_type send_data;
    {
        // Create Recorder
        auto recorder = create_recorder(file_name, downsampling, DdsRecorderState::RUNNING, 20, ros2_types);

        // Create Publisher
        ros2_types ? create_publisher(test::ros2_topic_name, test::dds_type_name, test::DOMAIN) : create_publisher(
            test::dds_topic_name, test::dds_type_name, test::DOMAIN);

        // Send data
        for (unsigned int i = 0; i < num_msgs; i++)
        {
            send_data = send_sample(test::index);
        }
    }

    return send_data;
}

mcap::LinearMessageView get_msgs_mcap(
        const std::string file_name,
        mcap::McapReader& mcap_reader_)
{
    auto status = mcap_reader_.open(file_name + ".mcap");

    auto messages = mcap_reader_.readMessages();

    return messages;
}

std::tuple<unsigned int, double> record_with_transitions(
        const std::string file_name,
        DdsRecorderState init_state,
        const unsigned int first_round,
        const unsigned int secound_round,
        DdsRecorderState current_state,
        EventKind event = EventKind::NO_EVENT,
        const unsigned int event_window = 20,
        unsigned int time_sleep = 0,
        const unsigned int downsampling = 1,
        const bool ros2_types = false)
{
    uint64_t current_time;
    {
        // Create Publisher
        ros2_types ? create_publisher(test::ros2_topic_name, test::ros2_type_name, test::DOMAIN) : create_publisher(
            test::dds_topic_name, test::dds_type_name, test::DOMAIN);

        // Create Recorder
        std::unique_ptr<DdsRecorder> recorder =
                create_recorder(file_name, downsampling, init_state, event_window, ros2_types);

        // Send data
        for (unsigned int i = 0; i < first_round; i++)
        {
            send_sample();
        }

        if (init_state != current_state)
        {
            switch (current_state)
            {
                case DdsRecorderState::RUNNING:
                    recorder->start();
                    break;
                case DdsRecorderState::SUSPENDED:
                    recorder->suspend();
                    break;
                case DdsRecorderState::STOPPED:
                    recorder->stop();
                    break;
                case DdsRecorderState::PAUSED:
                    recorder->pause();
                    break;
                default:
                    break;
            }
        }

        if (!time_sleep)
        {
            time_sleep = rand() % 2;
        }
        std::this_thread::sleep_for(std::chrono::seconds(time_sleep));

        for (unsigned int i = 0; i < secound_round; i++)
        {
            send_sample();
        }

        current_time = std::chrono::duration_cast<std::chrono::nanoseconds>
                    (std::chrono::system_clock::now().time_since_epoch()).count();

        if (event != EventKind::NO_EVENT && current_state == DdsRecorderState::PAUSED)
        {
            recorder->trigger_event();
            if (event == EventKind::EVENT_START)
            {
                recorder->start();
            }
            else if (event == EventKind::EVENT_STOP)
            {
                recorder->stop();
            }
            else if (event == EventKind::EVENT_SUSPEND)
            {
                recorder->suspend();
            }
        }
    }

    mcap::McapReader mcap_reader;
    auto messages = get_msgs_mcap(file_name, mcap_reader);

    unsigned int n_received_msgs = 0;
    double max_timestamp = 0;
    for (auto it = messages.begin(); it != messages.end(); it++)
    {
        n_received_msgs++;
        double time_seconds = ((current_time) - (it->message.logTime)) * pow(10.0, -9.0);
        if (time_seconds > max_timestamp)
        {
            max_timestamp = time_seconds;
        }
    }
    mcap_reader.close();

    return std::tuple<unsigned int, double>{n_received_msgs, max_timestamp};
}

TEST(McapFileCreationTest, mcap_data_msgs)
{
    const std::string file_name = "output_mcap_data_msgs";
    eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicData>::ref_type send_data;
    send_data = record(file_name);

    eprosima::fastdds::dds::DynamicPubSubType pubsubType(test::dynamic_type_);
    eprosima::fastdds::rtps::SerializedPayload_t payload;
    payload.reserve(
        pubsubType.calculate_serialized_size(
            &send_data,
            eprosima::fastdds::dds::DEFAULT_DATA_REPRESENTATION
            )
        );
    pubsubType.serialize(&send_data, payload, eprosima::fastdds::dds::DEFAULT_DATA_REPRESENTATION);

    mcap::McapReader mcap_reader;
    auto messages = get_msgs_mcap(file_name, mcap_reader);

    for (auto it = messages.begin(); it != messages.end(); it++)
    {
        auto received_msg = reinterpret_cast<unsigned char const*>(it->message.data);
        for (unsigned int i = 0; i < payload.length; i++)
        {
            ASSERT_EQ(payload.data[i], received_msg[i]) << "wrong data !!";
        }
        ASSERT_EQ(payload.length, it->message.dataSize) << "length fails !!";
    }
    mcap_reader.close();
}

TEST(McapFileCreationTest, mcap_dds_topic)
{

    const std::string file_name = "output_mcap_dds_topic";

    record(file_name);

    mcap::McapReader mcap_reader;
    auto messages = get_msgs_mcap(file_name, mcap_reader);

    std::string received_topic = "";
    std::string received_data_type_name =  "";

    for (auto it = messages.begin(); it != messages.end(); it++)
    {
        received_topic = it->channel->topic;
        received_data_type_name = it->schema->name;
    }
    mcap_reader.close();

    // Test data
    ASSERT_EQ(received_topic, test::dds_topic_name);
    ASSERT_EQ(received_data_type_name, test::dds_type_name);

}

TEST(McapFileCreationTest, mcap_ros2_topic)
{

    const std::string file_name = "output_mcap_ros2_topic";

    record(file_name, 1, 1, true);

    mcap::McapReader mcap_reader;
    auto messages = get_msgs_mcap(file_name, mcap_reader);

    std::string received_topic = "";
    std::string received_data_type_name = "";

    for (auto it = messages.begin(); it != messages.end(); it++)
    {
        received_topic = it->channel->topic;
        received_data_type_name = it->schema->name;
    }
    mcap_reader.close();

    // Test data
    ASSERT_EQ(received_topic, eprosima::utils::demangle_if_ros_topic(test::ros2_topic_name));
    ASSERT_EQ(received_data_type_name, test::dds_type_name);

}

TEST(McapFileCreationTest, mcap_data_num_msgs)
{

    const std::string file_name = "output_mcap_data_num_msgs";

    record(file_name, test::n_msgs);

    mcap::McapReader mcap_reader;
    auto messages = get_msgs_mcap(file_name, mcap_reader);

    unsigned int n_received_msgs = 0;
    for (auto it = messages.begin(); it != messages.end(); it++)
    {
        n_received_msgs++;
    }
    mcap_reader.close();

    // Test data
    ASSERT_EQ(test::n_msgs, n_received_msgs);

}

TEST(McapFileCreationTest, mcap_data_num_msgs_downsampling)
{

    const std::string file_name = "output_mcap_data_num_msgs_downsampling";

    record(file_name, test::n_msgs, test::downsampling);

    mcap::McapReader mcap_reader;
    auto messages = get_msgs_mcap(file_name, mcap_reader);

    unsigned int n_received_msgs = 0;
    for (auto it = messages.begin(); it != messages.end(); it++)
    {
        n_received_msgs++;
    }
    mcap_reader.close();

    // Test data
    unsigned int expected_msgs = test::n_msgs / test::downsampling;
    if (test::n_msgs % test::downsampling)
    {
        expected_msgs++;
    }
    ASSERT_EQ(expected_msgs, n_received_msgs);

}

//////////////////////
// With transitions //
//////////////////////

TEST(McapFileCreationTest, transition_running)
{
    const std::string file_name = "output_transition_running";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::RUNNING,
        n_data_1, n_data_2,
        DdsRecorderState::RUNNING);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, (n_data_1 + n_data_2));

}

TEST(McapFileCreationTest, transition_paused)
{
    const std::string file_name = "output_transition_paused";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::PAUSED,
        n_data_1, n_data_2,
        DdsRecorderState::PAUSED);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, 0);

}

TEST(McapFileCreationTest, transition_stopped)
{
    const std::string file_name = "output_transition_stopped";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::STOPPED,
        n_data_1, n_data_2,
        DdsRecorderState::STOPPED);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, 0);

}

TEST(McapFileCreationTest, transition_suspended)
{
    const std::string file_name = "output_transition_suspended";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::SUSPENDED,
        n_data_1, n_data_2,
        DdsRecorderState::SUSPENDED);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, 0);

}

TEST(McapFileCreationTest, transition_running_paused)
{
    const std::string file_name = "output_transition_running_paused";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::RUNNING,
        n_data_1, n_data_2,
        DdsRecorderState::PAUSED);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, (n_data_1));

}

TEST(McapFileCreationTest, transition_running_stopped)
{
    const std::string file_name = "output_transition_running_stopped";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::RUNNING,
        n_data_1, n_data_2,
        DdsRecorderState::STOPPED);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, (n_data_1));

}

TEST(McapFileCreationTest, transition_running_suspended)
{
    const std::string file_name = "output_transition_running_suspended";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::RUNNING,
        n_data_1, n_data_2,
        DdsRecorderState::SUSPENDED);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, (n_data_1));

}

TEST(McapFileCreationTest, transition_paused_running)
{
    const std::string file_name = "output_transition_paused_running";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::PAUSED,
        n_data_1, n_data_2,
        DdsRecorderState::RUNNING);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, (n_data_2));

}

TEST(McapFileCreationTest, transition_paused_stopped)
{
    const std::string file_name = "output_transition_paused_stopped";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::PAUSED,
        n_data_1, n_data_2,
        DdsRecorderState::STOPPED);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, 0);

}

TEST(McapFileCreationTest, transition_paused_suspended)
{
    const std::string file_name = "output_transition_paused_suspended";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::PAUSED,
        n_data_1, n_data_2,
        DdsRecorderState::SUSPENDED);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, 0);

}

TEST(McapFileCreationTest, transition_stopped_running)
{
    const std::string file_name = "output_transition_stopped_running";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::STOPPED,
        n_data_1, n_data_2,
        DdsRecorderState::RUNNING);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, (n_data_2));

}

TEST(McapFileCreationTest, transition_stopped_paused)
{
    const std::string file_name = "output_transition_stopped_paused";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::STOPPED,
        n_data_1, n_data_2,
        DdsRecorderState::PAUSED);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, 0);

}

TEST(McapFileCreationTest, transition_stopped_suspended)
{
    const std::string file_name = "output_transition_stopped_suspended";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::SUSPENDED,
        n_data_1, n_data_2,
        DdsRecorderState::SUSPENDED);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, 0);

}

TEST(McapFileCreationTest, transition_suspended_running)
{
    const std::string file_name = "output_transition_suspended_running";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::SUSPENDED,
        n_data_1, n_data_2,
        DdsRecorderState::RUNNING);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, (n_data_2));

}

TEST(McapFileCreationTest, transition_suspended_paused)
{
    const std::string file_name = "output_transition_suspended_paused";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::SUSPENDED,
        n_data_1, n_data_2,
        DdsRecorderState::PAUSED);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, 0);

}

TEST(McapFileCreationTest, transition_suspended_stopped)
{
    const std::string file_name = "output_transition_suspended_stopped";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::STOPPED,
        n_data_1, n_data_2,
        DdsRecorderState::SUSPENDED);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, 0);

}

// can fail due to two race conditions but is very unlikely
TEST(McapFileCreationTest, transition_paused_event_less_window)
{
    const std::string file_name = "output_transition_paused_event_less_window";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;
    unsigned int event_window = 3;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::PAUSED,
        n_data_1, n_data_2,
        DdsRecorderState::PAUSED,
        EventKind::EVENT, event_window, 1);

    unsigned int n_received_msgs = std::get<0>(recording);
    double max_timestamp = std::get<1>(recording);

    ASSERT_EQ(n_received_msgs, (n_data_1 + n_data_2));
    ASSERT_LE(max_timestamp, event_window);

}

TEST(McapFileCreationTest, transition_paused_event_max_window)
{
    const std::string file_name = "output_transition_paused_event_max_window";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;
    unsigned int event_window = 3;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::PAUSED,
        n_data_1, n_data_2,
        DdsRecorderState::PAUSED,
        EventKind::EVENT, event_window, 3);

    unsigned int n_received_msgs = std::get<0>(recording);
    double max_timestamp = std::get<1>(recording);

    ASSERT_EQ(n_received_msgs, n_data_2);
    ASSERT_LE(max_timestamp, event_window);

}

TEST(McapFileCreationTest, transition_paused_event_start)
{
    const std::string file_name = "output_transition_paused_event_start";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;
    unsigned int event_window = 3;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::PAUSED,
        n_data_1, n_data_2,
        DdsRecorderState::PAUSED,
        EventKind::EVENT_START, event_window, 3);

    unsigned int n_received_msgs = std::get<0>(recording);
    double max_timestamp = std::get<1>(recording);

    ASSERT_EQ(n_received_msgs, n_data_2);
    ASSERT_LE(max_timestamp, event_window);

}

TEST(McapFileCreationTest, transition_paused_event_stop)
{
    const std::string file_name = "output_transition_paused_event_stop";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;
    unsigned int event_window = 3;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::PAUSED,
        n_data_1, n_data_2,
        DdsRecorderState::PAUSED,
        EventKind::EVENT_STOP, event_window, 3);

    unsigned int n_received_msgs = std::get<0>(recording);
    double max_timestamp = std::get<1>(recording);

    ASSERT_EQ(n_received_msgs, n_data_2);
    ASSERT_LE(max_timestamp, event_window);

}

TEST(McapFileCreationTest, transition_paused_event_suspend)
{
    const std::string file_name = "output_transition_paused_event_suspend";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;
    unsigned int event_window = 3;

    auto recording = record_with_transitions(
        file_name,
        DdsRecorderState::PAUSED,
        n_data_1, n_data_2,
        DdsRecorderState::PAUSED,
        EventKind::EVENT_SUSPEND, event_window, 3);

    unsigned int n_received_msgs = std::get<0>(recording);
    double max_timestamp = std::get<1>(recording);

    ASSERT_EQ(n_received_msgs, n_data_2);
    ASSERT_LE(max_timestamp, event_window);

}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
