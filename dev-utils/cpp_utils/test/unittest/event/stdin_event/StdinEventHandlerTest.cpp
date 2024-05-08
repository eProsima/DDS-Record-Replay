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

#include <iostream>
#include <queue>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/event/StdinEventHandler.hpp>
#include <cpp_utils/utils.hpp>

using namespace eprosima::utils::event;

/**
 * @brief Create an object of StdinEventHandler and let it be destroyed.
 *
 * This test may crush if fails.
 */
TEST(StdinEventHandlerTest, trivial_create_handler)
{
    StdinEventHandler handler(
        [](std::string)
        {
            /* empty callback */ },
        0);

    // Let handler to be destroyed by itself
}

/**
 * @brief Read from custom source 2 lines
 */
TEST(StdinEventHandlerTest, read_lines_start)
{
    // Wait till call ++ 2 times
    CounterWaitHandler counter(1, 0, true);

    std::string str1("some_easy_line");
    std::string str2("Another extra large line to read from our beloved new Event Handler.");

    std::queue<std::string> expected_result;
    expected_result.push(str1);
    expected_result.push(str2);

    std::stringstream source;
    source << str1 << "\n";
    source << str2 << "\n";

    std::mutex mutex_;

    StdinEventHandler handler(
        [&](std::string read_from_source)
        {
            std::lock_guard<std::mutex> _(mutex_);
            auto& expected = expected_result.front();
            ASSERT_EQ(read_from_source, expected);
            expected_result.pop();
            ++counter;
        },
        true,
        2,
        source);

    // Wait for all messages received
    counter.wait_and_decrement();

    ASSERT_TRUE(expected_result.empty());
}

/**
 * @brief Read from custom source 5 strings separated by spaces
 */
TEST(StdinEventHandlerTest, read_spaces_start)
{
    // Wait till call ++ 5 times
    CounterWaitHandler counter(4, 0, true);

    std::string str1("This will be read separately.");

    std::queue<std::string> expected_result;
    for (auto& res : eprosima::utils::split_string(str1, " "))
    {
        expected_result.push(res);
    }

    std::stringstream source;
    source << str1 << " ";

    std::mutex mutex_;

    StdinEventHandler handler(
        [&](std::string read_from_source)
        {
            std::lock_guard<std::mutex> _(mutex_);
            auto& expected = expected_result.front();
            ASSERT_EQ(read_from_source, expected);
            expected_result.pop();
            ++counter;
        },
        false,
        expected_result.size(),
        source);

    // Wait for all messages received
    counter.wait_and_decrement();

    ASSERT_TRUE(expected_result.empty());
}

/**
 * @brief Read from custom source 2 lines by calling read_one_more_line for each
 */
TEST(StdinEventHandlerTest, read_lines_running)
{
    CounterWaitHandler counter(0, 0, true);

    std::string str1("some_easy_line");
    std::string str2("Another extra large line to read from our beloved new Event Handler.");

    std::queue<std::string> expected_result;
    expected_result.push(str1);
    expected_result.push(str2);

    std::stringstream source;
    source << str1 << "\n";
    source << str2 << "\n";

    std::mutex mutex_;

    StdinEventHandler handler(
        [&](std::string read_from_source)
        {
            std::lock_guard<std::mutex> _(mutex_);
            auto& expected = expected_result.front();
            ASSERT_EQ(read_from_source, expected);
            expected_result.pop();
            ++counter;
        },
        true,
        0,
        source);

    // Check nothing is read
    ASSERT_EQ(expected_result.size(), 2u);

    // Open to read one and wait
    handler.read_one_more_line();
    counter.wait_and_decrement();

    // Check there is still one to read
    ASSERT_EQ(expected_result.size(), 1u);

    // Open to read one and wait
    handler.read_one_more_line();
    counter.wait_and_decrement();

    // No more to read
    ASSERT_TRUE(expected_result.empty());
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
