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

const unsigned int DOMAIN = 110;

std::string topic_name = "/dds/topic";

} // test


/**
 * It is necessary to create the DDS Subscriber before the DDS Recorder
 * so that it does not cause a segfault that can trigger for an existing
 * race condition that shows up if the subscriber receives the type when
 * it is being created and tries to create the data reader before the
 * subscriber creation is complete.
 */
void create_subscriber_replayer(
        DataToCheck& data,
        std::string configuration_path = "resources/config_file.yaml",
        std::string input_file = "resources/helloworld_withtype_file.mcap")
{
    {
        // Create Subscriber
        HelloWorldDynTypesSubscriber subscriber(
            test::topic_name,
            static_cast<uint32_t>(test::DOMAIN),
            data);

        std::cout << "subscriber created !!!!" << std::endl;

        {
            // Configuration
            eprosima::ddsrecorder::yaml::ReplayerConfiguration configuration(configuration_path);
            configuration.replayer_configuration->domain.domain_id = test::DOMAIN;

            // Create replayer instance
            auto replayer = std::make_unique<DdsReplayer>(configuration, input_file);

            std::cout << "replayer created !!!!" << std::endl;

            // Start replaying data
            replayer->process_mcap();

            replayer->stop();

            std::cout << "replayer stop!!!!" << std::endl;

        }
        std::cout << "replayer destroyed!!!!" << std::endl;
    }
    std::cout << "subscriber destroyed!!!!" << std::endl;

    std::cout << "process info..." << std::endl;
}

TEST(McapFileReadWithTypeTest, trivial)
{
    // info to check
    DataToCheck data;
    create_subscriber_replayer(data);
    ASSERT_TRUE(true);
}

TEST(McapFileReadWithTypeTest, data_to_check)
{
    // info to check
    DataToCheck data;
    create_subscriber_replayer(data);
    ASSERT_EQ(data.n_received_msgs, 13);
    ASSERT_EQ(data.type_msg, "HelloWorld");
    ASSERT_EQ(data.message_msg, "Hello World");
    ASSERT_EQ(data.min_index_msg, 0);
    ASSERT_EQ(data.max_index_msg, 12);
    // hz ~ 200
    ASSERT_GT(data.hz_msgs, 185);
    ASSERT_LT(data.hz_msgs, 215);
}

TEST(McapFileReadWithTypeTest, less_playback_rate)
{
    // info to check
    DataToCheck data;
    std::string configuration = "resources/config_file_less_hz.yaml";
    create_subscriber_replayer(data, configuration);
    // hz ~ 200
    ASSERT_GT(data.hz_msgs, 85);
    ASSERT_LT(data.hz_msgs, 115);
}

TEST(McapFileReadWithTypeTest, more_playback_rate)
{
    // info to check
    DataToCheck data;
    std::string configuration = "resources/config_file_more_hz.yaml";
    create_subscriber_replayer(data, configuration);
    // hz ~ 200
    ASSERT_GT(data.hz_msgs, 385);
    ASSERT_LT(data.hz_msgs, 415);
}

TEST(McapFileReadWithTypeTest, begin_time)
{
    // info to check
    DataToCheck data;
    std::string configuration = "resources/config_file_begin_time_with_types.yaml";
    create_subscriber_replayer(data, configuration);
    ASSERT_EQ(data.n_received_msgs, 6);
    ASSERT_EQ(data.min_index_msg, 7);
    ASSERT_EQ(data.max_index_msg, 12);
}

TEST(McapFileReadWithTypeTest, end_time)
{
    // info to check
    DataToCheck data;
    std::string configuration = "resources/config_file_end_time_with_types.yaml";
    create_subscriber_replayer(data, configuration);
    ASSERT_EQ(data.n_received_msgs, 7);
    ASSERT_EQ(data.min_index_msg, 0);
    ASSERT_EQ(data.max_index_msg, 6);
}

TEST(McapFileReadWithTypeTest, start_replay_time_earlier)
{
    // info to check
    DataToCheck data;
    std::string configuration = "resources/config_file_start_replay_time_earlier_with_types.yaml";
    create_subscriber_replayer(data, configuration);
    ASSERT_EQ(data.n_received_msgs, 13);
    ASSERT_EQ(data.min_index_msg, 0);
    ASSERT_EQ(data.max_index_msg, 12);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
