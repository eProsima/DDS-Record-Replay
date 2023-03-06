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
#include <cpp_utils/event/FileWatcherHandler.hpp>
#include <cpp_utils/event/MultipleEventHandler.hpp>
#include <cpp_utils/event/PeriodicEventHandler.hpp>
#include <cpp_utils/event/SignalEventHandler.hpp>
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
#include <fastrtps/types/DynamicDataPtr.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/TypeObjectFactory.h>

#include <mcap/reader.hpp>

#include "command_receiver/CommandReceiver.hpp"

#include "types/hello_world/HelloWorldTypeObject.h"
#include "types/hello_world/HelloWorldPubSubTypes.h"

#include <iostream>

using namespace eprosima::fastdds::dds;
using namespace eprosima::ddspipe;
using namespace eprosima::ddsrecorder;

using CommandCode = eprosima::ddsrecorder::receiver::CommandCode;
using CommandCodeBuilder = eprosima::ddsrecorder::receiver::CommandCodeBuilder;
using McapHandlerState = eprosima::ddsrecorder::participants::McapHandler::StateCode;

enum class DataTypeKind
{
    HELLO_WORLD,
};

namespace test {

// Publisher

unsigned int domain = 223;

std::string topic = "TypeIntrospectionTopic";
std::string data_type_name = "HelloWorld";

eprosima::fastdds::dds::DataWriter* writer_;
eprosima::fastrtps::types::DynamicType_ptr dynamic_type_;

// Configuration

std::vector<const char*> yml_configurations =
{
    R"(
    dds:
        domain: 223
    recorder:
        buffer-size: 3
        event-window: 3
    remote-controller:
        enable: true
        domain: 222
    specs:
        threads: 8
        max-depth: 100
        max-pending-samples: 10
        cleanup-period: 3

    )",
};

YAML::Node yml;

} // test

std::unique_ptr<core::DdsPipe> create_recorder(
        eprosima::ddsrecorder::yaml::Configuration configuration,
        std::shared_ptr<eprosima::ddsrecorder::participants::McapHandler>& mcap_handler,
        std::string file_name,
        McapHandlerState mcap_handler_state)
{

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
        configuration.downsampling,
        configuration.event_window,
        configuration.cleanup_period);

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
        uint32_t domain,
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

eprosima::fastrtps::types::DynamicData_ptr send_sample()
{
    // Create and initialize new dynamic data
    eprosima::fastrtps::types::DynamicData_ptr dynamic_data_;
    dynamic_data_ = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(test::dynamic_type_);

    // Set index
    dynamic_data_->set_uint32_value(6, 0);
    // Set message
    dynamic_data_->set_string_value("Hello world", 1);
    test::writer_->write(dynamic_data_.get());

    usleep(100000);   // microsecond intervals - dont change

    logUser(DDSRECORDER_EXECUTION, "Message published.");

    return dynamic_data_;
}

std::tuple<unsigned int, double> record(
        std::string file_name,
        McapHandlerState init_state,
        int first_round,
        int secound_round,
        McapHandlerState current_state,
        bool event,
        int time_sleep = 0)
{
    {
        // Configuration
        for (const char* yml_configuration : test::yml_configurations)
        {
            test::yml = YAML::Load(yml_configuration);
        }
        eprosima::ddsrecorder::yaml::Configuration configuration(test::yml);

        // Create Publisher
        create_publisher(
            test::topic,
            test::domain,
            DataTypeKind::HELLO_WORLD);

        // Create Recorder
        std::shared_ptr<eprosima::ddsrecorder::participants::McapHandler> mcap_handler;

        std::unique_ptr<core::DdsPipe> recorder;
        if (init_state == McapHandlerState::stopped)
        {
            recorder = create_recorder(configuration, mcap_handler, file_name, McapHandlerState::started);
            usleep(100000);
            mcap_handler->stop();
        }
        else
        {
            recorder = create_recorder(configuration, mcap_handler, file_name, init_state);
        }

        // Send data
        for (int i = 0; i < first_round; i++)
        {
            send_sample();
        }

        if (init_state != current_state)
        {
            switch (current_state)
            {
                case McapHandlerState::started:
                    mcap_handler->start();
                    break;
                case McapHandlerState::stopped:
                    mcap_handler->stop();
                    break;
                case McapHandlerState::paused:
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
        sleep(time_sleep);

        for (int i = 0; i < secound_round; i++)
        {
            send_sample();
        }

        if (event && init_state == McapHandlerState::paused)
        {
            if (init_state == current_state)
            {
                mcap_handler->trigger_event();
            }
            // else {
            //     switch (current_state)
            //     {
            //     case McapHandlerState::started:
            //         mcap_handler->trigger_event("running");
            //         break;
            //     case McapHandlerState::stopped:
            //         mcap_handler->trigger_event("stopped");
            //         break;
            //     }
            // }
        }
    }

    // Read MCAP
    mcap::McapReader mcap_reader_;
    auto status = mcap_reader_.open(file_name);

    auto messages = mcap_reader_.readMessages();

    unsigned int n_received_msgs = 0;
    uint64_t actual_time = std::chrono::duration_cast<std::chrono::nanoseconds>
                (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    double max_timestamp = 0;
    for (auto it = messages.begin(); it != messages.end(); it++)
    {
        n_received_msgs++;
        double time_seconds = ((actual_time) - (it->message.publishTime)) * pow(10.0, -9.0);
        if (time_seconds > max_timestamp)
        {
            max_timestamp = time_seconds;
        }
    }
    mcap_reader_.close();

    return std::tuple<unsigned int, double>{n_received_msgs, max_timestamp};
}

TEST(McapFileCreationTestWithController, controller_paused_running)
{
    std::string file_name = "output_5_.mcap";

    int n_data_1 = rand() % 10 + 1;
    int n_data_2 = rand() % 10 + 1;

    auto recording = record(
        file_name,
        McapHandlerState::paused,
        n_data_1, n_data_2,
        McapHandlerState::started,
        0);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, (n_data_2));

}

TEST(McapFileCreationTestWithController, controller_running_paused)
{
    std::string file_name = "output_6_.mcap";

    int n_data_1 = rand() % 10 + 1;
    int n_data_2 = rand() % 10 + 1;

    auto recording = record(
        file_name,
        McapHandlerState::started,
        n_data_1, n_data_2,
        McapHandlerState::paused,
        0);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, (n_data_1));

}

TEST(McapFileCreationTestWithController, controller_running_stopped)
{
    std::string file_name = "output_7_.mcap";

    int n_data_1 = rand() % 10 + 1;
    int n_data_2 = rand() % 10 + 1;

    auto recording = record(
        file_name,
        McapHandlerState::started,
        n_data_1, n_data_2,
        McapHandlerState::stopped,
        0);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, (n_data_1));

}

TEST(McapFileCreationTestWithController, controller_stopped_running)
{
    std::string file_name = "output_8_.mcap";

    int n_data_1 = rand() % 10 + 1;
    int n_data_2 = rand() % 10 + 1;

    auto recording = record(
        file_name,
        McapHandlerState::stopped,
        n_data_1, n_data_2,
        McapHandlerState::started,
        0);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, (n_data_2));

}

TEST(McapFileCreationTestWithController, controller_paused_stopped)
{
    std::string file_name = "output_9_.mcap";

    int n_data_1 = rand() % 10 + 1;
    int n_data_2 = rand() % 10 + 1;

    auto recording = record(
        file_name,
        McapHandlerState::paused,
        n_data_1, n_data_2,
        McapHandlerState::stopped,
        0);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, 0);

}

TEST(McapFileCreationTestWithController, controller_stopped_paused)
{
    std::string file_name = "output_10_.mcap";

    int n_data_1 = rand() % 10 + 1;
    int n_data_2 = rand() % 10 + 1;

    auto recording = record(
        file_name,
        McapHandlerState::stopped,
        n_data_1, n_data_2,
        McapHandlerState::paused,
        0);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, 0);

}

TEST(McapFileCreationTestWithController, controller_running)
{
    std::string file_name = "output_12_.mcap";

    int n_data_1 = rand() % 10 + 1;
    int n_data_2 = rand() % 10 + 1;

    auto recording = record(
        file_name,
        McapHandlerState::started,
        n_data_1, n_data_2,
        McapHandlerState::started,
        0);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, (n_data_1 + n_data_2));

}

TEST(McapFileCreationTestWithController, controller_paused)
{
    std::string file_name = "output_13_.mcap";

    int n_data_1 = rand() % 10 + 1;
    int n_data_2 = rand() % 10 + 1;

    auto recording = record(
        file_name,
        McapHandlerState::paused,
        n_data_1, n_data_2,
        McapHandlerState::paused,
        0);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, 0);

}

TEST(McapFileCreationTestWithController, controller_stopped)
{
    std::string file_name = "output_14_.mcap";

    int n_data_1 = rand() % 10 + 1;
    int n_data_2 = rand() % 10 + 1;

    auto recording = record(
        file_name,
        McapHandlerState::stopped,
        n_data_1, n_data_2,
        McapHandlerState::stopped,
        0);

    unsigned int n_received_msgs = std::get<0>(recording);

    ASSERT_EQ(n_received_msgs, 0);

}

TEST(McapFileCreationTestWithController, controller_paused_event_less_window)
{
    std::string file_name = "output_15_.mcap";

    int n_data_1 = rand() % 10 + 1;
    int n_data_2 = rand() % 10 + 1;

    auto recording = record(
        file_name,
        McapHandlerState::paused,
        n_data_1, n_data_2,
        McapHandlerState::paused,
        1, 1);

    unsigned int n_received_msgs = std::get<0>(recording);
    double max_timestamp = std::get<1>(recording);
    unsigned int event_window = test::yml["recorder"]["event-window"].as<uint64_t>();

    ASSERT_EQ(n_received_msgs, (n_data_1 + n_data_2));
    ASSERT_LE(max_timestamp, event_window);

}

TEST(McapFileCreationTestWithController, controller_paused_event_max_window)
{
    std::string file_name = "output_16_.mcap";

    int n_data_1 = rand() % 10 + 1;
    int n_data_2 = rand() % 10 + 1;

    auto recording = record(
        file_name,
        McapHandlerState::paused,
        n_data_1, n_data_2,
        McapHandlerState::paused,
        1, 3);

    unsigned int n_received_msgs = std::get<0>(recording);
    double max_timestamp = std::get<1>(recording);
    unsigned int event_window = test::yml["recorder"]["event-window"].as<uint64_t>();

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
