#include <filesystem>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/testing/LogChecker.hpp>

#include <ddsrecorder_participants/recorder/mcap/McapHandler.hpp>

#include <mcap/errors.hpp>
#include <mcap/mcap.hpp>

namespace mcap {
namespace test {

// Mock class for FileWriter
class MCAP_PUBLIC MockFileWriter : public FileWriter {
public:

    using FileWriter::open;
    using FileWriter::write_;
    using FileWriter::get_space_available_;

    // Mock of the open, write_ and get_space_available_ methods
    MOCK_METHOD(Status, open, (std::string_view filename), (override));
    MOCK_CONST_METHOD3(write_, size_t(const void* data, size_t size, std::FILE* stream));
    MOCK_METHOD(std::uintmax_t, get_space_available_, (const std::string& path), (override));

    void set_file()
    {
        file_ = stdout;
    };
};

} /* namespace test */
} /* namespace mcap */

namespace eprosima {
namespace ddsrecorder {
namespace participants {
// Mock class for McapHandler
class MockMcapHandler : public McapHandler {
public:

    MockMcapHandler(const McapHandlerConfiguration& config,
                const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
                const McapHandlerStateCode& init_state)
    : McapHandler(config, payload_pool, init_state)
    {
    }

    using McapHandler::write_message_nts_;
    using McapHandler::open_file_nts_;

    MOCK_METHOD(void, write_message_nts_, (const Message& msg));
    MOCK_METHOD(void, open_file_nts_, ());

    using McapHandler::add_data_nts_;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */

const int BUFFER_SIZE = 100;
const int MAX_FILE_SIZE = 1024;
const int MAX_FILE_AGE = 60;
const int MAX_FILE_COUNT = 3600;
const bool AUTO_START = true;
const bool AUTO_STOP = false;
const bool COMPRESS = true;
const bool ENCRYPT = false;

/**
 * Test case to verify an exception and a logError are thrown when the disk is full
 *
 * CASES:
 *  · Expected exception thrown: This test simulates a disk size available of 0 bytes and a desired
 *    writing message of 100 bytes. As it does not have enough available space an exception will be displayed.
 *  · Expected logError thrown: This test simulates write_message_nts_ throws an exception so it's captured
 *    in add_data_nts_, chacking if its correspondent logError is displayed.
 */
TEST(McapLogErrorTests, disk_full) {

    //// Check of expected exception thrown
    // Create an instance of the mock
    mcap::test::MockFileWriter mock_writer;

    // When calling open method file_ will be set to stdout
    EXPECT_CALL(mock_writer, open(::testing::_))
        .WillOnce([&mock_writer](std::string_view filename) {
            mock_writer.set_file();
            return mcap::StatusCode::Success;
        });

    // Function to set behaviour of write_ method: It will return 10 every time
    ON_CALL(mock_writer, write_(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(testing::Return(10));

    // Function to set behaviour of get_space_available_ method: It will return 0 every time
    ON_CALL(mock_writer, get_space_available_(::testing::_))
        .WillByDefault(testing::Return(0));

    // Call open method to set file_ = stdout (to avoid crash in assert(file_))
    mcap::Status status = mock_writer.open("dummy_file.txt");

    // Expect thrown an exception when calling handleWrite method with a message size (100) different
    // than the written size (10) and greater than the available space in disk (0)
    ASSERT_THROW(mock_writer.handleWrite(nullptr, 100), std::overflow_error);

    //// Check of expected logError displayed
    // Create an instance of the Log Checker, in charge of capturing 1 LogError
    eprosima::utils::testing::LogChecker log_checker(
        eprosima::utils::Log::Kind::Error,
        1,
        1);

    // Check no logs have been captured yet
    ASSERT_FALSE(log_checker.check_valid());

    eprosima::ddsrecorder::participants::McapOutputSettings mcap_output_settings;
    mcap_output_settings.output_filepath = "."; // This folder does not exist -> error opening file
    mcap_output_settings.output_filename = "output_dummy.mcap";
    mcap_output_settings.prepend_timestamp = false;
    mcap_output_settings.output_timestamp_format = "%Y-%m-%d_%H-%M-%S";
    mcap_output_settings.output_local_timestamp = true;

    mcap::McapWriterOptions mcap_writer_options{"ros2"};

    eprosima::ddsrecorder::participants::McapHandlerConfiguration config(
        mcap_output_settings,
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
    eprosima::ddsrecorder::participants::McapHandlerStateCode init_state = eprosima::ddsrecorder::participants::McapHandlerStateCode::RUNNING;

    // Create an instance of the mock McapHandler
    eprosima::ddsrecorder::participants::MockMcapHandler mockHandler(config, payload_pool, init_state);

    // Set expectations for write_message_nts_ to throw an exception
    ON_CALL(mockHandler, write_message_nts_(::testing::_))
        .WillByDefault(testing::Throw(eprosima::utils::InconsistencyException("Test exception")));

    eprosima::ddsrecorder::participants::Message msg;

    // Call the function where the logError is expected to be thrown
    mockHandler.add_data_nts_(msg, true);

    // Assert that logError was captured
    ASSERT_TRUE(log_checker.check_valid());
}

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

    eprosima::ddsrecorder::participants::McapOutputSettings mcap_output_settings;
    mcap_output_settings.output_filepath = "./fake_folder"; // This folder does not exist -> error opening file
    mcap_output_settings.output_filename = "output_dummy.mcap";
    mcap_output_settings.prepend_timestamp = false;
    mcap_output_settings.output_timestamp_format = "%Y-%m-%d_%H-%M-%S";
    mcap_output_settings.output_local_timestamp = true;

    mcap::McapWriterOptions mcap_writer_options{"ros2"};

    eprosima::ddsrecorder::participants::McapHandlerConfiguration config(
        mcap_output_settings,
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
    eprosima::ddsrecorder::participants::McapHandlerStateCode init_state = eprosima::ddsrecorder::participants::McapHandlerStateCode::RUNNING;

    // Check if an InitializationException is thrown
    eprosima::ddsrecorder::participants::McapHandler mcap_handler(config, payload_pool, init_state);

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
