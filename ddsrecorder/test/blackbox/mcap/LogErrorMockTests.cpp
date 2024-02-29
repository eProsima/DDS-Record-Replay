#include <filesystem>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cpp_utils/testing/LogChecker.hpp>

#include <ddsrecorder_participants/recorder/mcap/McapHandler.hpp>

#include <mcap/errors.hpp>
#include <mcap/mcap.hpp>

namespace mcap {
namespace test {

// Mock class for FileWriter to allow access to private members
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

// Mock class for McapHandler to allow access to private members
class MCAP_PUBLIC MockMcapHandler : public McapHandler {
public:

    MockMcapHandler(const McapHandlerConfiguration& config,
                const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
                const McapHandlerStateCode& init_state)
    : McapHandler(config, payload_pool, init_state)
    {
    }

    using McapHandler::open_file_nts_;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */



/**
 * Test case to verify logError callback when the disk is full
 *
 * CASES:
 *  This test simulates a disk size available of 0 bytes and a desired writing message
 *  of 100 bytes. As it does not have enough available space a Log Error will be displayed.
 *  The Log Error will be captured by the Log Checker.
 */
TEST(LogErrorMockTests, disk_full) {

    // Create an instance of the Log Checker, in charge of capturing 1 LogError
    eprosima::utils::testing::LogChecker log_checker(
        eprosima::utils::Log::Kind::Error,
        1,
        1);

    // Check no logs have been captured yet
    ASSERT_FALSE(log_checker.check_valid());

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
    // Expect the handleWrite method to be called once with a size greater than available space (0)
    mock_writer.handleWrite(nullptr, 100);

    // Check if 1 LogError was captured
    ASSERT_TRUE(log_checker.check_valid());
}

/**
 * Test case to verify logError callback when the opening mcap file fails
 *
 * CASES:
 *  This test attemps to open a mcap file in a folder that does not exist, leading to
 *  its correspondent Log Error and exception.
 */
TEST(LogErrorMockTests, fail_to_open_file) {

    // Create an instance of the Log Checker, in charge of capturing 1 LogError
    eprosima::utils::testing::LogChecker log_checker(
        eprosima::utils::Log::Kind::Error,
        1,
        1);

    // Check no logs have been captured yet
    ASSERT_FALSE(log_checker.check_valid());

    try {
        eprosima::ddsrecorder::participants::McapOutputSettings mcap_output_settings;
        mcap_output_settings.output_filepath = "./fake_folder"; // This folder does not exist -> error opening file
        mcap_output_settings.output_filename = "output_dummy.mcap";
        mcap_output_settings.prepend_timestamp = false;
        mcap_output_settings.output_timestamp_format = "%Y-%m-%d_%H-%M-%S";
        mcap_output_settings.output_local_timestamp = true;

        mcap::McapWriterOptions mcap_writer_options{"ros2"};

        eprosima::ddsrecorder::participants::McapHandlerConfiguration config(
            mcap_output_settings,
            100,
            1024,
            60,
            3600,
            true,
            false,
            mcap_writer_options,
            true,
            false);

        std::shared_ptr<eprosima::ddspipe::core::PayloadPool> payload_pool;
        eprosima::ddsrecorder::participants::McapHandlerStateCode init_state = eprosima::ddsrecorder::participants::McapHandlerStateCode::RUNNING;

        // Create an instance of the mock
        eprosima::ddsrecorder::participants::MockMcapHandler mock_mcap_handler(config, payload_pool, init_state);

        mock_mcap_handler.open_file_nts_();

    } catch (const std::exception& ex) {
        // Catch any unexpected exceptions and print their type and description
        std::cerr << "Expected exception caught: " << typeid(ex).name() << ", " << ex.what() << std::endl;
    }

    // Check if 1 LogError was captured
    ASSERT_TRUE(log_checker.check_valid());

}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
