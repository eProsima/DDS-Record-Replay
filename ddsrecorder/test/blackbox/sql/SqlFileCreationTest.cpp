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


#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <sqlite3.h>

#include <gtest/gtest.h>

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/Formatter.hpp>
#include <cpp_utils/ros2_mangling.hpp>
#include <cpp_utils/testing/gtest_aux.hpp>

#include <ddspipe_core/types/dds/TopicQoS.hpp>

#include <ddsrecorder_participants/common/time_utils.hpp>
#include <ddsrecorder_participants/recorder/output/OutputSettings.hpp>

#include <tool/DdsRecorder.hpp>

#include "../constants.hpp"
#include "../FileCreationTest.hpp"

using namespace eprosima;

class SqlFileCreationTest : public FileCreationTest
{
public:

    void SetUp() override
    {
        FileCreationTest::SetUp();

        // Set the output library to SQL
        configuration_->output_library = ddsrecorder::participants::OutputLibrary::sql;
    }

protected:

    void exec_sql_statement_(
            const std::string& file_path,
            const std::string& statement,
            const std::vector<std::string>& bind_values,
            const std::function<void(sqlite3_stmt*)>& process_row)
    {
        sqlite3* database;

        // Open the SQL database
        const auto open_ret = sqlite3_open(file_path.c_str(), &database);

        if (open_ret != SQLITE_OK)
        {
            const std::string error_msg = utils::Formatter() << "Failed to open SQL file " << file_path
                                                            << " for reading: " << sqlite3_errmsg(database);
            sqlite3_close(database);

            throw std::runtime_error(error_msg);
        }

        sqlite3_stmt* stmt;

        // Prepare the SQL statement
        const auto ret = sqlite3_prepare_v2(database, statement.c_str(), -1, &stmt, nullptr);

        if (ret != SQLITE_OK)
        {
            const std::string error_msg = utils::Formatter() << "Failed to prepare SQL statement: "
                                                            << sqlite3_errmsg(database);
            sqlite3_finalize(stmt);

            logError(DDSREPLAYER_SQL_READER_PARTICIPANT, "FAIL_SQL_READ | " << error_msg);
            throw std::runtime_error(error_msg);
        }

        // Guard the statement to ensure it's always finalized
        std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt_guard(stmt, sqlite3_finalize);

        // Bind the values to the statement
        for (int i = 0; i < (int) bind_values.size(); i++)
        {
            const auto bind_ret = sqlite3_bind_text(stmt, i+1, bind_values[i].c_str(), -1, SQLITE_STATIC);

            if (bind_ret != SQLITE_OK)
            {
                const std::string error_msg = utils::Formatter() << "Failed to bind SQL statement to read messages: "
                                                                << sqlite3_errmsg(database);

                logError(DDSREPLAYER_SQL_READER_PARTICIPANT, "FAIL_SQL_READ | " << error_msg);
                throw std::runtime_error(error_msg);
            }
        }

        // Step through the statement and process the rows
        int step_ret;

        while ((step_ret = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            process_row(stmt);
        }

        if (step_ret != SQLITE_DONE)
        {
            const std::string error_msg = utils::Formatter() << "Failed to fetch data: "
                                                            << sqlite3_errmsg(database);

            logError(DDSREPLAYER_SQL_READER_PARTICIPANT, "FAIL_SQL_READ | " << error_msg);
            throw std::runtime_error(error_msg);
        }

        // Close the database
        sqlite3_close(database);
    }
};

/**
 * Verify that the DDS Recorder records properly in an SQL file.
 *
 * CASES:
 *  - Verify that the messages' sizes match the recorded data sizes.
 *  - Verify that the messages' data matches the recorded data.
 */
TEST_F(SqlFileCreationTest, sql_data_msgs)
{
    const std::string OUTPUT_FILE_NAME = "sql_data_msgs";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES = 10;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    auto sent_messages = record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES);

    auto sent_message = sent_messages.begin();

    auto read_message_count = 0;

    // Read the recorded messages
    exec_sql_statement_(
        OUTPUT_FILE_PATH.string(),
        "SELECT data, data_size FROM Messages ORDER BY log_time;", {}, [&](sqlite3_stmt* stmt)
    {
        read_message_count++;

        // Verify the data size
        const auto read_data_size = sqlite3_column_int(stmt, 1);
        ASSERT_EQ((*sent_message)->length, read_data_size);

        // Verify the data
        const auto read_data = (unsigned char*) reinterpret_cast<unsigned char const*>(sqlite3_column_blob(stmt, 0));
        ASSERT_EQ(strcmp((char*) (*sent_message)->data, (char*) read_data), 0);

        sent_message++;
    });

    // Verify that it read messages
    ASSERT_GT(read_message_count, 0);
}

/**
 * Verify that the DDS Recorder records topics properly in an SQL file.
 *
 * CASES:
 *  - Verify that the topic's name matches the recorded topic's name.
 *  - Verify that the topic's type matches the recorded topic's type.
 */
TEST_F(SqlFileCreationTest, sql_dds_topic)
{
    const std::string OUTPUT_FILE_NAME = "sql_dds_topic";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES = 10;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES);

    // Read the recorded topics
    auto read_topics_count = 0;

    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT name, type FROM Topics;", {}, [&](sqlite3_stmt* stmt)
    {
        read_topics_count++;

        // Verify the topic's name
        const std::string read_topic_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        ASSERT_EQ(topic_->get_name(), read_topic_name);

        // Verify the topic's type
        const std::string read_topic_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        ASSERT_EQ(topic_->get_type_name(), read_topic_type);
    });

    // Verify that it read messages
    ASSERT_GT(read_topics_count, 0);
}

/**
 * Verify that the DDS Recorder records ROS 2 topics properly in an SQL file.
 *
 * CASES:
 *  - Verify that the topic's name matches the recorded topic's name.
 *  - Verify that the topic's type matches the recorded topic's type.
 */
TEST_F(SqlFileCreationTest, sql_ros2_topic)
{
    const std::string OUTPUT_FILE_NAME = "sql_ros2_topic";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES = 10;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    configuration_->ros2_types = true;

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES);

    // Read the recorded topics
    auto read_topics_count = 0;

    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT name, type FROM Topics;", {}, [&](sqlite3_stmt* stmt)
    {
        read_topics_count++;

        // Verify the topic's name
        const std::string read_topic_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        ASSERT_EQ(utils::demangle_if_ros_topic(topic_->get_name()), read_topic_name);

        // Verify the topic's type
        const std::string read_topic_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        ASSERT_EQ(utils::demangle_if_ros_type(topic_->get_type_name()), read_topic_type);
    });

    // Verify that it read topics
    ASSERT_GT(read_topics_count, 0);
}

/**
 * Verify that the DDS Recorder records every message in an SQL file.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationTest, sql_data_num_msgs)
{
    const std::string OUTPUT_FILE_NAME = "sql_data_num_msgs";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES = 128;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the recorded messages count
        const auto recorded_messages = sqlite3_column_int(stmt, 0);
        ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES);
    });
}

/**
 * Verify that the DDS Recorder records every message in an SQL file.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationTest, sql_data_num_msgs_downsampling)
{
    const std::string OUTPUT_FILE_NAME = "sql_data_num_msgs_downsampling";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES = 10;
    constexpr int DOWNSAMPLING = 2;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // TODO: Change mechanism setting topic qos' default values from specs
    configuration_->topic_qos.downsampling = DOWNSAMPLING;
    ddspipe::core::types::TopicQoS::default_topic_qos.set_value(configuration_->topic_qos);

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the recorded messages count
        const auto recorded_messages = sqlite3_column_int(stmt, 0);
        const auto expected_messages = (NUMBER_OF_MESSAGES / DOWNSAMPLING) + (NUMBER_OF_MESSAGES % DOWNSAMPLING);
        ASSERT_EQ(recorded_messages, expected_messages);
    });
}

// //////////////////////
// // With transitions //
// //////////////////////

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file in RUNNING state.
 *
 * Since the recorder is in RUNNING state, it should record all messages.
 * NOTE: The recorder won't change states since both \c STATE_1 and \c STATE_2 are RUNNING.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationTest, transition_running)
{
    const std::string OUTPUT_FILE_NAME = "transition_running";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::RUNNING;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::RUNNING;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the recorded messages count
        const auto recorded_messages = sqlite3_column_int(stmt, 0);
        ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_1 + NUMBER_OF_MESSAGES_2);
    });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file in PAUSED state.
 *
 * Since the recorder is in PAUSED state, it should not record any messages.
 * NOTE: The recorder won't change states since both \c STATE_1 and \c STATE_2 are PAUSED.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationTest, transition_paused)
{
    const std::string OUTPUT_FILE_NAME = "transition_paused";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the recorded messages count
        const auto recorded_messages = sqlite3_column_int(stmt, 0);
        ASSERT_EQ(recorded_messages, 0);
    });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file in SUSPENDED state.
 *
 * Since the recorder is in SUSPENDED state, it should not record any messages.
 * NOTE: The recorder won't change states since both \c STATE_1 and \c STATE_2 are SUSPENDED.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationTest, transition_suspended)
{
    const std::string OUTPUT_FILE_NAME = "transition_suspended";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::SUSPENDED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::SUSPENDED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Verify that the SQL file wasn't created
    ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATH));
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file in STOPPED state.
 *
 * Since the recorder is in STOPPED state, it should not record any messages.
 * NOTE: The recorder won't change states since both \c STATE_1 and \c STATE_2 are STOPPED.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationTest, transition_stopped)
{
    const std::string OUTPUT_FILE_NAME = "transition_stopped";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::STOPPED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::STOPPED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Verify that the SQL file wasn't created
    ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATH));
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from RUNNING to PAUSED.
 *
 * The recorder should record all messages while in RUNNING state and none while in PAUSED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationTest, transition_running_paused)
{
    const std::string OUTPUT_FILE_NAME = "transition_running_paused";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::RUNNING;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the recorded messages count
        const auto recorded_messages = sqlite3_column_int(stmt, 0);
        ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_1);
    });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from RUNNING to SUSPENDED.
 *
 * The recorder should record all messages while in RUNNING state and none while in SUSPENDED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationTest, transition_running_suspended)
{
    const std::string OUTPUT_FILE_NAME = "transition_running_suspended";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::RUNNING;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::SUSPENDED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the recorded messages count
        const auto recorded_messages = sqlite3_column_int(stmt, 0);
        ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_1);
    });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from RUNNING to STOPPED.
 *
 * The recorder should record all messages while in RUNNING state and none while in STOPPED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationTest, transition_running_stopped)
{
    const std::string OUTPUT_FILE_NAME = "transition_running_stopped";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::RUNNING;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::STOPPED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the recorded messages count
        const auto recorded_messages = sqlite3_column_int(stmt, 0);
        ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_1);
    });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from PAUSED to RUNNING.
 *
 * The recorder should not record any messages while in PAUSED state and all messages while in RUNNING state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationTest, transition_paused_running)
{
    const std::string OUTPUT_FILE_NAME = "transition_paused_running";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::RUNNING;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the recorded messages count
        const auto recorded_messages = sqlite3_column_int(stmt, 0);
        ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_2);
    });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from PAUSED to SUSPENDED.
 *
 * The recorder should not record any messages while in PAUSED state or while in SUSPENDED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationTest, transition_paused_suspended)
{
    const std::string OUTPUT_FILE_NAME = "transition_paused_suspended";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::SUSPENDED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the recorded messages count
        const auto recorded_messages = sqlite3_column_int(stmt, 0);
        ASSERT_EQ(recorded_messages, 0);
    });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from PAUSED to STOPPED.
 *
 * The recorder should not record any messages while in PAUSED state or while in STOPPED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationTest, transition_paused_stopped)
{
    const std::string OUTPUT_FILE_NAME = "transition_paused_stopped";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::STOPPED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the recorded messages count
        const auto recorded_messages = sqlite3_column_int(stmt, 0);
        ASSERT_EQ(recorded_messages, 0);
    });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from SUSPENDED to RUNNING.
 *
 * The recorder should not record any messages while in SUSPENDED state and all messages while in RUNNING state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationTest, transition_suspended_running)
{
    const std::string OUTPUT_FILE_NAME = "transition_suspended_running";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::SUSPENDED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::RUNNING;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the recorded messages count
        const auto recorded_messages = sqlite3_column_int(stmt, 0);
        ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_2);
    });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from SUSPENDED to PAUSED.
 *
 * The recorder should not record any messages while in SUSPENDED state or while in PAUSED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationTest, transition_suspended_paused)
{
    const std::string OUTPUT_FILE_NAME = "transition_suspended_paused";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::SUSPENDED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the recorded messages count
        const auto recorded_messages = sqlite3_column_int(stmt, 0);
        ASSERT_EQ(recorded_messages, 0);
    });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from SUSPENDED to STOPPED.
 *
 * The recorder should not record any messages while in SUSPENDED state or while in STOPPED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationTest, transition_suspended_stopped)
{
    const std::string OUTPUT_FILE_NAME = "transition_suspended_stopped";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::SUSPENDED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::STOPPED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Verify that the SQL file wasn't created
    ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATH));
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from STOPPED to RUNNING.
 *
 * The recorder should not record any messages while in STOPPED state and all messages while in RUNNING state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationTest, transition_stopped_running)
{
    const std::string OUTPUT_FILE_NAME = "transition_stopped_running";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::STOPPED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::RUNNING;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the recorded messages count
        const auto recorded_messages = sqlite3_column_int(stmt, 0);
        ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_2);
    });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from STOPPED to PAUSED.
 *
 * The recorder should not record any messages while in STOPPED state or while in PAUSED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationTest, transition_stopped_paused)
{
    const std::string OUTPUT_FILE_NAME = "transition_stopped_paused";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::STOPPED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the recorded messages count
        const auto recorded_messages = sqlite3_column_int(stmt, 0);
        ASSERT_EQ(recorded_messages, 0);
    });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from STOPPED to SUSPENDED.
 *
 * The recorder should not record any messages while in STOPPED state or while in SUSPENDED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationTest, transition_stopped_suspended)
{
    const std::string OUTPUT_FILE_NAME = "transition_stopped_suspended";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::STOPPED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::SUSPENDED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Verify that the SQL file wasn't created
    ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATH));
}


// //////////////////
// // Event window //
// //////////////////

/**
 * @brief Verify that the DDS Recorder in PAUSED state records properly in an SQL file with an \c EVENT_WINDOW and a
 * small \c WAIT between the two batches of messages being sent.
 *
 * The recorder should record all messages.
 * WARNING: This test could fail due to two race conditions.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 *  - Verify that the oldest recorded message was recorded in the event window.
 */
TEST_F(SqlFileCreationTest, transition_paused_event_less_window)
{
    const std::string OUTPUT_FILE_NAME = "transition_paused_event_less_window";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

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

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the recorded messages count
        const auto recorded_messages = sqlite3_column_int(stmt, 0);
        ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_1 + NUMBER_OF_MESSAGES_2);
    });

    const auto now = utils::now();
    const auto now_tks = ddsrecorder::participants::to_ticks(now);

    // Find the oldest recorded message
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT MIN(log_time) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the oldest recorded message was recorded in the event window
        const auto log_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const auto log_time_ts = ddsrecorder::participants::to_std_timestamp(log_time);
        const auto log_time_tks = ddsrecorder::participants::to_ticks(log_time_ts);

        const auto NS_TO_SEC = pow(10, -9);
        const auto max_time_past = (now_tks - log_time_tks) * NS_TO_SEC;

        ASSERT_LE(max_time_past, EVENT_WINDOW);
    });
}

/**
 * @brief Verify that the DDS Recorder in PAUSED state records properly in an SQL file with an \c EVENT_WINDOW and a
 * \c WAIT as long as the \c EVENT_WINDOW between the two batches of messages being sent.
 *
 * The recorder should record the second batch of messages.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 *  - Verify that the oldest recorded message was recorded in the event window.
 */
TEST_F(SqlFileCreationTest, transition_paused_event_max_window)
{
    const std::string OUTPUT_FILE_NAME = "transition_paused_event_max_window";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

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

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the recorded messages count
        const auto recorded_messages = sqlite3_column_int(stmt, 0);
        ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_2);
    });

    const auto now = utils::now();
    const auto now_tks = ddsrecorder::participants::to_ticks(now);

    // Find the oldest recorded message
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT MIN(log_time) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the oldest recorded message was recorded in the event window
        const auto log_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const auto log_time_ts = ddsrecorder::participants::to_std_timestamp(log_time);
        const auto log_time_tks = ddsrecorder::participants::to_ticks(log_time_ts);

        const auto NS_TO_SEC = pow(10, -9);
        const auto max_time_past = (now_tks - log_time_tks) * NS_TO_SEC;

        ASSERT_LE(max_time_past, EVENT_WINDOW);
    });
}

// ////////////
// // Events //
// ////////////

/**
 * @brief Verify that the DDS Recorder in PAUSED state records properly in an SQL file with an \c EVENT_START.
 *
 * The recorder should record all messages.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 *  - Verify that the oldest recorded message was recorded in the event window.
 */
TEST_F(SqlFileCreationTest, transition_paused_event_start)
{
    const std::string OUTPUT_FILE_NAME = "transition_paused_event_start";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

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

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the recorded messages count
        const auto recorded_messages = sqlite3_column_int(stmt, 0);
        ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_2);
    });

    const auto now = utils::now();
    const auto now_tks = ddsrecorder::participants::to_ticks(now);

    // Find the oldest recorded message
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT MIN(log_time) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the oldest recorded message was recorded in the event window
        const auto log_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const auto log_time_ts = ddsrecorder::participants::to_std_timestamp(log_time);
        const auto log_time_tks = ddsrecorder::participants::to_ticks(log_time_ts);

        const auto NS_TO_SEC = pow(10, -9);
        const auto max_time_past = (now_tks - log_time_tks) * NS_TO_SEC;

        ASSERT_LE(max_time_past, EVENT_WINDOW);
    });
}

/**
 * @brief Verify that the DDS Recorder in PAUSED state records properly in an SQL file with an \c EVENT_SUSPEND.
 *
 * The recorder should record all messages.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 *  - Verify that the oldest recorded message was recorded in the event window.
 */
TEST_F(SqlFileCreationTest, transition_paused_event_suspend)
{
    const std::string OUTPUT_FILE_NAME = "transition_paused_event_suspend";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

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

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the recorded messages count
        const auto recorded_messages = sqlite3_column_int(stmt, 0);
        ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_2);
    });

    const auto now = utils::now();
    const auto now_tks = ddsrecorder::participants::to_ticks(now);

    // Find the oldest recorded message
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT MIN(log_time) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the oldest recorded message was recorded in the event window
        const auto log_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const auto log_time_ts = ddsrecorder::participants::to_std_timestamp(log_time);
        const auto log_time_tks = ddsrecorder::participants::to_ticks(log_time_ts);

        const auto NS_TO_SEC = pow(10, -9);
        const auto max_time_past = (now_tks - log_time_tks) * NS_TO_SEC;

        ASSERT_LE(max_time_past, EVENT_WINDOW);
    });
}

/**
 * @brief Verify that the DDS Recorder in PAUSED state records properly in an SQL file with an \c EVENT_STOP.
 *
 * The recorder should record all messages.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 *  - Verify that the oldest recorded message was recorded in the event window.
 */
TEST_F(SqlFileCreationTest, transition_paused_event_stop)
{
    const std::string OUTPUT_FILE_NAME = "transition_paused_event_stop";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

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

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the recorded messages count
        const auto recorded_messages = sqlite3_column_int(stmt, 0);
        ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_2);
    });

    const auto now = utils::now();
    const auto now_tks = ddsrecorder::participants::to_ticks(now);

    // Find the oldest recorded message
    exec_sql_statement_(OUTPUT_FILE_PATH.string(), "SELECT MIN(log_time) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
    {
        // Verify the oldest recorded message was recorded in the event window
        const auto log_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const auto log_time_ts = ddsrecorder::participants::to_std_timestamp(log_time);
        const auto log_time_tks = ddsrecorder::participants::to_ticks(log_time_ts);

        const auto NS_TO_SEC = pow(10, -9);
        const auto max_time_past = (now_tks - log_time_tks) * NS_TO_SEC;

        ASSERT_LE(max_time_past, EVENT_WINDOW);
    });
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
