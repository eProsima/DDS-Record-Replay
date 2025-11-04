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

#include <sqlite/sqlite3.h>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/Formatter.hpp>
#include <cpp_utils/ros2_mangling.hpp>

#include <ddspipe_core/types/dds/TopicQoS.hpp>

#include <ddsrecorder_participants/common/time_utils.hpp>
#include <ddsrecorder_participants/recorder/output/OutputSettings.hpp>

#include <tool/DdsRecorder.hpp>

#include "../constants.hpp"
#include "../FileCreationPartitionTest.hpp"

using namespace eprosima;

class SqlFileCreationPartitionTest : public FileCreationPartitionTest,
    public ::testing::WithParamInterface<std::string>
{
public:

    void SetUp() override
    {
        FileCreationPartitionTest::SetUp();
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

            EPROSIMA_LOG_ERROR(DDSREPLAYER_SQL_READER_PARTICIPANT, "FAIL_SQL_READ | " << error_msg);
            throw std::runtime_error(error_msg);
        }

        {

            // Guard the statement to ensure it's always finalized
            std::unique_ptr<sqlite3_stmt, decltype(& sqlite3_finalize)> stmt_guard(stmt, sqlite3_finalize);

            // Bind the values to the statement
            for (int i = 0; i < (int) bind_values.size(); i++)
            {
                const auto bind_ret = sqlite3_bind_text(stmt, i + 1, bind_values[i].c_str(), -1, SQLITE_STATIC);

                if (bind_ret != SQLITE_OK)
                {
                    const std::string error_msg = utils::Formatter() <<
                            "Failed to bind SQL statement to read messages: "
                                                                     << sqlite3_errmsg(database);

                    EPROSIMA_LOG_ERROR(DDSREPLAYER_SQL_READER_PARTICIPANT, "FAIL_SQL_READ | " << error_msg);
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

                EPROSIMA_LOG_ERROR(DDSREPLAYER_SQL_READER_PARTICIPANT, "FAIL_SQL_READ | " << error_msg);
                throw std::runtime_error(error_msg);
            }
        }

        // Close the database
        sqlite3_close(database);
    }

};

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file in data format CDR.
 *
 * Writer publish with partition = "A"
 *
 * CASES:
 *  - Verify that the messages' data_cdr_size matches the recorded data_cdr sizes.
 *  - Verify that the messages' data_cdr matches the recorded data_cdr.
 *  - Verify that the messages' data_json is empty.
 */
TEST_F(SqlFileCreationPartitionTest, sql_data_format_cdr_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "sql_data_cdr_msgs";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES = 10;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    configuration_->sql_data_format = ddsrecorder::participants::DataFormat::cdr;

    // Record messages
    auto sent_messages = record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES);

    auto sent_message = sent_messages.begin();

    auto read_message_count = 0;

    // Read the recorded messages
    exec_sql_statement_(
        OUTPUT_FILE_PATH,
        "SELECT data_cdr_size, data_cdr, data_json FROM Messages ORDER BY log_time;", {}, [&](sqlite3_stmt* stmt)
        {
            read_message_count++;

            // Verify the data_cdr_size
            const auto read_data_cdr_size = sqlite3_column_int(stmt, 0);
            ASSERT_EQ(to_cdr(*sent_message)->length, read_data_cdr_size);

            // Verify the data_cdr
            const auto read_data_cdr =
            (unsigned char*) reinterpret_cast<unsigned char const*>(sqlite3_column_blob(stmt, 1));
            ASSERT_EQ(strcmp((char*) to_cdr(*sent_message)->data, (char*) read_data_cdr), 0);

            // Verify the data_json
            const std::string read_data_json = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            ASSERT_EQ(read_data_json.size(), 0);

            sent_message++;
        });

    // Verify that it read messages
    ASSERT_GT(read_message_count, 0);
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file in data format JSON.
 *
 * Writer publish with partition = "A"
 *
 * CASES:
 *  - Verify that the messages' data_cdr_size is 0.
 *  - Verify that the messages' data_cdr is empty.
 *  - Verify that the messages' data_json matches the recorded data_json.
 */
TEST_F(SqlFileCreationPartitionTest, sql_data_format_json_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "sql_data_json_msgs";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES = 10;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    configuration_->sql_data_format = ddsrecorder::participants::DataFormat::json;

    // Record messages
    auto sent_messages = record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES);

    auto sent_message = sent_messages.begin();

    auto read_message_count = 0;

    // Read the recorded messages
    exec_sql_statement_(
        OUTPUT_FILE_PATH,
        "SELECT data_cdr_size, data_cdr, data_json FROM Messages ORDER BY log_time;", {}, [&](sqlite3_stmt* stmt)
        {
            read_message_count++;

            // Verify the data_cdr_size
            const auto read_data_cdr_size = sqlite3_column_int(stmt, 0);
            ASSERT_EQ(read_data_cdr_size, 0);

            // Verify the data_cdr
            const auto read_data_cdr = sqlite3_column_bytes(stmt, 1);
            ASSERT_EQ(read_data_cdr, 0);

            // Verify the data_json
            const std::string read_data_json = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            ASSERT_EQ(to_json(*sent_message), read_data_json);

            sent_message++;
        });

    // Verify that it read messages
    ASSERT_GT(read_message_count, 0);
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file in both data formats.
 *
 * Writer publish with partition = "A"
 *
 * CASES:
 *  - Verify that the messages' data_cdr_size matches the recorded data_cdr sizes.
 *  - Verify that the messages' data_cdr matches the recorded data_cdr.
 *  - Verify that the messages' data_json matches the recorded data_json.
 */
TEST_F(SqlFileCreationPartitionTest, sql_data_format_both_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "sql_data_cdr_msgs";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES = 10;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    configuration_->sql_data_format = ddsrecorder::participants::DataFormat::both;

    // Record messages
    auto sent_messages = record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES);

    auto sent_message = sent_messages.begin();

    auto read_message_count = 0;

    // Read the recorded messages
    exec_sql_statement_(
        OUTPUT_FILE_PATH,
        "SELECT data_cdr_size, data_cdr, data_json FROM Messages ORDER BY log_time;", {}, [&](sqlite3_stmt* stmt)
        {
            read_message_count++;

            // Verify the data_cdr_size
            const auto read_data_cdr_size = sqlite3_column_int(stmt, 0);
            ASSERT_EQ(to_cdr(*sent_message)->length, read_data_cdr_size);

            // Verify the data_cdr
            const auto read_data_cdr =
            (unsigned char*) reinterpret_cast<unsigned char const*>(sqlite3_column_blob(stmt, 1));
            ASSERT_EQ(strcmp((char*) to_cdr(*sent_message)->data, (char*) read_data_cdr), 0);

            // Verify the data_json
            const std::string read_data_json = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            ASSERT_EQ(to_json(*sent_message), read_data_json);

            sent_message++;
        });

    // Verify that it read messages
    ASSERT_GT(read_message_count, 0);
}

/**
 * @brief Verify that the DDS Recorder records topics properly in an SQL file.
 *
 * Writer publish with partition = "A"
 *
 * CASES:
 *  - Verify that the topic's name matches the recorded topic's name.
 *  - Verify that the topic's type matches the recorded topic's type.
 */
TEST_F(SqlFileCreationPartitionTest, sql_dds_topic_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "sql_dds_topic";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES = 10;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES);

    // Read the recorded topics
    auto read_topics_count = 0;

    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT name, type FROM Topics;", {}, [&](sqlite3_stmt* stmt)
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
 * @brief Verify that the DDS Recorder records ROS 2 topics properly in an SQL file.
 *
 * Writer publish with partition = "A"
 *
 * CASES:
 *  - Verify that the topic's name matches the recorded topic's name.
 *  - Verify that the topic's type matches the recorded topic's type.
 */
TEST_F(SqlFileCreationPartitionTest, sql_ros2_topic_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "sql_ros2_topic";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES = 10;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    configuration_->ros2_types = true;
    // Recreate the topic with ROS 2 types
    recreate_datawriter_();

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES);

    // Read the recorded topics
    auto read_topics_count = 0;

    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT name, type FROM Topics;", {}, [&](sqlite3_stmt* stmt)
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
 * @brief Verify that the DDS Recorder records every message in an SQL file.
 *
 * Writer publish with partition = "A"
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, sql_data_num_msgs_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "sql_data_num_msgs";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES = 128;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES);
            });
}

/**
 * @brief Verify that the DDS Recorder records every message in an SQL file with DOWNSAMPLING.
 *
 * Writer publish with partition = "A"
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, sql_data_num_msgs_downsampling_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "sql_data_num_msgs_downsampling";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES = 128;
    constexpr int DOWNSAMPLING = 2;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // TODO: Change mechanism setting topic qos' default values from specs
    configuration_->topic_qos.downsampling = DOWNSAMPLING;
    ddspipe::core::types::TopicQoS::default_topic_qos.set_value(configuration_->topic_qos);

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt,
                0);
                const auto expected_messages = (NUMBER_OF_MESSAGES / DOWNSAMPLING) +
                (NUMBER_OF_MESSAGES % DOWNSAMPLING);
                ASSERT_EQ(recorded_messages, expected_messages);
            });
}

// //////////////////////
// // With transitions //
// //////////////////////

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file in RUNNING state.
 *
 * Writer publish with partition = "A"
 *
 * Since the recorder is in RUNNING state, it should record all messages.
 * NOTE: The recorder won't change states since both \c STATE_1 and \c STATE_2 are RUNNING.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_running_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_running";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::RUNNING;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::RUNNING;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_1 + NUMBER_OF_MESSAGES_2);
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file in PAUSED state.
 *
 * Writer publish with partition = "A"
 *
 * Since the recorder is in PAUSED state, it should not record any messages.
 * NOTE: The recorder won't change states since both \c STATE_1 and \c STATE_2 are PAUSED.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_paused_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_paused";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, 0);
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file in SUSPENDED state.
 *
 * Writer publish with partition = "A"
 *
 * Since the recorder is in SUSPENDED state, it should not record any messages.
 * NOTE: The recorder won't change states since both \c STATE_1 and \c STATE_2 are SUSPENDED.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_suspended_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_suspended";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::SUSPENDED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::SUSPENDED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Verify that the SQL file wasn't created
    ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATH));
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file in STOPPED state.
 *
 * Writer publish with partition = "A"
 *
 * Since the recorder is in STOPPED state, it should not record any messages.
 * NOTE: The recorder won't change states since both \c STATE_1 and \c STATE_2 are STOPPED.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_stopped_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_stopped";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::STOPPED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::STOPPED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Verify that the SQL file wasn't created
    ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATH));
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from RUNNING to PAUSED.
 *
 * Writer publish with partition = "A"
 *
 * The recorder should record all messages while in RUNNING state and none while in PAUSED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_running_paused_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_running_paused";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::RUNNING;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_1);
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from RUNNING to SUSPENDED.
 *
 * Writer publish with partition = "A"
 *
 * The recorder should record all messages while in RUNNING state and none while in SUSPENDED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_running_suspended_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_running_suspended";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::RUNNING;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::SUSPENDED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_1);
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from RUNNING to STOPPED.
 *
 * Writer publish with partition = "A"
 *
 * The recorder should record all messages while in RUNNING state and none while in STOPPED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_running_stopped_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_running_stopped";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::RUNNING;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::STOPPED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_1);
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from PAUSED to RUNNING.
 *
 * Writer publish with partition = "A"
 *
 * The recorder should not record any messages while in PAUSED state and all messages while in RUNNING state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_paused_running_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_paused_running";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::RUNNING;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_2);
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from PAUSED to SUSPENDED.
 *
 * Writer publish with partition = "A"
 *
 * The recorder should not record any messages while in PAUSED state or while in SUSPENDED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_paused_suspended_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_paused_suspended";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::SUSPENDED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, 0);
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from PAUSED to STOPPED.
 *
 * Writer publish with partition = "A"
 *
 * The recorder should not record any messages while in PAUSED state or while in STOPPED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_paused_stopped_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_paused_stopped";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::STOPPED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, 0);
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from SUSPENDED to RUNNING.
 *
 * Writer publish with partition = "A"
 *
 * The recorder should not record any messages while in SUSPENDED state and all messages while in RUNNING state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_suspended_running_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_suspended_running";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::SUSPENDED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::RUNNING;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_2);
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from SUSPENDED to PAUSED.
 *
 * Writer publish with partition = "A"
 *
 * The recorder should not record any messages while in SUSPENDED state or while in PAUSED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_suspended_paused_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_suspended_paused";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::SUSPENDED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, 0);
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from SUSPENDED to STOPPED.
 *
 * Writer publish with partition = "A"
 *
 * The recorder should not record any messages while in SUSPENDED state or while in STOPPED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_suspended_stopped_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_suspended_stopped";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::SUSPENDED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::STOPPED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Verify that the SQL file wasn't created
    ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATH));
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from STOPPED to RUNNING.
 *
 * Writer publish with partition = "A"
 *
 * The recorder should not record any messages while in STOPPED state and all messages while in RUNNING state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_stopped_running_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_stopped_running";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::STOPPED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::RUNNING;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_2);
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from STOPPED to PAUSED.
 *
 * Writer publish with partition = "A"
 *
 * The recorder should not record any messages while in STOPPED state or while in PAUSED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_stopped_paused_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_stopped_paused";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::STOPPED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, 0);
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from STOPPED to SUSPENDED.
 *
 * Writer publish with partition = "A"
 *
 * The recorder should not record any messages while in STOPPED state or while in SUSPENDED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_stopped_suspended_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_stopped_suspended";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::STOPPED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::SUSPENDED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

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
 *
 * Writer publish with partition = "A"
 * small \c WAIT between the two batches of messages being sent.
 *
 * The recorder should record all messages.
 * WARNING: This test could fail due to two race conditions.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 *  - Verify that the oldest recorded message was recorded in the event window.
 */
TEST_F(SqlFileCreationPartitionTest, transition_paused_event_less_window_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

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

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2, WAIT, EVENT);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_1 + NUMBER_OF_MESSAGES_2);
            });

    const auto now = utils::now();
    const auto now_tks = ddsrecorder::participants::to_ticks(now);

    // Find the oldest recorded message
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT MIN(log_time) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
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
 *
 * Writer publish with partition = "A"
 * \c WAIT as long as the \c EVENT_WINDOW between the two batches of messages being sent.
 *
 * The recorder should record the second batch of messages.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 *  - Verify that the oldest recorded message was recorded in the event window.
 */
TEST_F(SqlFileCreationPartitionTest, transition_paused_event_max_window_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

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

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2, WAIT, EVENT);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_2);
            });

    const auto now = utils::now();
    const auto now_tks = ddsrecorder::participants::to_ticks(now);

    // Find the oldest recorded message
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT MIN(log_time) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
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
 * Writer publish with partition = "A"
 *
 * The recorder should record all messages.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 *  - Verify that the oldest recorded message was recorded in the event window.
 */
TEST_F(SqlFileCreationPartitionTest, transition_paused_event_start_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

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

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2, WAIT, EVENT);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_2);
            });

    const auto now = utils::now();
    const auto now_tks = ddsrecorder::participants::to_ticks(now);

    // Find the oldest recorded message
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT MIN(log_time) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
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
 * Writer publish with partition = "A"
 *
 * The recorder should record all messages.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 *  - Verify that the oldest recorded message was recorded in the event window.
 */
TEST_F(SqlFileCreationPartitionTest, transition_paused_event_suspend_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

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

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2, WAIT, EVENT);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_2);
            });

    const auto now = utils::now();
    const auto now_tks = ddsrecorder::participants::to_ticks(now);

    // Find the oldest recorded message
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT MIN(log_time) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
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
 * Writer publish with partition = "A"
 *
 * The recorder should record all messages.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 *  - Verify that the oldest recorded message was recorded in the event window.
 */
TEST_F(SqlFileCreationPartitionTest, transition_paused_event_stop_partition)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

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

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2, WAIT, EVENT);

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, NUMBER_OF_MESSAGES_2);
            });

    const auto now = utils::now();
    const auto now_tks = ddsrecorder::participants::to_ticks(now);

    // Find the oldest recorded message
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT MIN(log_time) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
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
 * @brief Verify that the DDS Recorder records properly in an SQL file in data format CDR.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * CASES:
 *  - Verify that the messages' data_cdr_size matches the recorded data_cdr sizes.
 *  - Verify that the messages' data_cdr matches the recorded data_cdr.
 *  - Verify that the messages' data_json is empty.
 */
TEST_F(SqlFileCreationPartitionTest, sql_data_format_cdr_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "sql_data_cdr_msgs";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES = 10;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    configuration_->sql_data_format = ddsrecorder::participants::DataFormat::cdr;

    // Record messages
    auto sent_messages = record_messages_(
        OUTPUT_FILE_NAME,
        NUMBER_OF_MESSAGES,
        DdsRecorderState::RUNNING,
        0,
        DdsRecorderState::RUNNING,
        0,
        EventKind::NO_EVENT,
        "B");

    auto sent_message = sent_messages.begin();

    auto read_message_count = 0;

    // Read the recorded messages
    exec_sql_statement_(
        OUTPUT_FILE_PATH,
        "SELECT data_cdr_size, data_cdr, data_json FROM Messages ORDER BY log_time;", {}, [&](sqlite3_stmt* stmt)
        {
            read_message_count++;

            // Verify the data_cdr_size
            const auto read_data_cdr_size = sqlite3_column_int(stmt, 0);
            ASSERT_EQ(to_cdr(*sent_message)->length, read_data_cdr_size);

            // Verify the data_cdr
            const auto read_data_cdr =
            (unsigned char*) reinterpret_cast<unsigned char const*>(sqlite3_column_blob(stmt, 1));
            ASSERT_EQ(strcmp((char*) to_cdr(*sent_message)->data, (char*) read_data_cdr), 0);

            // Verify the data_json
            const std::string read_data_json = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            ASSERT_EQ(read_data_json.size(), 0);

            sent_message++;
        });

    // Verify that it read messages
    ASSERT_EQ(read_message_count, 0);
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file in data format JSON.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * CASES:
 *  - Verify that the messages' data_cdr_size is 0.
 *  - Verify that the messages' data_cdr is empty.
 *  - Verify that the messages' data_json matches the recorded data_json.
 */
TEST_F(SqlFileCreationPartitionTest, sql_data_format_json_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "sql_data_json_msgs";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES = 10;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    configuration_->sql_data_format = ddsrecorder::participants::DataFormat::json;

    // Record messages
    auto sent_messages = record_messages_(
        OUTPUT_FILE_NAME,
        NUMBER_OF_MESSAGES,
        DdsRecorderState::RUNNING,
        0,
        DdsRecorderState::RUNNING,
        0,
        EventKind::NO_EVENT,
        "B");

    auto sent_message = sent_messages.begin();

    auto read_message_count = 0;

    // Read the recorded messages
    exec_sql_statement_(
        OUTPUT_FILE_PATH,
        "SELECT data_cdr_size, data_cdr, data_json FROM Messages ORDER BY log_time;", {}, [&](sqlite3_stmt* stmt)
        {
            read_message_count++;

            // Verify the data_cdr_size
            const auto read_data_cdr_size = sqlite3_column_int(stmt, 0);
            ASSERT_EQ(read_data_cdr_size, 0);

            // Verify the data_cdr
            const auto read_data_cdr = sqlite3_column_bytes(stmt, 1);
            ASSERT_EQ(read_data_cdr, 0);

            // Verify the data_json
            const std::string read_data_json = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            ASSERT_EQ(to_json(*sent_message), read_data_json);

            sent_message++;
        });

    // Verify that it read messages
    ASSERT_EQ(read_message_count, 0);
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file in both data formats.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * CASES:
 *  - Verify that the messages' data_cdr_size matches the recorded data_cdr sizes.
 *  - Verify that the messages' data_cdr matches the recorded data_cdr.
 *  - Verify that the messages' data_json matches the recorded data_json.
 */
TEST_F(SqlFileCreationPartitionTest, sql_data_format_both_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "sql_data_cdr_msgs";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES = 10;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    configuration_->sql_data_format = ddsrecorder::participants::DataFormat::both;

    // Record messages
    auto sent_messages = record_messages_(
        OUTPUT_FILE_NAME,
        NUMBER_OF_MESSAGES,
        DdsRecorderState::RUNNING,
        0,
        DdsRecorderState::RUNNING,
        0,
        EventKind::NO_EVENT,
        "B");

    auto sent_message = sent_messages.begin();

    auto read_message_count = 0;

    // Read the recorded messages
    exec_sql_statement_(
        OUTPUT_FILE_PATH,
        "SELECT data_cdr_size, data_cdr, data_json FROM Messages ORDER BY log_time;", {}, [&](sqlite3_stmt* stmt)
        {
            read_message_count++;

            // Verify the data_cdr_size
            const auto read_data_cdr_size = sqlite3_column_int(stmt, 0);
            ASSERT_EQ(to_cdr(*sent_message)->length, read_data_cdr_size);

            // Verify the data_cdr
            const auto read_data_cdr =
            (unsigned char*) reinterpret_cast<unsigned char const*>(sqlite3_column_blob(stmt, 1));
            ASSERT_EQ(strcmp((char*) to_cdr(*sent_message)->data, (char*) read_data_cdr), 0);

            // Verify the data_json
            const std::string read_data_json = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            ASSERT_EQ(to_json(*sent_message), read_data_json);

            sent_message++;
        });

    // Verify that it read messages
    ASSERT_EQ(read_message_count, 0);
}

/**
 * @brief Verify that the DDS Recorder records topics properly in an SQL file.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * CASES:
 *  - Verify that the topic's name matches the recorded topic's name.
 *  - Verify that the topic's type matches the recorded topic's type.
 */
TEST_F(SqlFileCreationPartitionTest, sql_dds_topic_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "sql_dds_topic";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES = 10;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(
        OUTPUT_FILE_NAME,
        NUMBER_OF_MESSAGES,
        DdsRecorderState::RUNNING,
        0,
        DdsRecorderState::RUNNING,
        0,
        EventKind::NO_EVENT,
        "B");

    // Read the recorded topics
    auto read_topics_count = 0;

    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT name, type FROM Topics;", {}, [&](sqlite3_stmt* stmt)
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
    ASSERT_EQ(read_topics_count, 0);
}

/**
 * @brief Verify that the DDS Recorder records ROS 2 topics properly in an SQL file.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * CASES:
 *  - Verify that the topic's name matches the recorded topic's name.
 *  - Verify that the topic's type matches the recorded topic's type.
 */
TEST_F(SqlFileCreationPartitionTest, sql_ros2_topic_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "sql_ros2_topic";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES = 10;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    configuration_->ros2_types = true;
    // Recreate the topic with ROS 2 types
    recreate_datawriter_();

    // Record messages
    record_messages_(
        OUTPUT_FILE_NAME,
        NUMBER_OF_MESSAGES,
        DdsRecorderState::RUNNING,
        0,
        DdsRecorderState::RUNNING,
        0,
        EventKind::NO_EVENT,
        "B");

    // Read the recorded topics
    auto read_topics_count = 0;

    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT name, type FROM Topics;", {}, [&](sqlite3_stmt* stmt)
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
    ASSERT_EQ(read_topics_count, 0);
}

/**
 * @brief Verify that the DDS Recorder records every message in an SQL file.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, sql_data_num_msgs_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "sql_data_num_msgs";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES = 128;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(
        OUTPUT_FILE_NAME,
        NUMBER_OF_MESSAGES,
        DdsRecorderState::RUNNING,
        0,
        DdsRecorderState::RUNNING,
        0,
        EventKind::NO_EVENT,
        "B");

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, 0); // the recorder has a filter in "B" and do no store information.
            });
}

/**
 * @brief Verify that the DDS Recorder records every message in an SQL file with DOWNSAMPLING.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, sql_data_num_msgs_downsampling_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "sql_data_num_msgs_downsampling";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES = 128;
    constexpr int DOWNSAMPLING = 2;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // TODO: Change mechanism setting topic qos' default values from specs
    configuration_->topic_qos.downsampling = DOWNSAMPLING;
    ddspipe::core::types::TopicQoS::default_topic_qos.set_value(configuration_->topic_qos);

    // Record messages
    record_messages_(
        OUTPUT_FILE_NAME,
        NUMBER_OF_MESSAGES,
        DdsRecorderState::RUNNING,
        0,
        DdsRecorderState::RUNNING,
        0,
        EventKind::NO_EVENT,
        "B");

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt,
                0);
                const auto expected_messages = (NUMBER_OF_MESSAGES / DOWNSAMPLING) +
                (NUMBER_OF_MESSAGES % DOWNSAMPLING);
                ASSERT_EQ(recorded_messages, 0); // the recorder has a filter in "B" and do no store information.
            });
}

// //////////////////////
// // With transitions //
// //////////////////////

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file in RUNNING state.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * Since the recorder is in RUNNING state, it should record all messages.
 * NOTE: The recorder won't change states since both \c STATE_1 and \c STATE_2 are RUNNING.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_running_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_running";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::RUNNING;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::RUNNING;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2,
            0, EventKind::NO_EVENT, "B");

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, 0); // the recorder has a filter in "B" and do no store information.
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file in PAUSED state.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * Since the recorder is in PAUSED state, it should not record any messages.
 * NOTE: The recorder won't change states since both \c STATE_1 and \c STATE_2 are PAUSED.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_paused_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_paused";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2,
            0, EventKind::NO_EVENT, "B");

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, 0);
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file in SUSPENDED state.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * Since the recorder is in SUSPENDED state, it should not record any messages.
 * NOTE: The recorder won't change states since both \c STATE_1 and \c STATE_2 are SUSPENDED.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_suspended_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_suspended";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::SUSPENDED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::SUSPENDED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2,
            0, EventKind::NO_EVENT, "B");

    // Verify that the SQL file wasn't created
    ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATH));
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file in STOPPED state.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * Since the recorder is in STOPPED state, it should not record any messages.
 * NOTE: The recorder won't change states since both \c STATE_1 and \c STATE_2 are STOPPED.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_stopped_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_stopped";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::STOPPED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::STOPPED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2,
            0, EventKind::NO_EVENT, "B");

    // Verify that the SQL file wasn't created
    ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATH));
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from RUNNING to PAUSED.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * The recorder should record all messages while in RUNNING state and none while in PAUSED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_running_paused_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_running_paused";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::RUNNING;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2,
            0, EventKind::NO_EVENT, "B");

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, 0); // the recorder has a filter in "B" and do no store information.
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from RUNNING to SUSPENDED.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * The recorder should record all messages while in RUNNING state and none while in SUSPENDED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_running_suspended_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_running_suspended";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::RUNNING;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::SUSPENDED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2,
            0, EventKind::NO_EVENT, "B");

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, 0); // the recorder has a filter in "B" and do no store information.
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from RUNNING to STOPPED.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * The recorder should record all messages while in RUNNING state and none while in STOPPED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_running_stopped_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_running_stopped";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::RUNNING;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::STOPPED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2,
            0, EventKind::NO_EVENT, "B");

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, 0); // the recorder has a filter in "B" and do no store information.
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from PAUSED to RUNNING.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * The recorder should not record any messages while in PAUSED state and all messages while in RUNNING state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_paused_running_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_paused_running";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::RUNNING;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2,
            0, EventKind::NO_EVENT, "B");

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, 0); // the recorder has a filter in "B" and do no store information.
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from PAUSED to SUSPENDED.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * The recorder should not record any messages while in PAUSED state or while in SUSPENDED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_paused_suspended_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_paused_suspended";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::SUSPENDED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2,
            0, EventKind::NO_EVENT, "B");

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, 0);
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from PAUSED to STOPPED.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * The recorder should not record any messages while in PAUSED state or while in STOPPED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_paused_stopped_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_paused_stopped";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::PAUSED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::STOPPED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2,
            0, EventKind::NO_EVENT, "B");

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, 0);
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from SUSPENDED to RUNNING.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * The recorder should not record any messages while in SUSPENDED state and all messages while in RUNNING state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_suspended_running_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_suspended_running";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::SUSPENDED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::RUNNING;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2,
            0, EventKind::NO_EVENT, "B");

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, 0); // the recorder has a filter in "B" and do no store information.
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from SUSPENDED to PAUSED.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * The recorder should not record any messages while in SUSPENDED state or while in PAUSED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_suspended_paused_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_suspended_paused";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::SUSPENDED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2,
            0, EventKind::NO_EVENT, "B");

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, 0);
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from SUSPENDED to STOPPED.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * The recorder should not record any messages while in SUSPENDED state or while in STOPPED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_suspended_stopped_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_suspended_stopped";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::SUSPENDED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::STOPPED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2,
            0, EventKind::NO_EVENT, "B");

    // Verify that the SQL file wasn't created
    ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATH));
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from STOPPED to RUNNING.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * The recorder should not record any messages while in STOPPED state and all messages while in RUNNING state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_stopped_running_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_stopped_running";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::STOPPED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::RUNNING;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2,
            0, EventKind::NO_EVENT, "B");

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, 0); // the recorder has a filter in "B" and do no store information.
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from STOPPED to PAUSED.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * The recorder should not record any messages while in STOPPED state or while in PAUSED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_stopped_paused_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_stopped_paused";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::STOPPED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::PAUSED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2,
            0, EventKind::NO_EVENT, "B");

    // Count the recorded messages
    exec_sql_statement_(OUTPUT_FILE_PATH, "SELECT COUNT(*) FROM Messages;", {}, [&](sqlite3_stmt* stmt)
            {
                // Verify the recorded messages count
                const auto recorded_messages = sqlite3_column_int(stmt, 0);
                ASSERT_EQ(recorded_messages, 0);
            });
}

/**
 * @brief Verify that the DDS Recorder records properly in an SQL file after transitioning from STOPPED to SUSPENDED.
 *
 * Writer publish with partition = "A".
 * The recorder is created with "B" filter partition,
 * the recorded file will not have any data
 *
 * The recorder should not record any messages while in STOPPED state or while in SUSPENDED state.
 *
 * CASES:
 *  - Verify that the message count matches the recorded message count.
 */
TEST_F(SqlFileCreationPartitionTest, transition_stopped_suspended_partition_no_record)
{
    // adds the partition A in the publisher
    init_dds_data(std::vector<std::string>{"A"}, true);

    const std::string OUTPUT_FILE_NAME = "transition_stopped_suspended";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");

    constexpr auto NUMBER_OF_MESSAGES_1 = 11;
    constexpr auto STATE_1 = DdsRecorderState::STOPPED;
    constexpr auto NUMBER_OF_MESSAGES_2 = 9;
    constexpr auto STATE_2 = DdsRecorderState::SUSPENDED;

    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    const auto OUTPUT_FILE_PATH_MCAP = get_output_file_path_(OUTPUT_FILE_NAME + ".mcap");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH_MCAP));

    // Record messages
    record_messages_(OUTPUT_FILE_NAME, NUMBER_OF_MESSAGES_1, STATE_1, NUMBER_OF_MESSAGES_2, STATE_2,
            0, EventKind::NO_EVENT, "B");

    // Verify that the SQL file wasn't created
    ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATH));
}


int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
