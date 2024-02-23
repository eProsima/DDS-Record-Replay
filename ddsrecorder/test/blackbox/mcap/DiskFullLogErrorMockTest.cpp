#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <mcap/mcap.hpp>
#include <mcap/errors.hpp>

#include <cpp_utils/testing/LogChecker.hpp>

namespace mcap {
namespace test {

// Mock class for FileWriter to allow access to private members
class MCAP_PUBLIC MockFileWriter : public FileWriter {
public:

    using FileWriter::open;
    using FileWriter::write_;
    using FileWriter::get_space_available_;

    // Mock of the write_ and get_space_available_ methods
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


/**
 * Test case to verify logError callback when the disk is full
 *
 * CASES:
 *  This test simulates a disk size available of 0 bytes and a desired writing message
 *  of 100 bytes. As it does not have enough available space a Log Error will be displayed.
 *  The Log Error will be captured by the Log Checker.
 */
TEST(DiskFullLogErrorMockTest, log_error_when_disk_is_full) {

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

    // Function to set behaviour of write_ method: It will return 0 every time
    ON_CALL(mock_writer, get_space_available_(::testing::_))
        .WillByDefault(testing::Return(0));

    // Call open method to set file_ = stdout (to avoid crash in assert(file_))
    mcap::Status status = mock_writer.open("dummy_file.txt");
    // Expect the handleWrite method to be called once with a size greater than available space (0)
    mock_writer.handleWrite(nullptr, 100);

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
