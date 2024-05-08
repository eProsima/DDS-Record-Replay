// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <algorithm>
#include <array>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/exception/PreconditionNotMet.hpp>
#include <cpp_utils/file/file_utils.hpp>
#include <cpp_utils/utils.hpp>

using namespace eprosima::utils;

/*
 * This tests are tested with file resources/file.test that is generated as:
 * echo -ne "First Line\n2line\n\nafter empty line\nline with strange char\r\n6th line 6\n"
 */

namespace test {

constexpr const char* FILE_NAME_TEST = "resources/file.test";

constexpr const std::array<const char*, 6> FILE_ARRAY = {
    "First Line",
    "2line",
    "",
    "after empty line",
    "line with strange char\r",
    "6th line 6"
};

constexpr const char* FILE_IN_LINE = "First Line\n2line\n\nafter empty line\nline with strange char\r\n6th line 6\n";

} // namespace test


/**
 * Test function \c file_to_strings with default args.
 */
TEST(fileTest, read_file_by_lines)
{
    auto result = file_to_strings(test::FILE_NAME_TEST);

    for (std::size_t i = 0; i < result.size(); i++)
    {
        std::string expected_line(result[i].c_str());
        // Strip string by default
        strip_str(expected_line);

        ASSERT_EQ(result[i], expected_line);
    }
}

/**
 * Test function \c file_to_strings leaving the strange chars.
 */
TEST(fileTest, read_file_by_lines_no_strip_chars)
{
    auto result = file_to_strings(test::FILE_NAME_TEST, false);

    for (std::size_t i = 0; i < result.size(); i++)
    {
        std::string expected_line(result[i].c_str());
        // Do not strip string

        ASSERT_EQ(result[i], expected_line);
    }
}

/**
 * Test function \c file_to_strings stripping empty lines and strange chars.
 */
TEST(fileTest, read_file_by_lines_strip_empty_lines)
{
    auto result = file_to_strings(test::FILE_NAME_TEST, true, true);

    std::size_t j = 0;  // Index for result element
    for (std::size_t i = 0; i < result.size(); i++)
    {
        std::string expected_line(result[i].c_str());
        // Strip string by default
        strip_str(expected_line);

        if (expected_line == "")
        {
            // This case should not be in result array, so keep going
            continue;
        }
        else
        {
            ASSERT_EQ(result[j++], expected_line);
        }
    }
}

/**
 * Test function \c file_to_string with default arguments.
 */
TEST(fileTest, read_file_one_line)
{
    auto result = file_to_string(test::FILE_NAME_TEST);

    // The line expected has \r removed, but not \n
    std::string expected_line(test::FILE_IN_LINE);
    strip_str(expected_line, "", {"\r"});

    ASSERT_EQ(result, expected_line);
}

/**
 * Test function \c file_to_string without stripping strange chars.
 */
TEST(fileTest, read_file_one_line_no_strip_chars)
{
    auto result = file_to_string(test::FILE_NAME_TEST, false);

    std::string expected_line(test::FILE_IN_LINE);

    ASSERT_EQ(result, expected_line);
}

/**
 * Test exception thrown calling \c file_to_string and \c file_to_strings with an incorrect file name
 */
TEST(fileTest, read_incorrect_file)
{
    const char* incorrect_file_name = "resource/file.test";

    ASSERT_THROW(file_to_string(incorrect_file_name), PreconditionNotMet);

    ASSERT_THROW(file_to_strings(incorrect_file_name), PreconditionNotMet);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
