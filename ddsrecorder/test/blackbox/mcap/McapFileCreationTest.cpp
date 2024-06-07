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

#include <cstdint>
#include <filesystem>
#include <string>

#include <mcap/reader.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/ros2_mangling.hpp>
#include <cpp_utils/testing/gtest_aux.hpp>

#include <ddspipe_core/types/dds/TopicQoS.hpp>

#include <ddsrecorder_participants/common/time_utils.hpp>
#include <ddsrecorder_participants/recorder/output/OutputSettings.hpp>
#include <ddsrecorder_yaml/recorder/yaml_configuration_tags.hpp>

#include <tool/DdsRecorder.hpp>

#include "../constants.hpp"
#include "../FileCreationTest.hpp"

using namespace eprosima::ddspipe;
using namespace eprosima::ddsrecorder;
using namespace eprosima::ddsrecorder::recorder;
using namespace eprosima::fastdds::dds;

class McapFileCreationTest : public FileCreationTest
{
public:

    void SetUp() override
    {
        FileCreationTest::SetUp();

        // Set the output library to MCAP
        configuration_->output_library = ddsrecorder::participants::OutputLibrary::mcap;
    }

    void TearDown() override
    {
        mcap_reader_.close();

        FileCreationTest::TearDown();
    }

protected:

    mcap::LinearMessageView read_messages_(
            const std::string& file_path)
    {
        const auto status = mcap_reader_.open(file_path);

        // NOTE: This can't be made const since a copy const constructor doesn't exist.
        auto messages = mcap_reader_.readMessages();

        return messages;
    }

    unsigned int count_messages_(
            mcap::LinearMessageView& messages)
    {
        unsigned int received_messages = 0;

        // TODO: Replace this method with messages.size() or with std::distance when MCAP's API allows it.
        for (const auto& message : messages)
        {
            received_messages++;

            // Avoid unused variable warning
            (void) message;
        }

        return received_messages;
    }

    double find_max_time_past_(
            mcap::LinearMessageView& messages)
    {
        const auto now = ddsrecorder::participants::to_mcap_timestamp(utils::now());

        std::uint64_t max_time_past = 0;

        for (const auto& it : messages)
        {
            const auto time_past = now - it.message.logTime;

            if (time_past > max_time_past)
            {
                max_time_past = time_past;
            }
        }

        const auto NS_TO_SEC = pow(10, -9);
        const auto max_time_past_sec = max_time_past * NS_TO_SEC;

        return max_time_past_sec;
    }

    mcap::McapReader mcap_reader_;
};

/**
 * Verify that the DDS Recorder records properly in an MCAP file.
 *
 * CASES:
 *  - Verify that the messages' data matches the recorded data.
 *  - Verify that the messages' sizes match the recorded data sizes.
 */
TEST_F(McapFileCreationTest, mcap_data_msgs)
{
    const std::string OUTPUT_FILE_NAME = "mcap_data_msgs";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES = 10;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    const auto sent_messages = record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES);
    auto sent_message = sent_messages.begin();

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    auto read_message_count = 0;

    for (const auto& it : read_messages)
    {
        read_message_count++;

        // Verify the data size
        const auto read_data_size = it.message.dataSize;
        ASSERT_EQ((*sent_message)->length, read_data_size);

        // Verify the data
        const auto read_data = (unsigned char*) reinterpret_cast<unsigned char const*>(it.message.data);
        ASSERT_EQ(strcmp((char*) (*sent_message)->data, (char*) read_data), 0);

        sent_message++;
    }

    // Verify that it read messages
    ASSERT_GT(read_message_count, 0);
}

/**
 * Verify that the DDS Recorder records topics properly in an MCAP file.
 *
 * CASES:
 *  - Verify that the topic's name matches the recorded topic's name.
 *  - Verify that the topic's type matches the recorded topic's type.
 */
TEST_F(McapFileCreationTest, mcap_dds_topic)
{
    const std::string OUTPUT_FILE_NAME = "mcap_dds_topic";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES = 10;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    auto read_message_count = 0;

    for (const auto& read_message : read_messages)
    {
        read_message_count++;

        // Verify the topic's name
        const auto read_topic_name = read_message.channel->topic;
        ASSERT_EQ(topic_->get_name(), read_topic_name);

        // Verify the topic's type
        const auto read_topic_type = read_message.schema->name;
        ASSERT_EQ(topic_->get_type_name(), read_topic_type);
    }

    // Verify that it read messages
    ASSERT_GT(read_message_count, 0);
}

/**
 * Verify that the DDS Recorder records ROS 2 topics properly in an MCAP file.
 *
 * CASES:
 *  - Verify that the topic's name matches the recorded topic's name.
 *  - Verify that the topic's type matches the recorded topic's type.
 */
TEST_F(McapFileCreationTest, mcap_ros2_topic)
{
    const std::string OUTPUT_FILE_NAME = "mcap_ros2_topic";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES = 10;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    configuration_->ros2_types = true;

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    auto read_message_count = 0;

    for (const auto& read_message : read_messages)
    {
        read_message_count++;

        // Verify the topic's name
        const auto read_topic_name = read_message.channel->topic;
        ASSERT_EQ(utils::demangle_if_ros_topic(topic_->get_name()), read_topic_name);

        // Verify the topic's type
        const auto read_topic_type = read_message.schema->name;
        ASSERT_EQ(utils::demangle_if_ros_topic(topic_->get_type_name()), read_topic_type);
    }

    // Verify that it read messages
    ASSERT_GT(read_message_count, 0);
}

TEST_F(McapFileCreationTest, mcap_data_num_msgs)
{
    const std::string OUTPUT_FILE_NAME = "mcap_data_num_msgs";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES = 10;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    // Verify the recorded messages count
    const auto read_messages_count = count_messages_(read_messages);
    ASSERT_EQ(read_messages_count, NUMBER_OF_MESSAGES);
}

TEST_F(McapFileCreationTest, mcap_data_num_msgs_downsampling)
{
    const std::string OUTPUT_FILE_NAME = "mcap_data_num_msgs_downsampling";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES = 10;
    constexpr int DOWNSAMPLING = 2;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // TODO: Change mechanism setting topic qos' default values from specs
    configuration_->topic_qos.downsampling = DOWNSAMPLING;
    ddspipe::core::types::TopicQoS::default_topic_qos.set_value(configuration_->topic_qos);

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    // Verify the recorded messages count
    const auto read_messages_count = count_messages_(read_messages);
    const auto expected_messages = (NUMBER_OF_MESSAGES / DOWNSAMPLING) + (NUMBER_OF_MESSAGES % DOWNSAMPLING);
    ASSERT_EQ(read_messages_count, expected_messages);
}

// //////////////////////
// // With transitions //
// //////////////////////

/**
 * @brief Verify that the DDS Recorder records properly in an MCAP file in RUNNING state.
 *
 * Since the recorder is in RUNNING state, it should record all messages.
 * NOTE: The recorder won't change states since both \c STATE_1 and \c STATE_2 are RUNNING.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(McapFileCreationTest, transition_running)
{
    const std::string OUTPUT_FILE_NAME = "transition_running";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::RUNNING;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::RUNNING;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    // Verify the recorded messages count
    const auto read_messages_count = count_messages_(read_messages);
    ASSERT_EQ(read_messages_count, NUMBER_OF_MESSAGES_1 + NUMBER_OF_MESSAGES_2);
}

/**
 * @brief Verify that the DDS Recorder records properly in an MCAP file in PAUSED state.
 *
 * Since the recorder is in PAUSED state, it should not record any messages.
 * NOTE: The recorder won't change states since both \c STATE_1 and \c STATE_2 are PAUSED.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(McapFileCreationTest, transition_paused)
{
    const std::string OUTPUT_FILE_NAME = "transition_paused";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    // Verify the recorded messages count
    const auto read_messages_count = count_messages_(read_messages);
    ASSERT_EQ(read_messages_count, 0);
}

/**
 * @brief Verify that the DDS Recorder records properly in an MCAP file in SUSPENDED state.
 *
 * Since the recorder is in SUSPENDED state, it should not record any messages.
 * NOTE: The recorder won't change states since both \c STATE_1 and \c STATE_2 are SUSPENDED.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(McapFileCreationTest, transition_suspended)
{
    const std::string OUTPUT_FILE_NAME = "transition_suspended";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::SUSPENDED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::SUSPENDED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Verify that the MCAP file wasn't created
    ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATH));
}

/**
 * @brief Verify that the DDS Recorder records properly in an MCAP file in STOPPED state.
 *
 * Since the recorder is in STOPPED state, it should not record any messages.
 * NOTE: The recorder won't change states since both \c STATE_1 and \c STATE_2 are STOPPED.
 *
 * CASES:
 * - Verify that the message count matches the recorded message count.
 */
TEST_F(McapFileCreationTest, transition_stopped)
{
    const std::string OUTPUT_FILE_NAME = "transition_stopped";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::STOPPED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::STOPPED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Verify that the MCAP file wasn't created
    ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATH));
}

/**
 * @brief Verify that the DDS Recorder records properly in an MCAP file after transitioning from RUNNING to PAUSED.
 *
 * The recorder should record all messages while in RUNNING state and none while in PAUSED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(McapFileCreationTest, transition_running_paused)
{
    const std::string OUTPUT_FILE_NAME = "transition_running_paused";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::RUNNING;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    // Verify the recorded messages count
    const auto read_messages_count = count_messages_(read_messages);
    ASSERT_EQ(read_messages_count, NUMBER_OF_MESSAGES_1);
}

/**
 * @brief Verify that the DDS Recorder records properly in an MCAP file after transitioning from RUNNING to SUSPENDED.
 *
 * The recorder should record all messages while in RUNNING state and none while in SUSPENDED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(McapFileCreationTest, transition_running_suspended)
{
    const std::string OUTPUT_FILE_NAME = "transition_running_suspended";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::RUNNING;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::SUSPENDED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    // Verify the recorded messages count
    const auto read_messages_count = count_messages_(read_messages);
    ASSERT_EQ(read_messages_count, NUMBER_OF_MESSAGES_1);
}

/**
 * @brief Verify that the DDS Recorder records properly in an MCAP file after transitioning from RUNNING to STOPPED.
 *
 * The recorder should record all messages while in RUNNING state and none while in STOPPED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(McapFileCreationTest, transition_running_stopped)
{
    const std::string OUTPUT_FILE_NAME = "transition_running_stopped";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::RUNNING;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::STOPPED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    // Verify the recorded messages count
    const auto read_messages_count = count_messages_(read_messages);
    ASSERT_EQ(read_messages_count, NUMBER_OF_MESSAGES_1);
}

/**
 * @brief Verify that the DDS Recorder records properly in an MCAP file after transitioning from PAUSED to RUNNING.
 *
 * The recorder should not record any messages while in PAUSED state and all messages while in RUNNING state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(McapFileCreationTest, transition_paused_running)
{
    const std::string OUTPUT_FILE_NAME = "transition_paused_running";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::RUNNING;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    // Verify the recorded messages count
    const auto read_messages_count = count_messages_(read_messages);
    ASSERT_EQ(read_messages_count, NUMBER_OF_MESSAGES_2);
}

/**
 * @brief Verify that the DDS Recorder records properly in an MCAP file after transitioning from PAUSED to SUSPENDED.
 *
 * The recorder should not record any messages while in PAUSED state or while in SUSPENDED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(McapFileCreationTest, transition_paused_suspended)
{
    const std::string OUTPUT_FILE_NAME = "transition_paused_suspended";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::SUSPENDED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    // Verify the recorded messages count
    const auto read_messages_count = count_messages_(read_messages);
    ASSERT_EQ(read_messages_count, 0);
}

/**
 * @brief Verify that the DDS Recorder records properly in an MCAP file after transitioning from PAUSED to STOPPED.
 *
 * The recorder should not record any messages while in PAUSED state or while in STOPPED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(McapFileCreationTest, transition_paused_stopped)
{
    const std::string OUTPUT_FILE_NAME = "transition_paused_stopped";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::STOPPED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    // Verify the recorded messages count
    const auto read_messages_count = count_messages_(read_messages);
    ASSERT_EQ(read_messages_count, 0);
}

/**
 * @brief Verify that the DDS Recorder records properly in an MCAP file after transitioning from SUSPENDED to RUNNING.
 *
 * The recorder should not record any messages while in SUSPENDED state and all messages while in RUNNING state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(McapFileCreationTest, transition_suspended_running)
{
    const std::string OUTPUT_FILE_NAME = "transition_suspended_running";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::SUSPENDED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::RUNNING;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    // Verify the recorded messages count
    const auto read_messages_count = count_messages_(read_messages);
    ASSERT_EQ(read_messages_count, NUMBER_OF_MESSAGES_2);
}

/**
 * @brief Verify that the DDS Recorder records properly in an MCAP file after transitioning from SUSPENDED to PAUSED.
 *
 * The recorder should not record any messages while in SUSPENDED state or while in PAUSED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(McapFileCreationTest, transition_suspended_paused)
{
    const std::string OUTPUT_FILE_NAME = "transition_suspended_paused";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::SUSPENDED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    // Verify the recorded messages count
    const auto read_messages_count = count_messages_(read_messages);
    ASSERT_EQ(read_messages_count, 0);
}

/**
 * @brief Verify that the DDS Recorder records properly in an MCAP file after transitioning from SUSPENDED to STOPPED.
 *
 * The recorder should not record any messages while in SUSPENDED state or while in STOPPED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(McapFileCreationTest, transition_suspended_stopped)
{
    const std::string OUTPUT_FILE_NAME = "transition_suspended_stopped";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::SUSPENDED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::STOPPED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Verify that the MCAP file wasn't created
    ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATH));
}

/**
 * @brief Verify that the DDS Recorder records properly in an MCAP file after transitioning from STOPPED to RUNNING.
 *
 * The recorder should not record any messages while in STOPPED state and all messages while in RUNNING state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(McapFileCreationTest, transition_stopped_running)
{
    const std::string OUTPUT_FILE_NAME = "transition_stopped_running";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::STOPPED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::RUNNING;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    // Verify the recorded messages count
    const auto read_messages_count = count_messages_(read_messages);
    ASSERT_EQ(read_messages_count, NUMBER_OF_MESSAGES_2);
}

/**
 * @brief Verify that the DDS Recorder records properly in an MCAP file after transitioning from STOPPED to PAUSED.
 *
 * The recorder should not record any messages while in STOPPED state or while in PAUSED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(McapFileCreationTest, transition_stopped_paused)
{
    const std::string OUTPUT_FILE_NAME = "transition_stopped_paused";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::STOPPED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    // Verify the recorded messages count
    const auto read_messages_count = count_messages_(read_messages);
    ASSERT_EQ(read_messages_count, 0);
}

/**
 * @brief Verify that the DDS Recorder records properly in an MCAP file after transitioning from STOPPED to SUSPENDED.
 *
 * The recorder should not record any messages while in STOPPED state or while in SUSPENDED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(McapFileCreationTest, transition_stopped_suspended)
{
    const std::string OUTPUT_FILE_NAME = "transition_stopped_suspended";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::STOPPED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::SUSPENDED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Verify that the MCAP file wasn't created
    ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATH));
}


// //////////////////
// // Event window //
// //////////////////

/**
 * @brief Verify that the DDS Recorder in PAUSED state records properly in an MCAP file with an \c EVENT_WINDOW and a
 * small \c WAIT between the two batches of messages being sent.
 *
 * The recorder should record all messages.
 * WARNING: This test could fail due to two race conditions.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 *  - Verify that the oldest recorded message was recorded in the event window.
 */
TEST_F(McapFileCreationTest, transition_paused_event_less_window)
{
    const std::string OUTPUT_FILE_NAME = "transition_paused_event_less_window";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;
    constexpr auto EVENT_WINDOW = 3;
    constexpr auto WAIT = 1;
    constexpr auto EVENT = EventKind::EVENT;

    configuration_->event_window = EVENT_WINDOW;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2, WAIT, EVENT);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    // Verify the recorded messages count
    const auto read_messages_count = count_messages_(read_messages);
    ASSERT_EQ(read_messages_count, NUMBER_OF_MESSAGES_1 + NUMBER_OF_MESSAGES_2);

    // Verify the oldest recorded message was recorded in the event window
    const auto max_time_past = find_max_time_past_(read_messages);
    ASSERT_LE(max_time_past, EVENT_WINDOW);
}

/**
 * @brief Verify that the DDS Recorder in PAUSED state records properly in an MCAP file with an \c EVENT_WINDOW and a
 * \c WAIT as long as the \c EVENT_WINDOW between the two batches of messages being sent.
 *
 * The recorder should record the second batch of messages.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 *  - Verify that the oldest recorded message was recorded in the event window.
 */
TEST_F(McapFileCreationTest, transition_paused_event_max_window)
{
    const std::string OUTPUT_FILE_NAME = "transition_paused_event_max_window";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;
    constexpr auto EVENT_WINDOW = 3;
    constexpr auto WAIT = EVENT_WINDOW;
    constexpr auto EVENT = EventKind::EVENT;

    configuration_->event_window = EVENT_WINDOW;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2, WAIT, EVENT);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    // Verify the recorded messages count
    const auto read_messages_count = count_messages_(read_messages);
    ASSERT_EQ(read_messages_count, NUMBER_OF_MESSAGES_2);

    // Verify the oldest recorded message was recorded in the event window
    const auto max_time_past = find_max_time_past_(read_messages);
    ASSERT_LE(max_time_past, EVENT_WINDOW);
}

// ////////////
// // Events //
// ////////////

/**
 * @brief Verify that the DDS Recorder in PAUSED state records properly in an MCAP file with an \c EVENT_START.
 *
 * The recorder should record all messages.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 *  - Verify that the oldest recorded message was recorded in the event window.
 */
TEST_F(McapFileCreationTest, transition_paused_event_start)
{
    const std::string OUTPUT_FILE_NAME = "transition_paused_event_start";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;
    constexpr auto EVENT_WINDOW = 3;
    constexpr auto WAIT = 3;
    constexpr auto EVENT = EventKind::EVENT_START;

    configuration_->event_window = EVENT_WINDOW;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2, WAIT, EVENT);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    // Verify the recorded messages count
    const auto read_messages_count = count_messages_(read_messages);
    ASSERT_EQ(read_messages_count, NUMBER_OF_MESSAGES_2);

    // Verify the oldest recorded message was recorded in the event window
    const auto max_time_past = find_max_time_past_(read_messages);
    ASSERT_LE(max_time_past, EVENT_WINDOW);
}

/**
 * @brief Verify that the DDS Recorder in PAUSED state records properly in an MCAP file with an \c EVENT_SUSPEND.
 *
 * The recorder should record all messages.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 *  - Verify that the oldest recorded message was recorded in the event window.
 */
TEST_F(McapFileCreationTest, transition_paused_event_suspend)
{
    const std::string OUTPUT_FILE_NAME = "transition_paused_event_suspend";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;
    constexpr auto EVENT_WINDOW = 3;
    constexpr auto WAIT = 3;
    constexpr auto EVENT = EventKind::EVENT_SUSPEND;

    configuration_->event_window = EVENT_WINDOW;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2, WAIT, EVENT);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    // Verify the recorded messages count
    const auto read_messages_count = count_messages_(read_messages);
    ASSERT_EQ(read_messages_count, NUMBER_OF_MESSAGES_2);

    // Verify the oldest recorded message was recorded in the event window
    const auto max_time_past = find_max_time_past_(read_messages);
    ASSERT_LE(max_time_past, EVENT_WINDOW);
}

/**
 * @brief Verify that the DDS Recorder in PAUSED state records properly in an MCAP file with an \c EVENT_STOP.
 *
 * The recorder should record all messages.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 *  - Verify that the oldest recorded message was recorded in the event window.
 */
TEST_F(McapFileCreationTest, transition_paused_event_stop)
{
    const std::string OUTPUT_FILE_NAME = "transition_paused_event_stop";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;
    constexpr auto EVENT_WINDOW = 3;
    constexpr auto WAIT = 3;
    constexpr auto EVENT = EventKind::EVENT_STOP;

    configuration_->event_window = EVENT_WINDOW;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2, WAIT, EVENT);

    // Read the recorded messages
    auto read_messages = read_messages_(OUTPUT_FILE_PATH);

    // Verify the recorded messages count
    const auto read_messages_count = count_messages_(read_messages);
    ASSERT_EQ(read_messages_count, NUMBER_OF_MESSAGES_2);

    // Verify the oldest recorded message was recorded in the event window
    const auto max_time_past = find_max_time_past_(read_messages);
    ASSERT_LE(max_time_past, EVENT_WINDOW);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
