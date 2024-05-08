// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include <memory>
#include <thread>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/testing/LogChecker.hpp>


namespace test {

constexpr const unsigned int DEFAULT_TEST_VALUE = 3u;

} /* namespace test */

/**
 * @brief Check that creation and destruction of object is correct.
 */
TEST(LogCheckerTest, trivial_create)
{
    eprosima::utils::testing::LogChecker log_checker;
}

/**
 * @brief Check that using default macro and no logs works correctly.
 */
TEST(LogCheckerTest, non_logs_default)
{
    DEFAULT_LOG_TESTER;
}

/**
 * @brief Check that using macro and no logs works correctly.
 */
TEST(LogCheckerTest, non_logs)
{
    INSTANTIATE_LOG_TESTER(eprosima::utils::Log::Kind::Info, 0, 0);
}

/**
 * @brief Check Log Checker with log Error with minimum expected
 *
 * CASES:
 * - minimum 0 no logs
 * - minimum 0 with 1 log
 * - minimum N no logs
 * - minimum N with n<N logs
 * - minimum N with N+1 logs
 */
TEST(LogCheckerTest, minimum_logs)
{
    // minimum 0
    {
        eprosima::utils::testing::LogChecker log_checker(eprosima::utils::Log::Kind::Error, 0);

        // no logs
        ASSERT_TRUE(log_checker.check_valid());

        // add log
        logError(LOGCHECKER_TEST, "Test propose log.");

        // with 1 log
        ASSERT_FALSE(log_checker.check_valid());
    }

    // minimum N
    {
        eprosima::utils::testing::LogChecker log_checker(
            eprosima::utils::Log::Kind::Error,
            test::DEFAULT_TEST_VALUE,
            test::DEFAULT_TEST_VALUE* 2);

        // no logs
        ASSERT_FALSE(log_checker.check_valid());

        // add N logs
        for (unsigned int i = 0; i < test::DEFAULT_TEST_VALUE; ++i)
        {
            logError(LOGCHECKER_TEST, "Test propose log.");

        }

        // with N logs
        ASSERT_TRUE(log_checker.check_valid());

        // add log
        logError(LOGCHECKER_TEST, "Test propose log.");

        // with N+1 log
        ASSERT_TRUE(log_checker.check_valid());
    }
}

/**
 * @brief Check Log Checker with log Error with maximum expected
 *
 * CASES:
 * - maximum 0 no logs
 * - maximum 0 with 1 log
 * - maximum N no logs
 * - maximum N with n<N logs
 * - maximum N with N+1 logs
 */
TEST(LogCheckerTest, maximum_logs)
{
    // maximum 0
    {
        eprosima::utils::testing::LogChecker log_checker(
            eprosima::utils::Log::Kind::Error,
            0,
            0);

        // no logs
        ASSERT_TRUE(log_checker.check_valid());

        // add log
        logError(LOGCHECKER_TEST, "Test propose log.");

        // with 1 log
        ASSERT_FALSE(log_checker.check_valid());
    }

    // maximum N
    {
        eprosima::utils::testing::LogChecker log_checker(
            eprosima::utils::Log::Kind::Error,
            0,
            test::DEFAULT_TEST_VALUE);

        // no logs
        ASSERT_TRUE(log_checker.check_valid());

        // add N logs
        for (unsigned int i = 0; i < test::DEFAULT_TEST_VALUE; ++i)
        {
            logError(LOGCHECKER_TEST, "Test propose log.");

        }

        // with N logs
        ASSERT_TRUE(log_checker.check_valid());

        // add log
        logError(LOGCHECKER_TEST, "Test propose log.");

        // with N+1 log
        ASSERT_FALSE(log_checker.check_valid());
    }
}

/**
 * @brief Check Log Checker with log Error with exacted expected
 *
 * CASES:
 * - expected N n<N logs
 * - expected N with N logs
 * - expected N with N+1 logs
 */
TEST(LogCheckerTest, exact_logs)
{
    eprosima::utils::testing::LogChecker log_checker(
        eprosima::utils::Log::Kind::Error,
        test::DEFAULT_TEST_VALUE,
        test::DEFAULT_TEST_VALUE);

    // add N logs
    for (unsigned int i = 0; i < test::DEFAULT_TEST_VALUE; ++i)
    {
        ASSERT_FALSE(log_checker.check_valid());
        logError(LOGCHECKER_TEST, "Test propose log.");
    }

    // with N logs
    ASSERT_TRUE(log_checker.check_valid());

    // add log
    logError(LOGCHECKER_TEST, "Test propose log.");

    // with N+1 log
    ASSERT_FALSE(log_checker.check_valid());
}

/**
 * @brief Test that only logs of kind Warning are used. 0 logs are allowed.
 *
 * CASES:
 * - with log info
 * - with log warning
 * - with log error
 */
TEST(LogCheckerTest, severe_logs_info)
{
    // Log info
    {
        eprosima::utils::testing::LogChecker log_checker(
            eprosima::utils::Log::Kind::Info);

        // No logs
        ASSERT_TRUE(log_checker.check_valid());

        // After log info
        logInfo(LOGCHECKER_TEST, "Test propose log.");
        ASSERT_FALSE(log_checker.check_valid());
    }

    // Log warning
    {
        eprosima::utils::testing::LogChecker log_checker(
            eprosima::utils::Log::Kind::Info);

        // No logs
        ASSERT_TRUE(log_checker.check_valid());

        // After log warning
        logWarning(LOGCHECKER_TEST, "Test propose log.");
        ASSERT_FALSE(log_checker.check_valid());
    }

    // Log error
    {
        eprosima::utils::testing::LogChecker log_checker(
            eprosima::utils::Log::Kind::Info);

        // No logs
        ASSERT_TRUE(log_checker.check_valid());

        // After log error
        logError(LOGCHECKER_TEST, "Test propose log.");
        ASSERT_FALSE(log_checker.check_valid());
    }
}

/**
 * @brief Test that only logs of kind Warning are used. 0 logs are allowed.
 *
 * CASES:
 * - with log warning
 * - with log error
 */
TEST(LogCheckerTest, severe_logs_warning)
{
    // Log warning
    {
        eprosima::utils::testing::LogChecker log_checker(
            eprosima::utils::Log::Kind::Warning);

        // No logs
        ASSERT_TRUE(log_checker.check_valid());

        // After log warning
        logWarning(LOGCHECKER_TEST, "Test propose log.");
        ASSERT_FALSE(log_checker.check_valid());
    }

    // Log error
    {
        eprosima::utils::testing::LogChecker log_checker(
            eprosima::utils::Log::Kind::Warning);

        // No logs
        ASSERT_TRUE(log_checker.check_valid());

        // After log error
        logError(LOGCHECKER_TEST, "Test propose log.");
        ASSERT_FALSE(log_checker.check_valid());
    }
}

/**
 * @brief Test that only logs of kind Error are used. 0 logs are allowed.
 */
TEST(LogCheckerTest, severe_logs_error)
{
    eprosima::utils::testing::LogChecker log_checker(
        eprosima::utils::Log::Kind::Error);

    // No logs
    ASSERT_TRUE(log_checker.check_valid());

    // After log info
    logInfo(LOGCHECKER_TEST, "Test propose log.");
    ASSERT_TRUE(log_checker.check_valid());

    // After log warning
    logWarning(LOGCHECKER_TEST, "Test propose log.");
    ASSERT_TRUE(log_checker.check_valid());

    // After log error
    logError(LOGCHECKER_TEST, "Test propose log.");
    ASSERT_FALSE(log_checker.check_valid());
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
