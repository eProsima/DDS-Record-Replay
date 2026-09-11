// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include <fstream>
#include <string>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>
#include <ddsrecorder_participants/recorder/output/OutputSettings.hpp>

using namespace eprosima::ddsrecorder::participants;

namespace test {

constexpr const char* FILENAME = "output";
constexpr const char* EXTENSION = ".mcap";
constexpr std::uint64_t FILE_SIZE = 100;

} /* namespace test */

class FileTrackerTest : public testing::Test
{
public:

    void SetUp() override
    {
        test_directory_ = std::filesystem::temp_directory_path() /
                ("file_tracker_test_" + std::string(testing::UnitTest::GetInstance()->current_test_info()->name()));

        std::filesystem::remove_all(test_directory_);

        ASSERT_TRUE(std::filesystem::create_directories(test_directory_));
    }

    void TearDown() override
    {
        std::filesystem::remove_all(test_directory_);
    }

protected:

    OutputSettings create_configuration_(
            const std::uint64_t max_size,
            const std::uint64_t max_file_size,
            const bool include_existing_files,
            const bool prepend_timestamp = false)
    {
        OutputSettings configuration;

        configuration.filepath = test_directory_.string();
        configuration.filename = test::FILENAME;
        configuration.extension = test::EXTENSION;
        configuration.prepend_timestamp = prepend_timestamp;
        configuration.timestamp_format = "%Y-%m-%d_%H-%M-%S";
        configuration.local_timestamp = false;
        configuration.resource_limits.max_size_ = max_size;
        configuration.resource_limits.max_file_size_ = max_file_size;
        configuration.resource_limits.file_rotation_ = true;
        configuration.resource_limits.include_existing_files_ = include_existing_files;

        return configuration;
    }

    /**
     * @brief Creates a file of \c size bytes that was last modified \c age_in_seconds ago.
     *
     * The modification time is set explicitly so that the order in which the existing files are removed is
     * deterministic.
     */
    void create_file_(
            const std::string& filename,
            const std::uint64_t size,
            const std::uint32_t age_in_seconds)
    {
        const auto file_path = test_directory_ / filename;

        {
            std::ofstream file(file_path, std::ios::binary);
            file << std::string(size, 'a');
        }

        ASSERT_TRUE(std::filesystem::exists(file_path));
        ASSERT_EQ(std::filesystem::file_size(file_path), size);

        std::filesystem::last_write_time(file_path,
                std::filesystem::last_write_time(file_path) - std::chrono::seconds(age_in_seconds));
    }

    bool exists_(
            const std::string& filename) const
    {
        return std::filesystem::exists(test_directory_ / filename);
    }

    std::filesystem::path test_directory_;
};

/**
 * @brief Test that the existing output files are ignored unless the user requests otherwise.
 *
 * CASES:
 * - check that the aggregate size of a new FileTracker doesn't include the existing output files.
 */
TEST_F(FileTrackerTest, existing_files_not_tracked_by_default)
{
    create_file_("output_0.mcap", test::FILE_SIZE, 30);
    create_file_("output_1.mcap", test::FILE_SIZE, 20);

    FileTracker file_tracker(create_configuration_(3 * test::FILE_SIZE, test::FILE_SIZE, false));

    ASSERT_EQ(file_tracker.get_total_size(), 0u);
}

/**
 * @brief Test that the existing output files are tracked when the user requests it.
 *
 * CASES:
 * - check that the aggregate size of a new FileTracker includes the existing output files.
 */
TEST_F(FileTrackerTest, existing_files_tracked)
{
    create_file_("output_0.mcap", test::FILE_SIZE, 30);
    create_file_("output_1.mcap", test::FILE_SIZE, 20);

    FileTracker file_tracker(create_configuration_(3 * test::FILE_SIZE, test::FILE_SIZE, true));

    ASSERT_EQ(file_tracker.get_total_size(), 2 * test::FILE_SIZE);
}

/**
 * @brief Test that only the files that the DDS Recorder would have created are tracked.
 *
 * CASES:
 * - check that a file with a different filename is not tracked.
 * - check that a file with a different extension is not tracked.
 * - check that the temporary file of an interrupted execution is not tracked.
 */
TEST_F(FileTrackerTest, unrelated_files_not_tracked)
{
    create_file_("output_0.mcap", test::FILE_SIZE, 30);
    create_file_("other_output.mcap", test::FILE_SIZE, 30);
    create_file_("output_0.db", test::FILE_SIZE, 30);
    create_file_("output_1.mcap.tmp~", test::FILE_SIZE, 30);

    FileTracker file_tracker(create_configuration_(3 * test::FILE_SIZE, test::FILE_SIZE, true));

    ASSERT_EQ(file_tracker.get_total_size(), test::FILE_SIZE);
}

/**
 * @brief Test that the existing output files are tracked when the output filename has a timestamp prefix.
 *
 * CASES:
 * - check that the aggregate size of a new FileTracker includes the timestamped existing output files.
 */
TEST_F(FileTrackerTest, timestamped_existing_files_tracked)
{
    create_file_("2026-01-01_00-00-00_output_0.mcap", test::FILE_SIZE, 30);
    create_file_("2026-01-01_00-01-00_output_1.mcap", test::FILE_SIZE, 20);

    FileTracker file_tracker(create_configuration_(3 * test::FILE_SIZE, test::FILE_SIZE, true, true));

    ASSERT_EQ(file_tracker.get_total_size(), 2 * test::FILE_SIZE);
}

/**
 * @brief Test that the file rotation removes the oldest existing output file to make room for a new one.
 *
 * This is the regression test for a DDS Recorder restart: the output files of the previous execution used to be
 * ignored, so the aggregate size of the output directory was not bounded by the max-size any more.
 *
 * CASES:
 * - check that the oldest existing output file is removed when there is no space for a new file.
 * - check that the rest of the existing output files are kept.
 * - check that the new file doesn't reuse the id of an existing output file.
 */
TEST_F(FileTrackerTest, oldest_existing_file_removed)
{
    // The files are created in reverse order, so that their age doesn't match their position in the directory
    create_file_("output_1.mcap", test::FILE_SIZE, 20);
    create_file_("output_2.mcap", test::FILE_SIZE, 10);
    create_file_("output_0.mcap", test::FILE_SIZE, 30);

    FileTracker file_tracker(create_configuration_(3 * test::FILE_SIZE, test::FILE_SIZE, true));

    ASSERT_EQ(file_tracker.get_total_size(), 3 * test::FILE_SIZE);

    file_tracker.new_file(test::FILE_SIZE / 2);

    ASSERT_EQ(file_tracker.get_total_size(), 2 * test::FILE_SIZE);
    ASSERT_FALSE(exists_("output_0.mcap"));
    ASSERT_TRUE(exists_("output_1.mcap"));
    ASSERT_TRUE(exists_("output_2.mcap"));

    // The new file's id follows the greatest id of the existing files, so that no output file is overwritten
    ASSERT_EQ(file_tracker.get_current_filename(), (test_directory_ / "output_3.mcap").string() + ".tmp~");
}

/**
 * @brief Test that the file rotation bounds the output directory when the existing output files exceed the max-size.
 *
 * The existing output files may exceed the max-size when they were recorded with a different configuration.
 *
 * CASES:
 * - check that as many existing output files as necessary are removed to fit the new file within the max-size.
 */
TEST_F(FileTrackerTest, existing_files_exceeding_max_size_removed)
{
    create_file_("output_0.mcap", test::FILE_SIZE, 50);
    create_file_("output_1.mcap", test::FILE_SIZE, 40);
    create_file_("output_2.mcap", test::FILE_SIZE, 30);
    create_file_("output_3.mcap", test::FILE_SIZE, 20);
    create_file_("output_4.mcap", test::FILE_SIZE, 10);

    FileTracker file_tracker(create_configuration_(3 * test::FILE_SIZE, test::FILE_SIZE, true));

    ASSERT_EQ(file_tracker.get_total_size(), 5 * test::FILE_SIZE);

    file_tracker.new_file(test::FILE_SIZE);

    // Three of the existing files are removed, so that the new file fits within the max-size
    ASSERT_EQ(file_tracker.get_total_size(), 2 * test::FILE_SIZE);
    ASSERT_FALSE(exists_("output_0.mcap"));
    ASSERT_FALSE(exists_("output_1.mcap"));
    ASSERT_FALSE(exists_("output_2.mcap"));
    ASSERT_TRUE(exists_("output_3.mcap"));
    ASSERT_TRUE(exists_("output_4.mcap"));
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
