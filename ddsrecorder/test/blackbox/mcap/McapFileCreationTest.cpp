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

#define MCAP_IMPLEMENTATION

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/logging/CustomStdLogConsumer.hpp>

#include <ddspipe_core/core/DdsPipe.hpp>
#include <ddspipe_core/efficiency/payload/FastPayloadPool.hpp>

#include <ddspipe_participants/participant/dynamic_types/DynTypesParticipant.hpp>
#include <ddspipe_participants/participant/dynamic_types/SchemaParticipant.hpp>

#include <ddsrecorder_participants/mcap/McapHandler.hpp>
#include <ddsrecorder_participants/mcap/McapHandlerConfiguration.hpp>

#include <ddsrecorder_yaml/YamlReaderConfiguration.hpp>
#include <ddsrecorder_yaml/yaml_configuration_tags.hpp>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastrtps/types/DynamicDataPtr.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/TypeObjectFactory.h>

#include <mcap/reader.hpp>

#include "types/hello_world/HelloWorldTypeObject.h"
#include "types/hello_world/HelloWorldPubSubTypes.h"

#include <iostream>
#include <thread>
#include <chrono>

using namespace eprosima::fastdds::dds;
using namespace eprosima::ddspipe;
using namespace eprosima::ddsrecorder;

using McapHandlerState = eprosima::ddsrecorder::participants::McapHandlerStateCode;

enum class DataTypeKind
{
    HELLO_WORLD,
};

namespace test {

// Publisher

const unsigned int DOMAIN = 222;

std::string topic = "TypeIntrospectionTopic";
std::string data_type_name = "HelloWorld";

unsigned int n_msgs = 3;
std::string send_message = "Hello World";
unsigned int index = 6;
unsigned int downsampling = 3;

eprosima::fastdds::dds::DataWriter* writer_;
eprosima::fastrtps::types::DynamicType_ptr dynamic_type_;

} // test

std::unique_ptr<core::DdsPipe> create_recorder(
        std::string file_name,
        std::shared_ptr<eprosima::ddsrecorder::participants::McapHandler>& mcap_handler,
        int downsampling,
        McapHandlerState mcap_handler_state = McapHandlerState::RUNNING,
        unsigned int event_window = 20)
{
    YAML::Node yml;

    eprosima::ddsrecorder::yaml::Configuration configuration(yml);
    configuration.downsampling = downsampling;
    // Set default value for downsampling
    // TODO: Change mechanism setting topic qos' default values from specs
    eprosima::ddspipe::core::types::TopicQoS::default_downsampling.store(downsampling);
    configuration.event_window = event_window;
    eprosima::ddspipe::core::types::DomainId domainId;
    domainId.domain_id = test::DOMAIN;
    configuration.simple_configuration->domain = domainId;

    // Create allowed topics list
    auto allowed_topics = std::make_shared<core::AllowedTopicList>(
        configuration.allowlist,
        configuration.blocklist);
    // Create Discovery Database
    std::shared_ptr<core::DiscoveryDatabase> discovery_database =
            std::make_shared<core::DiscoveryDatabase>();
    // Create Payload Pool
    std::shared_ptr<core::PayloadPool> payload_pool =
            std::make_shared<core::FastPayloadPool>();
    // Create Thread Pool
    std::shared_ptr<eprosima::utils::SlotThreadPool> thread_pool =
            std::make_shared<eprosima::utils::SlotThreadPool>(configuration.n_threads);

    // Create MCAP Handler
    eprosima::ddsrecorder::participants::McapHandlerConfiguration handler_config(
        file_name,
        configuration.max_pending_samples,
        configuration.buffer_size,
        configuration.event_window,
        configuration.cleanup_period,
        configuration.log_publish_time);

    mcap_handler = std::make_shared<eprosima::ddsrecorder::participants::McapHandler>(
        handler_config,
        payload_pool,
        mcap_handler_state);

    // Create DynTypes Participant
    auto dyn_participant = std::make_shared<eprosima::ddspipe::participants::DynTypesParticipant>(
        configuration.simple_configuration,
        payload_pool,
        discovery_database);
    dyn_participant->init();

    // Create Recorder Participant
    auto recorder_participant = std::make_shared<eprosima::ddspipe::participants::SchemaParticipant>(
        configuration.recorder_configuration,
        payload_pool,
        discovery_database,
        mcap_handler);

    // Create and populate Participant Database
    std::shared_ptr<core::ParticipantsDatabase> participant_database =
            std::make_shared<core::ParticipantsDatabase>();

    // Populate Participant Database
    participant_database->add_participant(
        dyn_participant->id(),
        dyn_participant
        );
    participant_database->add_participant(
        recorder_participant->id(),
        recorder_participant
        );

    return std::make_unique<core::DdsPipe>(
        allowed_topics,
        discovery_database,
        payload_pool,
        participant_database,
        thread_pool,
        configuration.builtin_topics,
        true
        );
}

void create_publisher(
        std::string topic_name,
        unsigned int domain,
        DataTypeKind data_type_kind)
{
    eprosima::fastdds::dds::DomainParticipantQos pqos;
    pqos.name("TypeIntrospectionExample_Participant_Publisher");
    pqos.wire_protocol().builtin.typelookup_config.use_client = false;
    pqos.wire_protocol().builtin.typelookup_config.use_server = true;

    // Create the Participant
    eprosima::fastdds::dds::DomainParticipant* participant_ =
            DomainParticipantFactory::get_instance()->create_participant(domain, pqos);

    // Register the type
    registerHelloWorldTypes();
    test::dynamic_type_ = eprosima::fastrtps::types::TypeObjectFactory::get_instance()->build_dynamic_type(
        test::data_type_name,
        GetHelloWorldIdentifier(true),
        GetHelloWorldObject(true));

    TypeSupport type(new eprosima::fastrtps::types::DynamicPubSubType(test::dynamic_type_));
    // Set type so introspection info is sent
    type->auto_fill_type_information(true);
    type->auto_fill_type_object(false);
    // Register the type in the Participant
    participant_->register_type(type);

    // Create the Publisher
    eprosima::fastdds::dds::Publisher* publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

    // Create the DDS Topic
    eprosima::fastdds::dds::Topic* topic_ = participant_->create_topic(topic_name, test::data_type_name,
                    TOPIC_QOS_DEFAULT);

    // Create the DDS DataWriter
    test::writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, nullptr);
}

eprosima::fastrtps::types::DynamicData_ptr send_sample(
        unsigned int index = 1,
        unsigned int time_sleep = 100)
{
    // Create and initialize new dynamic data
    eprosima::fastrtps::types::DynamicData_ptr dynamic_data_;
    dynamic_data_ = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(test::dynamic_type_);

    // Set index
    dynamic_data_->set_uint32_value(index, 0);
    // Set message
    dynamic_data_->set_string_value(test::send_message, 1);
    test::writer_->write(dynamic_data_.get());

    logInfo(DDSRECORDER_EXECUTION, "Message published.");

    std::this_thread::sleep_for(std::chrono::milliseconds(time_sleep));

    return dynamic_data_;
}

eprosima::fastrtps::types::DynamicData_ptr record(
        std::string file_name,
        unsigned int num_msgs = 1,
        unsigned int downsampling = 1)
{
    eprosima::fastrtps::types::DynamicData_ptr send_data;

    // Create Recorder
    std::shared_ptr<eprosima::ddsrecorder::participants::McapHandler> mcap_handler;
    auto recorder = create_recorder(file_name, mcap_handler, downsampling);

    // Create Publisher
    create_publisher(
        test::topic,
        test::DOMAIN,
        DataTypeKind::HELLO_WORLD);

    // Send data
    for (unsigned int i = 0; i < num_msgs; i++)
    {
        send_data = send_sample(test::index);
    }

    return send_data;
}

mcap::LinearMessageView get_msgs_mcap(
        std::string file_name,
        mcap::McapReader& mcap_reader_)
{
    auto status = mcap_reader_.open(file_name);

    auto messages = mcap_reader_.readMessages();

    return messages;
}

std::tuple<unsigned int, double> record_with_transitions(
        std::string file_name,
        McapHandlerState init_state,
        unsigned int first_round,
        unsigned int secound_round,
        McapHandlerState current_state,
        bool event = false,
        unsigned int event_window = 20,
        unsigned int time_sleep = 0,
        unsigned int downsampling = 1)
{
    uint64_t current_time;
    {
        // Create Publisher
        create_publisher(
            test::topic,
            test::DOMAIN,
            DataTypeKind::HELLO_WORLD);

        // Create Recorder
        std::shared_ptr<eprosima::ddsrecorder::participants::McapHandler> mcap_handler;

        std::unique_ptr<core::DdsPipe> recorder;
        if (init_state == McapHandlerState::STOPPED)
        {
            // avoid race condition on TypeObject reception
            recorder = create_recorder(file_name, mcap_handler, downsampling, McapHandlerState::RUNNING, event_window);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            mcap_handler->stop();
        }
        else
        {
            recorder = create_recorder(file_name, mcap_handler, downsampling, init_state, event_window);
        }

        // Send data
        for (unsigned int i = 0; i < first_round; i++)
        {
            send_sample();
        }

        if (init_state != current_state)
        {
            switch (current_state)
            {
                case McapHandlerState::RUNNING:
                    mcap_handler->start();
                    break;
                case McapHandlerState::STOPPED:
                    mcap_handler->stop();
                    break;
                case McapHandlerState::PAUSED:
                    mcap_handler->pause();
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
                    (std::chrono::high_resolution_clock::now().time_since_epoch()).count();

        if (event && current_state == McapHandlerState::PAUSED)
        {
            mcap_handler->trigger_event();
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

    std::string file_name = "output_mcap_data_msgs_.mcap";
    eprosima::fastrtps::types::DynamicData_ptr send_data;
    send_data = record(file_name);

    eprosima::fastrtps::types::DynamicPubSubType pubsubType;
    eprosima::fastrtps::rtps::SerializedPayload_t payload;
    payload.reserve(
        pubsubType.getSerializedSizeProvider(
            send_data.get()
            )()
        );
    pubsubType.serialize(send_data.get(), &payload);

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

TEST(McapFileCreationTest, mcap_data_topic)
{

    std::string file_name = "output_mcap_data_topic_.mcap";

    record(file_name);

    mcap::McapReader mcap_reader;
    auto messages = get_msgs_mcap(file_name, mcap_reader);

    std::string received_topic;
    std::string received_data_type_name;

    for (auto it = messages.begin(); it != messages.end(); it++)
    {
        received_topic = it->channel->topic;
        received_data_type_name = it->schema->name;
    }
    mcap_reader.close();

    // Test data
    ASSERT_EQ(received_topic, test::topic);
    ASSERT_EQ(received_data_type_name, test::data_type_name);

}

TEST(McapFileCreationTest, mcap_data_num_msgs)
{

    std::string file_name = "output_mcap_data_num_msgs_.mcap";

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

    std::string file_name = "output_mcap_data_num_msgs_downsampling_.mcap";

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

TEST(McapFileCreationTest, transition_paused_running)
{
    std::string file_name = "output_transition_paused_running_.mcap";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        McapHandlerState::PAUSED,
        n_data_1, n_data_2,
        McapHandlerState::RUNNING);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, (n_data_2));

}

TEST(McapFileCreationTest, transition_running_paused)
{
    std::string file_name = "output_transition_running_paused_.mcap";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        McapHandlerState::RUNNING,
        n_data_1, n_data_2,
        McapHandlerState::PAUSED);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, (n_data_1));

}

TEST(McapFileCreationTest, transition_running_stopped)
{
    std::string file_name = "output_transition_running_stopped_.mcap";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        McapHandlerState::RUNNING,
        n_data_1, n_data_2,
        McapHandlerState::STOPPED);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, (n_data_1));

}

TEST(McapFileCreationTest, transition_stopped_running)
{
    std::string file_name = "output_transition_stopped_running_.mcap";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        McapHandlerState::STOPPED,
        n_data_1, n_data_2,
        McapHandlerState::RUNNING);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, (n_data_2));

}

TEST(McapFileCreationTest, transition_paused_stopped)
{
    std::string file_name = "output_transition_paused_stopped_.mcap";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        McapHandlerState::PAUSED,
        n_data_1, n_data_2,
        McapHandlerState::STOPPED);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, 0);

}

TEST(McapFileCreationTest, transition_stopped_paused)
{
    std::string file_name = "output_transition_stopped_paused_.mcap";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        McapHandlerState::STOPPED,
        n_data_1, n_data_2,
        McapHandlerState::PAUSED);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, 0);

}

TEST(McapFileCreationTest, transition_running)
{
    std::string file_name = "output_transition_running_.mcap";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        McapHandlerState::RUNNING,
        n_data_1, n_data_2,
        McapHandlerState::RUNNING);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, (n_data_1 + n_data_2));

}

TEST(McapFileCreationTest, transition_paused)
{
    std::string file_name = "output_transition_paused_.mcap";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        McapHandlerState::PAUSED,
        n_data_1, n_data_2,
        McapHandlerState::PAUSED);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, 0);

}

TEST(McapFileCreationTest, transition_stopped)
{
    std::string file_name = "output_transition_stopped_.mcap";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;

    auto recording = record_with_transitions(
        file_name,
        McapHandlerState::STOPPED,
        n_data_1, n_data_2,
        McapHandlerState::STOPPED);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, 0);

}

// can fail due to two race conditions but is very unlikely
TEST(McapFileCreationTest, transition_paused_event_less_window)
{
    std::string file_name = "output_transition_paused_event_less_window_.mcap";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;
    unsigned int event_window = 3;

    auto recording = record_with_transitions(
        file_name,
        McapHandlerState::PAUSED,
        n_data_1, n_data_2,
        McapHandlerState::PAUSED,
        true, event_window, 1);

    unsigned int n_received_msgs = std::get<0>(recording);
    double max_timestamp = std::get<1>(recording);

    ASSERT_EQ(n_received_msgs, (n_data_1 + n_data_2));
    ASSERT_LE(max_timestamp, event_window);

}

TEST(McapFileCreationTest, transition_paused_event_max_window)
{
    std::string file_name = "output_transition_paused_event_max_window_.mcap";

    unsigned int n_data_1 = rand() % 10 + 1;
    unsigned int n_data_2 = rand() % 10 + 1;
    unsigned int event_window = 3;

    auto recording = record_with_transitions(
        file_name,
        McapHandlerState::PAUSED,
        n_data_1, n_data_2,
        McapHandlerState::PAUSED,
        true, event_window, 3);

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
