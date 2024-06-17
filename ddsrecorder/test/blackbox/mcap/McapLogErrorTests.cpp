#include <filesystem>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/testing/LogChecker.hpp>

#include <ddsrecorder_participants/recorder/mcap/McapHandler.hpp>
#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>

#include <mcap/errors.hpp>
#include <mcap/mcap.hpp>

const int BUFFER_SIZE = 100;
const int MAX_FILE_SIZE = 1024;
const int MAX_FILE_AGE = 60;
const int MAX_FILE_COUNT = 3600;
const bool AUTO_START = true;
const bool AUTO_STOP = false;
const bool COMPRESS = true;
const bool ENCRYPT = false;

/**
 * Test case to verify a logError is displayed when the opening mcap file fails
 *
 * CASES:
 *  This test attemps to open a mcap file in a folder that does not exist, leading to
 *  its correspondent Log Error. An additional logError failing to rename the MCAP file
 *  will appear when the McapHandler destructor is called (this happens after
 *  log_checker.check_valid() assertion)
 */
TEST(McapLogErrorTests, fail_to_open_file) {

    // Create an instance of the Log Checker, in charge of capturing 1 LogError
    eprosima::utils::testing::LogChecker log_checker(
        eprosima::utils::Log::Kind::Error,
        1,
        1);

    // Check no logs have been captured yet
    ASSERT_FALSE(log_checker.check_valid());

    eprosima::ddsrecorder::participants::OutputSettings output_settings;
    output_settings.filepath = "./fake_folder"; // This folder does not exist -> error opening file
    output_settings.filename = "output_dummy";
    output_settings.prepend_timestamp = false;
    output_settings.timestamp_format = "%Y-%m-%d_%H-%M-%S";
    output_settings.local_timestamp = true;
    output_settings.max_file_size = 100 * 1000; // 100KB
    output_settings.max_size = output_settings.max_file_size; // 100KB

    mcap::McapWriterOptions mcap_writer_options{"ros2"};

    eprosima::ddsrecorder::participants::McapHandlerConfiguration config(
        output_settings,
        BUFFER_SIZE,
        MAX_FILE_SIZE,
        MAX_FILE_AGE,
        MAX_FILE_COUNT,
        AUTO_START,
        AUTO_STOP,
        mcap_writer_options,
        COMPRESS,
        ENCRYPT
        );

    std::shared_ptr<eprosima::ddspipe::core::PayloadPool> payload_pool;
    eprosima::ddsrecorder::participants::McapHandlerStateCode init_state =
            eprosima::ddsrecorder::participants::McapHandlerStateCode::RUNNING;

    // Create the McapWriter
    auto file_tracker = std::make_shared<eprosima::ddsrecorder::participants::FileTracker>(config.output_settings);

    // Check if an InitializationException is thrown
    ASSERT_THROW(
        eprosima::ddsrecorder::participants::McapHandler mcap_handler(config, payload_pool, file_tracker, init_state),
        eprosima::utils::InitializationException);

    // Assert that logErrors were captured
    ASSERT_TRUE(log_checker.check_valid());
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
