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
#include <cstdint>
#include <filesystem>
#include <system_error>
#include <thread>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>


#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <sqlite/sqlite3.h>

#include <cpp_utils/Log.hpp>
#include <cpp_utils/logging/BaseLogConfiguration.hpp>
#include <cpp_utils/logging/StdLogConsumer.hpp>

#include <ddspipe_yaml/Yaml.hpp>
#include <ddspipe_yaml/YamlReader.hpp>

#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>
#include <ddsrecorder_yaml/recorder/CommandlineArgsRecorder.hpp>
#include <ddsrecorder_yaml/recorder/YamlReaderConfiguration.hpp>

#include <tool/DdsRecorder.hpp>

#include "../../resources/types/hello_world/HelloWorld.hpp"
#include "../../resources/types/hello_world/HelloWorldPubSubTypes.hpp"

#include "../constants.hpp"

using namespace eprosima;
using namespace eprosima::fastdds::dds;

class ResourceLimitsTest : public testing::Test
{
public:

    void SetUp() override
    {
        // Route DDS Pipe / DDS Recorder diagnostics to the test output.
        // NOTE: the log configuration is only ever applied by the tool's main(), so the log_filter
        // that reset_configuration_() sets below is parsed and then never used. Without a consumer
        // registered here, everything the SQL writer reports while deciding to rotate or to free
        // space -- FAIL_SQL_REMOVE, FAIL_SQL_SIZE, DISCARDED_SAMPLE -- is discarded, and an
        // under-sized output file gives no clue as to why.
        utils::Log::ClearConsumers();
        utils::Log::SetVerbosity(log_configuration_.verbosity);
        utils::Log::RegisterConsumer(std::make_unique<utils::StdLogConsumer>(&log_configuration_));

        limits_ = &mcap_limits_;
        // Create the participant
        DomainParticipantQos pqos;
        pqos.name(test::PARTICIPANT_ID);

        participant_ = DomainParticipantFactory::get_instance()->create_participant(test::DOMAIN, pqos);

        ASSERT_NE(participant_, nullptr);

        // Register the type
        TypeSupport type(new HelloWorldPubSubType());
        type.register_type(participant_);

        // Create the topic
        topic_ = participant_->create_topic(test::TOPIC_NAME, type.get_type_name(), TOPIC_QOS_DEFAULT);

        ASSERT_NE(topic_, nullptr);

        // Create the publisher
        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

        ASSERT_NE(publisher_, nullptr);
    }

    void TearDown() override
    {
        // Delete the participant
        if (participant_ != nullptr)
        {
            participant_->delete_contained_entities();
            DomainParticipantFactory::get_instance()->delete_participant(participant_);
        }

        // Remove the output files
        for (const auto& path : paths_)
        {
            delete_file_(path);
        }

        // Flush before dropping the consumer, so entries queued during teardown are still printed
        utils::Log::Flush();
        utils::Log::ClearConsumers();
    }

protected:

    void reset_configuration_(
            test::FileTypes file_type,
            std::string output_file_name = "output",
            std::uint32_t max_size = 0,
            std::uint32_t max_file_size = 0,
            bool log_rotation = false)
    {
        const std::string maxFileSize = std::to_string(max_file_size) + "B";
        const std::string maxSize = std::to_string(max_size) + "B";

        std::string yml_str =
                "dds:\n"
                "  domain: " + std::to_string(test::DOMAIN) + "\n"
                "recorder:\n"
                "  output:\n"
                "    filename: " + output_file_name + "\n"
                "    path: \"" + std::filesystem::current_path().string() + "\"\n";
        if (file_type == test::FileTypes::MCAP || file_type == test::FileTypes::BOTH)
        {
            yml_str +=
                    "  mcap:\n"
                    "    enable: true\n"
                    "    resource-limits:\n";
        }

        if (file_type == test::FileTypes::SQL || file_type == test::FileTypes::BOTH)
        {
            yml_str +=
                    "  sql:\n"
                    "    enable: true\n"
                    "    resource-limits:\n";
        }

        if (max_file_size > 0)
        {
            yml_str +=
                    "      max-file-size: \"" + maxFileSize + "\"\n";
        }
        if (max_size > 0)
        {
            yml_str +=
                    "      max-size: \"" + maxSize + "\"\n";
        }
        if (log_rotation)
        {
            yml_str +=
                    "      log-rotation: true\n";
        }
        #if defined(_WIN32) // On windows, the path separator is '\', but the yaml parser expects '/'.
        std::replace(yml_str.begin(), yml_str.end(), '\\', '/');
        #endif // _WIN32
        Yaml yml = YAML::Load(yml_str);

        // Setting CommandLine arguments as if configured from CommandLine
        ddsrecorder::yaml::CommandlineArgsRecorder commandline_args;

        commandline_args.log_filter[eprosima::utils::VerbosityKind::Warning].set_value("DDSRECORDER|DDSPIPE|DEBUG");
        configuration_ = std::make_unique<ddsrecorder::yaml::RecorderConfiguration>(yml, &commandline_args);
        configuration_->mcap_writer_options.compression = mcap::Compression::None;
        configuration_->buffer_size = 1;
    }

    void reset_datawriter_()
    {
        if (writer_ != nullptr)
        {
            publisher_->delete_datawriter(writer_);
        }

        // Configure the DataWriter's QoS to ensure that the DDS Recorder receives all the msgs
        DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;
        wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
        wqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        wqos.history().kind = KEEP_ALL_HISTORY_QOS;

        // Create the writer
        writer_ = publisher_->create_datawriter(topic_, wqos);

        ASSERT_NE(writer_, nullptr);
    }

    void publish_msgs_(
            const std::uint32_t num_msgs)
    {
        // Reset the DataWriter to clear its history
        reset_datawriter_();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        for (std::uint32_t i = 0; i < num_msgs; i++)
        {
            HelloWorld hello;
            hello.index(i);
            writer_->write(&hello);

            // Wait for a millisecond
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    std::vector<std::filesystem::path> get_output_file_paths_(
            const std::uint32_t number_of_files,
            const std::string& output_file_name,
            const test::FileTypes file_type)
    {
        for (std::uint32_t i = 0; i < number_of_files; i++)
        {
            paths_.push_back(get_output_file_path_(output_file_name + "_" + std::to_string(i), file_type));
        }

        return paths_;
    }

    std::filesystem::path get_output_file_path_(
            const std::string& output_file_name,
            const test::FileTypes file_type)
    {
        if (file_type == test::FileTypes::SQL)
        {
            return std::filesystem::current_path() / (output_file_name + ".db");
        }
        return std::filesystem::current_path() / (output_file_name + ".mcap");
    }

    bool delete_file_(
            const std::filesystem::path& file_path)
    {
        if (!std::filesystem::exists(file_path))
        {
            return true;
        }
        return std::filesystem::remove(file_path);
    }

    bool is_file_size_acceptable_(
            const std::filesystem::path& file_path)
    {

        if (!std::filesystem::exists(file_path))
        {
            return false;
        }

        bool mcap_file = file_path.string()[file_path.string().size() - 1] == 'p';

        const auto file_size = std::filesystem::file_size(file_path);
        const auto is_acceptable =
                file_size >= limits_->MIN_ACCEPTABLE_FILE_SIZE &&
                file_size <= limits_->MAX_ACCEPTABLE_FILE_SIZE;

        if (!is_acceptable)
        {
            std::cout << "is_file_size_acceptable_: File " << file_path << " has an unacceptable size of "
                      << file_size << " bytes, when limits: " << limits_->MIN_ACCEPTABLE_FILE_SIZE
                      << " <= size <= " << limits_->MAX_ACCEPTABLE_FILE_SIZE << std::endl;

            // An unexpected size on its own does not say whether samples never arrived or whether
            // log rotation freed more space than it needed to. Report what the output actually
            // holds so the two can be told apart from the CI log alone.
            if (!mcap_file)
            {
                report_sql_contents_(file_path);
            }
        }
        return is_acceptable;
    }

    /**
     * @brief Print the row counts of an SQL output, to diagnose an unexpected file size.
     *
     * A low row count means samples never reached the writer; a high row count with a small file
     * means log rotation reclaimed more space than expected.
     */
    void report_sql_contents_(
            const std::filesystem::path& file_path)
    {
        sqlite3* database = nullptr;

        // NOTE: std::filesystem::path::c_str() yields const wchar_t* on Windows, so the path has to
        // be narrowed explicitly. Keep it in a named local so it outlives the call.
        const std::string file_path_str = file_path.string();

        if (sqlite3_open_v2(file_path_str.c_str(), &database, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK)
        {
            std::cout << "report_sql_contents_: could not open " << file_path << ": "
                      << sqlite3_errmsg(database) << std::endl;
            sqlite3_close(database);
            return;
        }

        for (const auto* query : {"SELECT COUNT(*) FROM Messages;", "PRAGMA page_count;",
                                  "PRAGMA page_size;", "PRAGMA freelist_count;"})
        {
            sqlite3_stmt* stmt = nullptr;

            if (sqlite3_prepare_v2(database, query, -1, &stmt, nullptr) == SQLITE_OK
                    && sqlite3_step(stmt) == SQLITE_ROW)
            {
                std::cout << "report_sql_contents_: " << query << " -> "
                          << sqlite3_column_int64(stmt, 0) << std::endl;
            }
            else
            {
                std::cout << "report_sql_contents_: " << query << " failed: "
                          << sqlite3_errmsg(database) << std::endl;
            }

            sqlite3_finalize(stmt);
        }

        sqlite3_close(database);
    }

    /**
     * @brief Wait until the DDS Recorder has finished writing the received samples to disk.
     */
    void wait_for_recording_to_settle_(
            const std::filesystem::path& output_file_path)
    {
        // Temporary output
        const std::filesystem::path tmp_path(output_file_path.string() + ".tmp~");
        // Build the WAL sidecar path used by SQLite when write-ahead logging is enabled
        const std::filesystem::path wal_path(tmp_path.string() + "-wal");

        // Compute the total number of bytes currently persisted by the recorder.
        const auto recorded_size = [&]() -> std::uintmax_t
                {
                    std::error_code ec;
                    std::uintmax_t ret = 0;

                    if (std::filesystem::exists(tmp_path, ec))
                    {
                        // temp file exists
                        ret += std::filesystem::file_size(tmp_path, ec);
                    }

                    if (std::filesystem::exists(wal_path, ec))
                    {
                        // WAL sidecar exists
                        ret += std::filesystem::file_size(wal_path, ec);
                    }

                    return ret;
                };

        // NOTE: there is deliberately no size threshold to break out early on. recorded_size() sums
        // the temporary output and its WAL sidecar, and that over-counts: the WAL holds page images
        // that overwrite existing pages on checkpoint rather than extending the file. Treating that
        // sum as "the recording reached max-size" stopped the recorder while the database was still
        // short, and the caller then measured the smaller checkpointed file. Waiting for the writes
        // to actually stop is the only signal here that means what it says.
        //
        // Progress is measured from file sizes rather than by querying the database, which would be
        // a concurrent reader of a WAL-mode database and reports races under ThreadSanitizer.

        // Sleep to avoid busy-waiting
        constexpr auto POLL_PERIOD = std::chrono::milliseconds(200);
        // Consider the recorder drained once the observed size stays unchanged for this long. Kept
        // generous: while draining the recorder writes continuously, so a long unchanged period
        // means it is done. A short window (e.g. 1s) is unreliable because a slow write cadence or a
        // WAL checkpoint pause on a loaded machine can be mistaken for completion.
        constexpr auto STABLE_PERIOD = std::chrono::seconds(5);
        // Hard cap, kept well below the ctest per-test timeout. Reached only on a genuine
        // under-recording, in which case the caller's assertion reports the real (too small) size.
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(40);

        auto last_size = recorded_size();
        auto last_change = std::chrono::steady_clock::now();

        // Keep polling until the recording stops growing, or the timeout expires
        while (std::chrono::steady_clock::now() < deadline)
        {
            const auto size = recorded_size();

            if (size != last_size)
            {
                // Update the last observed size so the next iteration compares against the new value
                last_size = size;
                // Reset the stability timer because new output was still being produced
                last_change = std::chrono::steady_clock::now();
            }
            else if (std::chrono::steady_clock::now() - last_change >= STABLE_PERIOD)
            {
                // The recording has not grown for STABLE_PERIOD: the pipeline has drained
                break;
            }

            std::this_thread::sleep_for(POLL_PERIOD);
        }
    }

    void test_max_file_size(
            const test::FileTypes file_type)
    {
        const std::string OUTPUT_FILE_NAME = std::string("max_file_size_test") +
                (file_type == test::FileTypes::MCAP ? "_mcap" : "_sql");
        auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME, file_type);
        if (file_type == test::FileTypes::MCAP)
        {
            reset_configuration_(file_type, OUTPUT_FILE_NAME, limits_->MAX_FILE_SIZE, limits_->MAX_FILE_SIZE);
        }
        else
        {
            reset_configuration_(file_type, OUTPUT_FILE_NAME, limits_->MAX_SIZE, 0);
        }


        // Delete the output file if it exists
        ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

        ddsrecorder::recorder::DdsRecorder recorder(*configuration_,
                ddsrecorder::recorder::DdsRecorderStateCode::RUNNING,
                OUTPUT_FILE_NAME);

        // Send many more messages than can be stored in a file with a size of max-file-size
        const auto WAY_TOO_MANY_MSGS = limits_->FILE_OVERFLOW_THRESHOLD * 2;
        publish_msgs_(WAY_TOO_MANY_MSGS);

        // Make sure the DDS Recorder has received all the messages
        ASSERT_EQ(writer_->wait_for_acknowledgments(test::MAX_WAITING_TIME), RETCODE_OK);

        // Wait for the DDS Recorder to write the received messages before stopping it
        wait_for_recording_to_settle_(OUTPUT_FILE_PATH);

        // All the messages have been sent. Stop the DDS Recorder.
        recorder.stop();

        ASSERT_TRUE(is_file_size_acceptable_(OUTPUT_FILE_PATH));
    }

    void test_max_size(
            const test::FileTypes file_type)
    {
        std::uint32_t NUMBER_OF_FILES = limits_->MAX_FILES + 1;
        const std::string OUTPUT_FILE_NAME = std::string("max_size_test") +
                (file_type == test::FileTypes::MCAP ? "_mcap" : "_sql");
        const auto OUTPUT_FILE_PATHS = get_output_file_paths_(NUMBER_OF_FILES, OUTPUT_FILE_NAME, file_type);

        reset_configuration_(file_type, OUTPUT_FILE_NAME, limits_->MAX_SIZE, limits_->MAX_FILE_SIZE);

        // Delete the output files if they exist
        for (const auto& path : OUTPUT_FILE_PATHS)
        {
            ASSERT_TRUE(delete_file_(path));
        }

        if (file_type == test::FileTypes::SQL)
        {
            const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME, file_type);
            ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));
            reset_configuration_(file_type, OUTPUT_FILE_NAME, limits_->MAX_SIZE, limits_->MAX_SIZE / 3);
            // Verify that an InitializationException was thrown
            eprosima::utils::Formatter error_msg;
            ASSERT_FALSE(configuration_->is_valid(error_msg));
            return;
        }

        ddsrecorder::recorder::DdsRecorder recorder(*configuration_,
                ddsrecorder::recorder::DdsRecorderStateCode::RUNNING,
                OUTPUT_FILE_NAME);


        for (std::uint32_t i = 0; i < limits_->MAX_FILES; i++)
        {
            // Send more messages than can be stored in a file with a size of max-file-size
            publish_msgs_(limits_->FILE_OVERFLOW_THRESHOLD);

            // Make sure the DDS Recorder has received all the messages
            ASSERT_EQ(writer_->wait_for_acknowledgments(test::MAX_WAITING_TIME), RETCODE_OK);

            // Verify that the DDS Recorder has created the expected number of output files and that their size is close
            // but doesn't exceed the max-file-size
            for (std::uint32_t j = 0; j <= i; j++)
            {
                ASSERT_TRUE(is_file_size_acceptable_(OUTPUT_FILE_PATHS[j]));
            }
        }

        // Send more messages than can be stored in a file with a size of max-file-size
        publish_msgs_(limits_->FILE_OVERFLOW_THRESHOLD);

        // Make sure the DDS Recorder has received all the messages
        ASSERT_EQ(writer_->wait_for_acknowledgments(test::MAX_WAITING_TIME), RETCODE_OK);

        // All the messages have been sent. Stop the DDS Recorder.
        recorder.stop();

        // Verify that the size of the rest of the files hasn't changed and no new files have been created
        for (std::uint32_t i = 0; i < NUMBER_OF_FILES; i++)
        {
            if (i < limits_->MAX_FILES)
            {
                ASSERT_TRUE(is_file_size_acceptable_(OUTPUT_FILE_PATHS[i]));
            }
            else
            {
                ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATHS[i]));
            }
        }
    }

    void fill_file(
            const std::vector<std::filesystem::path>& output_file_paths,
            const std::uint32_t file_index)
    {
        // Send more messages than can be stored in a file with a size of max-file-size
        publish_msgs_(limits_->FILE_OVERFLOW_THRESHOLD);

        // Make sure the DDS Recorder has received all the messages
        ASSERT_EQ(writer_->wait_for_acknowledgments(test::MAX_WAITING_TIME), RETCODE_OK);
    }

    void test_file_rotation(
            const test::FileTypes file_type)
    {
        if (file_type == test::FileTypes::SQL)
        {
            ASSERT_TRUE(false) << "SQL does not support file rotation, only log rotation";
            return;
        }

        constexpr std::uint32_t NUMBER_OF_FILES = 6;
        const std::string OUTPUT_FILE_NAME = std::string("rotation_test") +
                (file_type == test::FileTypes::MCAP ? "_mcap" : "_sql");
        const auto OUTPUT_FILE_PATHS = get_output_file_paths_(NUMBER_OF_FILES + 1, OUTPUT_FILE_NAME, file_type);

        reset_configuration_(file_type, OUTPUT_FILE_NAME, limits_->MAX_SIZE, limits_->MAX_FILE_SIZE, true);

        // Delete the output files if they exist
        for (const auto& path : OUTPUT_FILE_PATHS)
        {
            ASSERT_TRUE(delete_file_(path));
        }

        ddsrecorder::recorder::DdsRecorder recorder(*configuration_,
                ddsrecorder::recorder::DdsRecorderStateCode::RUNNING,
                OUTPUT_FILE_NAME);

        // Verify that the DDS Recorder creates a new file after each batch of messages, before reaching the max-size
        for (std::int32_t i = 0; i < NUMBER_OF_FILES; i++)
        {
            // Send more messages than can be stored in a file with a size of max-file-size
            fill_file(OUTPUT_FILE_PATHS, i);

            // Verify that the DDS Recorder has created the expected number of output files with the expected size
            for (std::int32_t j = 0; j < NUMBER_OF_FILES; j++)
            {
                if ((i - j) < limits_->MAX_FILES - 1 && (i - j) >= 0)
                {
                    ASSERT_TRUE(is_file_size_acceptable_(OUTPUT_FILE_PATHS[j]));
                }
                else
                {
                    ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATHS[j]));
                }
            }
        }
    }

    void test_log_rotation(
            test::FileTypes file_type)
    {
        if (file_type == test::FileTypes::MCAP)
        {
            ASSERT_TRUE(false)
                <<
                "This test is not useful. Maybe throw an exception if in MCAP the max_size and log_rotation is set but no max_file_size? It would overwriting the same file over and over";
            return;
        }

        constexpr int NUMBER_OF_FILES = 3;

        const std::string OUTPUT_FILE_NAME = std::string("log_rotation_test") +
                (file_type == test::FileTypes::MCAP ? "_mcap" : "_sql");
        const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME, file_type);

        reset_configuration_(file_type, OUTPUT_FILE_NAME, limits_->MAX_SIZE, 0, true);

        // Delete the output file if it exists
        ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

        // These files are just to check that the DDS Recorder doesn't create them
        const auto OUTPUT_FILE_PATHS = get_output_file_paths_(NUMBER_OF_FILES + 1, OUTPUT_FILE_NAME, file_type);

        reset_configuration_(file_type, OUTPUT_FILE_NAME, limits_->MAX_FILE_SIZE, limits_->MAX_SIZE, true);

        // Delete the output files if they exist
        for (const auto& path : OUTPUT_FILE_PATHS)
        {
            ASSERT_TRUE(delete_file_(path));
        }

        ddsrecorder::recorder::DdsRecorder recorder(*configuration_,
                ddsrecorder::recorder::DdsRecorderStateCode::RUNNING,
                OUTPUT_FILE_NAME);


        // Verify that the DDS Recorder doesnt create a new file after each batch of messages, even when reaching the max-size
        for (std::uint32_t i = 0; i < NUMBER_OF_FILES; i++)
        {
            // Send more messages than can be stored in a file with a size of max-file-size
            publish_msgs_(limits_->FILE_OVERFLOW_THRESHOLD);

            // Make sure the DDS Recorder has received all the messages
            ASSERT_EQ(writer_->wait_for_acknowledgments(test::MAX_WAITING_TIME), RETCODE_OK);
        }

        // Wait for the DDS Recorder to write the received messages before stopping it
        wait_for_recording_to_settle_(OUTPUT_FILE_PATH);

        // All the messages have been sent. Stop the DDS Recorder.
        recorder.stop();

        // Verify that the DDS Recorder has created the expected number of output files
        ASSERT_TRUE(is_file_size_acceptable_(OUTPUT_FILE_PATH));

        // Verify that the DDS Recorder hasn't created any extra files
        for (std::uint32_t j = 0; j < NUMBER_OF_FILES; j++)
        {
            ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATHS[j]));
        }
    }

    DomainParticipant* participant_ = nullptr;
    Publisher* publisher_ = nullptr;
    Topic* topic_ = nullptr;
    DataWriter* writer_ = nullptr;

    std::unique_ptr<ddsrecorder::yaml::RecorderConfiguration> configuration_;
    std::vector<std::filesystem::path> paths_;

    test::limits mcap_limits_{35 * 1024, 7 * 1024, 0.2, 71};
    test::limits sql_limits_{300 * 1024,  300 * 1024, 0.2, 273};

    test::limits* limits_;

    //! Warning verbosity with an empty (match-all) filter, see BaseLogConfiguration's constructor
    eprosima::utils::BaseLogConfiguration log_configuration_;


};

/**
 * Test that the DDS Recorder doesn't exceed the max-file-size.
 *
 * CASES:
 * - check that the DDS Recorder records data until it reaches the max-file-size.
 */
TEST_F(ResourceLimitsTest, mcap_max_file_size)
{
    limits_ = &mcap_limits_;
    test_max_file_size(test::FileTypes::MCAP);
}
TEST_F(ResourceLimitsTest, sql_max_file_size)
{
    limits_ = &sql_limits_;
    test_max_file_size(test::FileTypes::SQL);
}

/**
 * @brief Test that the DDS Recorder's output doesn't exceed the max-size.
 *
 * In this test, the DDS Recorder's output is configured to have a max-size of 30KiB and a max-file-size of 7.5KiB.
 * The DDS Recorder should create 4 output files, each of them with a size between 7KiB and 8KiB.
 *
 * A writer publishes 110 messages 4 times to verify that the DDS Recorder creates a new file after each batch of
 * messages. The writer then publishes 110 messages again to verify that the DDS Recorder doesn't create a new file,
 * since it would exceed the max-size. Then, the test verifies that the size of each of the DDS Recorder's output files
 * in bounds.
 *
 * CASES:
 * - check that the size of each of the DDS Recorder's output files is in bounds.
 * - check that the aggregate size of the DDS Recorder's output files is in bounds.
 */
TEST_F(ResourceLimitsTest, mcap_max_size)
{
    limits_ = &mcap_limits_;
    test_max_size(test::FileTypes::MCAP);
}
TEST_F(ResourceLimitsTest, sql_max_size)
{
    limits_ = &sql_limits_;
    test_max_size(test::FileTypes::SQL);
}

/**
 * @brief Test that the DDS Recorder's applies log-rotation to its output after reaching the max-size.
 *
 * In this test, the DDS Recorder's output is configured to have a max-size of 30KiB and a max-file-size of 7.5KiB.
 * The DDS Recorder should create 3 output files, each of them with a size between 7KiB and 8KiB. Then, after
 * receiving more data, the DDS Recorder should remove the oldest files and create new ones.
 *
 * A writer publishes 110 messages 3 times to verify that the DDS Recorder creates a new file after each batch of
 * messages. The writer then publishes 110 messages 5 more times to verify that, after each batch of messages, the DDS
 * Recorder deletes the oldest file and creates a new one. The test also verifies that the size of each of the DDS
 * Recorder's output files is close to the max-file-size but doesn't surpass it.
 *
 * CASES:
 * - check that the size of each of the DDS Recorder's output files is close but doesn't exceed the max-file-size.
 * - check that the aggregate size of the DDS Recorder's output files doesn't exceed the max-size.
 * - check that the DDS Recorder applies log rotation after reaching the max-size.
 */
TEST_F(ResourceLimitsTest, mcap_file_rotation)
{
    limits_ = &mcap_limits_;
    test_file_rotation(test::FileTypes::MCAP);
}
TEST_F(ResourceLimitsTest, sql_log_rotation)
{
    limits_ = &sql_limits_;
    test_log_rotation(test::FileTypes::SQL);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
