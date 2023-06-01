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

// #define MCAP_IMPLEMENTATION

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/event/MultipleEventHandler.hpp>
#include <cpp_utils/logging/CustomStdLogConsumer.hpp>

#include "dds/HelloWorldSubscriber.h"

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
    // // Debug options
    // std::string log_filter = "DDSREPLAYER|DDSPIPE";
    // eprosima::fastdds::dds::Log::Kind log_verbosity = eprosima::fastdds::dds::Log::Kind::Info;

    // // Remove every consumer
    // eprosima::utils::Log::ClearConsumers();

    // // Activate log with verbosity, as this will avoid running log thread with not desired kind
    // eprosima::utils::Log::SetVerbosity(log_verbosity);

    // eprosima::utils::Log::RegisterConsumer(
    //     std::make_unique<eprosima::utils::CustomStdLogConsumer>(log_filter, log_verbosity));

    // Create replayer instance
    std::string input_file = "resources/helloworld_file.mcap";
    {
        // Configuration
        eprosima::ddsrecorder::yaml::ReplayerConfiguration configuration(configuration_path);

        auto replayer = std::make_unique<DdsReplayer>(configuration, input_file);

        std::cout << "replayer created !!!!" << std::endl;
        {
            // Create Subscriber
            HelloWorldSubscriber subscriber(
                test::topic_name,
                static_cast<uint32_t>(test::DOMAIN),
                expected_msgs,
                data);

            std::cout << "subscriber created !!!!" << std::endl;

            // Start replaying data
            replayer->process_mcap();

            // Run Participant
            subscriber.run();

            // Disable inner pipe, which would abort replaying messages in case execution stopped by signal
            replayer->stop();
        }
        std::cout << "subscriber destroyed!!!!" << std::endl;
    }
    std::cout << "replayer destroyed!!!!" << std::endl;

    std::cout << "process info..." << std::endl;

    // // Force print every log before closing
    // eprosima::utils::Log::Flush();
}

TEST(McapFileReadTest, trivial)
{
    // info to check
    DataToCheck data;
    create_subscriber_replayer(data);
    ASSERT_TRUE(true);
}

TEST(McapFileReadTest, data_to_check)
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
    ASSERT_GT(data.hz_msgs, 185);
    ASSERT_LT(data.hz_msgs, 215);
}

TEST(McapFileReadTest, less_playback_rate)
{
    // info to check
    DataToCheck data;
    std::string configuration = "resources/config_file_less_hz.yaml";
    create_subscriber_replayer(data, configuration);
    // hz ~ 200
    ASSERT_GT(data.hz_msgs, 85);
    ASSERT_LT(data.hz_msgs, 115);
}

TEST(McapFileReadTest, more_playback_rate)
{
    // info to check
    DataToCheck data;
    std::string configuration = "resources/config_file_more_hz.yaml";
    create_subscriber_replayer(data, configuration);
    // hz ~ 200
    ASSERT_GT(data.hz_msgs, 385);
    ASSERT_LT(data.hz_msgs, 415);
}

TEST(McapFileReadTest, begin_time)
{
    // info to check
    DataToCheck data;
    std::string configuration = "resources/config_file_begin_time.yaml";
    create_subscriber_replayer(data, configuration, 3);
    ASSERT_EQ(data.n_received_msgs, 3);
    ASSERT_EQ(data.min_index_msg, 8);
    ASSERT_EQ(data.max_index_msg, 10);
}

TEST(McapFileReadTest, end_time)
{
    // info to check
    DataToCheck data;
    std::string configuration = "resources/config_file_end_time.yaml";
    create_subscriber_replayer(data, configuration, 8);
    ASSERT_EQ(data.n_received_msgs, 8);
    ASSERT_EQ(data.min_index_msg, 0);
    ASSERT_EQ(data.max_index_msg, 7);
}

TEST(McapFileReadTest, start_replay_time_earlier)
{
    // info to check
    DataToCheck data;
    std::string configuration = "resources/config_file_start_replay_time_earlier.yaml";
    create_subscriber_replayer(data, configuration);
    ASSERT_EQ(data.n_received_msgs, 11);
    ASSERT_EQ(data.min_index_msg, 0);
    ASSERT_EQ(data.max_index_msg, 10);
}

TEST(McapFileReadTest, start_replay_time_later)
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
