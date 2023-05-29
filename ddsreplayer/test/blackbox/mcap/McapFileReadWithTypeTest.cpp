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

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/event/MultipleEventHandler.hpp>

#include "dds/HelloWorldDynTypesSubscriber.h"

#include "tool/DdsReplayer.hpp"

#include <iostream>
#include <thread>
#include <chrono>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps;
using namespace eprosima::ddsrecorder::replayer;

namespace test {

// Publisher

const unsigned int DOMAIN = 0;

std::string topic_name = "/dds/topic";
// std::string data_type_name = "HelloWorld";

} // test


void create_subscriber_replayer(
        DataToCheck& data,
        std::string configuration_path = "resources/config_file.yaml",
        uint32_t expected_msgs = 11)
{
    // Configuration
    eprosima::ddsrecorder::yaml::ReplayerConfiguration configuration(configuration_path);

    // Create replayer instance
    std::string input_file = "resources/helloworld_withtype_file.mcap";

    auto replayer = std::make_unique<DdsReplayer>(configuration, input_file);

    // Create a multiple event handler that handles all events that make subscriber and replayer stop
    auto close_handler_subscriber = std::make_shared<eprosima::utils::event::MultipleEventHandler>();

    // Create Subscriber
    HelloWorldDynTypesSubscriber subscriber(
        test::topic_name,
        static_cast<uint32_t>(test::DOMAIN),
        expected_msgs,
        data);

    std::cout << "subscriber created !!!!" << std::endl;

    // Run Participant
    std::thread run_subscriber([&]
        {
            try
            {
                subscriber.run();
            }
            catch (const eprosima::utils::InconsistencyException& e)
            {
                logError(DDSREPLAYER_ERROR,
                "Error running subscriber. Error message:\n " <<
                    e.what());
            }
            close_handler_subscriber->simulate_event_occurred();
        });

    // Start replaying data
    replayer->process_mcap();

    // Wait until signal arrives
    close_handler_subscriber->wait_for_event();

    // Disable inner pipe, which would abort replaying messages in case execution stopped by signal
    replayer->stop();

    run_subscriber.join();
    std::cout << "thread joined!!!!" << std::endl;

    std::cout << "process info..." << std::endl;
}

TEST(McapFileReadWithTypeTest, trivial)
{
    // info to check
    DataToCheck data;
    std::string configuration = "resources/config_file.yaml";
    create_subscriber_replayer(data, configuration);
    ASSERT_TRUE(true);
}

TEST(McapFileReadWithTypeTest, data_to_check)
{
    // info to check
    DataToCheck data;
    create_subscriber_replayer(data);
    ASSERT_EQ(data.n_received_msgs, 11);
    ASSERT_EQ(data.type_msg, "HelloWorld");
    ASSERT_EQ(data.message_msg, "Hello World");
    ASSERT_EQ(data.min_index_msg, 0);
    ASSERT_EQ(data.max_index_msg, 10);
    // hz ~ 200
    ASSERT_GT(data.hz_msgs, 190);
    ASSERT_LT(data.hz_msgs, 210);
}

TEST(McapFileReadWithTypeTest, less_playback_rate)
{
    // info to check
    DataToCheck data;
    std::string configuration = "resources/config_file_less_hz.yaml";
    create_subscriber_replayer(data, configuration);
    // hz ~ 200
    ASSERT_GT(data.hz_msgs, 90);
    ASSERT_LT(data.hz_msgs, 110);
}

TEST(McapFileReadWithTypeTest, more_playback_rate)
{
    // info to check
    DataToCheck data;
    std::string configuration = "resources/config_file_more_hz.yaml";
    create_subscriber_replayer(data, configuration);
    // hz ~ 200
    ASSERT_GT(data.hz_msgs, 390);
    ASSERT_LT(data.hz_msgs, 410);
}

TEST(McapFileReadWithTypeTest, begin_time)
{
    // info to check
    DataToCheck data;
    std::string configuration = "resources/config_file_begin_time.yaml";
    create_subscriber_replayer(data, configuration, 2);
    ASSERT_EQ(data.n_received_msgs, 2);
    ASSERT_EQ(data.min_index_msg, 9);       // should be 8 !!?
    ASSERT_EQ(data.max_index_msg, 10);
}

TEST(McapFileReadWithTypeTest, end_time)
{
    // info to check
    DataToCheck data;
    std::string configuration = "resources/config_file_end_time.yaml";
    create_subscriber_replayer(data, configuration, 8);
    ASSERT_EQ(data.n_received_msgs, 8);
    ASSERT_EQ(data.min_index_msg, 0);
    ASSERT_EQ(data.max_index_msg, 7);
}

TEST(McapFileReadWithTypeTest, start_replay_time_earlier)
{
    // info to check
    DataToCheck data;
    std::string configuration = "resources/config_file_start_replay_time_earlier.yaml";
    create_subscriber_replayer(data, configuration);
    ASSERT_EQ(data.n_received_msgs, 11);
    ASSERT_EQ(data.min_index_msg, 0);
    ASSERT_EQ(data.max_index_msg, 10);
}

TEST(McapFileReadWithTypeTest, start_replay_time_later)
{
    // info to check
    DataToCheck data;
    std::string configuration = "resources/config_file_start_replay_time_later.yaml";
    create_subscriber_replayer(data, configuration, 0);
    ASSERT_EQ(data.n_received_msgs, 0);
    ASSERT_EQ(data.min_index_msg, -1);
    ASSERT_EQ(data.max_index_msg, -1);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
