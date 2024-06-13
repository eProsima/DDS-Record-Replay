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
#include <memory>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipantListener.hpp>

#include <cpp_utils/testing/gtest_aux.hpp>

#include <ddsrecorder_yaml/recorder/YamlReaderConfiguration.hpp>

#include <tool/DdsReplayer.hpp>

#include "../resources/constants.hpp"
#include "../resources/dds/DataToCheck.hpp"
#include "../resources/dds/HelloWorldDynTypesSubscriber.h"
#include "../resources/dds/HelloWorldSubscriber.h"

using namespace eprosima;

/**
 * @brief Test fixture for testing the DDS Replayer with different input files.
 *
 * Include tests that must pass with different input files.
 */
class FileReadTest : public testing::Test
{
public:

    /**
     * Verify that the DDS Replayer replays messages correctly.
     *
     * CASES:
     *  - Verify that the shared \c replay_ method replays messages.
     */
    static void trivial_test(
            const std::string& input_file,
            const bool publish_types = false,
            const bool is_ros2_topic = false)
    {
        const auto configuration = "../../resources/config/config_file.yaml";
        const auto data = replay_(configuration, input_file, publish_types);

        ASSERT_TRUE(true);
    }

    /**
     * Verify that the DDS Replayer replays simple messages correctly.
     *
     * CASES:
     *  - Verify that the received data is correct.
     *  - Verify that the message was sent at the correct rate.
     */
    static void data_to_check_test(
            const std::string& input_file,
            const bool publish_types = false,
            const bool is_ros2_topic = false)
    {
        const auto configuration = "../../resources/config/config_file.yaml";
        const auto data = replay_(configuration, input_file, publish_types);

        // Verify that the received data is correct
        ASSERT_EQ(data.n_received_msgs, 11);
        ASSERT_EQ(data.type_msg, "HelloWorld");
        ASSERT_EQ(data.message_msg, "Hello World");
        ASSERT_EQ(data.min_index_msg, 0);
        ASSERT_EQ(data.max_index_msg, 10);

        // Verify that the average miliseconds between messages is about 200 ms
        const auto expected_mean_ms_between_msgs = 200;
        const auto expected_error = 0.01;

        const auto min_expected_ms = expected_mean_ms_between_msgs * (1 - expected_error);
        const auto max_expected_ms = expected_mean_ms_between_msgs * (1 + expected_error);

        ASSERT_GT(data.mean_ms_between_msgs, min_expected_ms);
        ASSERT_LT(data.mean_ms_between_msgs, max_expected_ms);
    }

    /**
     * Verify that the DDS Replayer replays messages published at a high frequency correctly.
     *
     * CASES:
     *  - Verify that the messages were received at the correct rate.
     */
    static void more_playback_rate_test(
            const std::string& input_file,
            const bool publish_types = false,
            const bool is_ros2_topic = false)
    {
        const auto configuration = "../../resources/config/config_file_more_hz.yaml";
        const auto data = replay_(configuration, input_file, publish_types);

        // Verify that the average miliseconds between messages is about 100 ms
        const auto expected_mean_ms_between_msgs = 100;
        const auto expected_error = 0.01;

        const auto min_expected_ms = expected_mean_ms_between_msgs * (1 - expected_error);
        const auto max_expected_ms = expected_mean_ms_between_msgs * (1 + expected_error);

        ASSERT_GT(data.mean_ms_between_msgs, min_expected_ms);
        ASSERT_LT(data.mean_ms_between_msgs, max_expected_ms);
    }

    /**
     * Verify that the DDS Replayer replays messages published at a low frequency correctly.
     *
     * CASES:
     *  - Verify that the messages were received at the correct rate.
     */
    static void less_playback_rate_test(
            const std::string& input_file,
            const bool publish_types = false,
            const bool is_ros2_topic = false)
    {
        const auto configuration = "../../resources/config/config_file_less_hz.yaml";
        const auto data = replay_(configuration, input_file, publish_types);

        // Verify that the average miliseconds between messages is about 400 ms
        const auto expected_mean_ms_between_msgs = 400;
        const auto expected_error = 0.01;

        const auto min_expected_ms = expected_mean_ms_between_msgs * (1 - expected_error);
        const auto max_expected_ms = expected_mean_ms_between_msgs * (1 + expected_error);

        ASSERT_GT(data.mean_ms_between_msgs, min_expected_ms);
        ASSERT_LT(data.mean_ms_between_msgs, max_expected_ms);
    }

    /**
     * Verify that the DDS Replayer replays messages only from the begin-time.
     *
     * CASES:
     *  - Verify that the right number of messages were received.
     *  - Verify that the messages received are the correct ones.
     */
    static void begin_time_test(
            const std::string& input_file,
            const bool publish_types = false,
            const bool is_ros2_topic = false)
    {
        const auto configuration = "../../resources/config/config_file_begin_time.yaml";
        const auto data = replay_(configuration, input_file, publish_types);

        // Verify that only the messages after the begin-time were received
        ASSERT_EQ(data.n_received_msgs, 3);
        ASSERT_EQ(data.min_index_msg, 8);
        ASSERT_EQ(data.max_index_msg, 10);
    }

    /**
     * Verify that the DDS Replayer replays messages only until the end-time.
     *
     * CASES:
     *  - Verify that the right number of messages were received.
     *  - Verify that the messages received are the correct ones.
     */
    static void end_time_test(
            const std::string& input_file,
            const bool publish_types = false,
            const bool is_ros2_topic = false)
    {
        const auto configuration = "../../resources/config/config_file_end_time.yaml";
        const auto data = replay_(configuration, input_file, publish_types);

        // Verify that only the messages before the end-time were received
        ASSERT_EQ(data.n_received_msgs, 8);
        ASSERT_EQ(data.min_index_msg, 0);
        ASSERT_EQ(data.max_index_msg, 7);
    }

    /**
     * Verify that the DDS Replayer replays messages when the start-replay-time is earlier than the earliest message.
     *
     * CASES:
     *  - Verify that the right number of messages were received.
     *  - Verify that the messages received are the correct ones.
     */
    static void start_replay_time_earlier_test(
            const std::string& input_file,
            const bool publish_types = false,
            const bool is_ros2_topic = false)
    {
        const auto configuration = "../../resources/config/config_file_start_replay_time_earlier.yaml";
        const auto data = replay_(configuration, input_file, publish_types);

        // Verify that all the messages were received
        ASSERT_EQ(data.n_received_msgs, 11);
        ASSERT_EQ(data.min_index_msg, 0);
        ASSERT_EQ(data.max_index_msg, 10);
    }

protected:

    /**
     * The order in which objects are created is relevant;
     * if the replayer was created before the subscriber,
     * a segmentation fault may occur as the dynamic type
     * could be received by the subscriber's participant
     * before the DDS subscriber is created (which is required
     * for creating a DataReader with the received type).
     */
    static DataToCheck replay_(
            const std::string& configuration_path,
            const std::string& input_file,
            bool publish_types = false,
            bool is_ros2_topic = false)
    {
        DataToCheck data;

        // Create Subscriber
        std::unique_ptr<fastdds::dds::DomainParticipantListener> subscriber;

        const auto topic_name = is_ros2_topic ? test::ROS2_TOPIC_NAME : test::DDS_TOPIC_NAME;

        if (publish_types)
        {
            subscriber = std::make_unique<HelloWorldDynTypesSubscriber>(topic_name, test::DOMAIN, data);
        }
        else
        {
            subscriber = std::make_unique<HelloWorldSubscriber>(topic_name, test::DOMAIN, data);
        }

        // Configuration
        ddsrecorder::yaml::ReplayerConfiguration configuration(configuration_path);
        configuration.replayer_configuration->domain.domain_id = test::DOMAIN;

        // Create replayer instance
        auto replayer = std::make_unique<ddsrecorder::replayer::DdsReplayer>(configuration, input_file);

        // Give time for replayer and subscriber to match.
        // Waiting for the subscriber to match the replayer
        // before starting to replay messages does not ensure
        // that no samples will be lost (even if using reliable QoS).
        // This is because endpoint matching does not occur
        // at the same exact moment in both ends of communication,
        // so the replayer's writer might have not yet matched the
        // subscriber even if the latter already has (matched the writer).
        // Transient local QoS would be a solution for this,
        // but it is not used as it might pollute frequency arrival
        // measurements.
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Start replaying data
        replayer->process_file();

        replayer->stop();

        // Replayer waits on destruction a maximum of wait-all-acked-timeout
        // ms until all sent msgs are acknowledged
        return data;
    }
};
