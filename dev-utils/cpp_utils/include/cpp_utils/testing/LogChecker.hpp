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

/**
 * @file LogChecker.hpp
 */

#pragma once

#include <memory>
#include <functional>

#include <cpp_utils/event/LogSevereEventHandler.hpp>

namespace eprosima {
namespace utils {
namespace testing {

/**
 * @brief This is an auxiliary class to check the logs produced in a test.
 *
 * The main idea is to create one of this objects at the beginning of a test execution, and it will
 * have a counter of the logs consumed (only those higher than threshold given).
 * At the end, \c check_valid() should be called in order to know if the logs consumed are between the minimum
 * and maximum logs expected.
 *
 * In order to automatically check that no warnings nor errors are produced by a test,
 * call \c DEFAULT_LOG_TESTER macro at the beginning of the test.
 * To use specific arguments, use \c INSTANTIATE_LOG_TESTER instead.
 */
class LogChecker
{
public:

    /**
     * @brief Construct a LogChecker object.
     *
     * @param threshold minimum log level that will be taken into account when counting logs consumed.
     * @param expected_severe_logs the number of logs this object expects to consume.
     * @param max_severe_logs the maximum number of logs this object will allow.
     */
    CPP_UTILS_DllAPI LogChecker(
            utils::Log::Kind threshold = utils::Log::Kind::Warning,
            unsigned int expected_severe_logs = 0,
            unsigned int max_severe_logs = 0);

    //! Default destructor.
    CPP_UTILS_DllAPI ~LogChecker() = default;

    /**
     * @brief Whether the logs consumed so far are between the limits expected.
     *
     * @return true if logs consumed are equal ot higher than \c expected_severe_logs
     * and equal or lower than \c max_severe_logs .
     * false otherwise.
     */
    CPP_UTILS_DllAPI bool check_valid();

protected:

    /**
     * @brief Log Handler object.
     *
     * It is a Severe one to only take into account those logs higher than threashold
     */
    utils::event::LogSevereEventHandler log_consumer_;

    //! Expected minimum number of logs.
    unsigned int expected_severe_logs_;

    //! Expected maximum number of logs.
    unsigned int max_severe_logs_;
};

/**
 * @brief Instantiate a LogChecker instance that assert that logs has been as expected when runs out of scope.
 *
 * This is an easy way to instantiate a LogChecker by using a unique ptr that autodestroys when exit the scope,
 * and in deletion it is checked that logs has been correct regarding the parameters, and ASSERT if not.
 *
 * @note this is just to avoid the user to write this code. It is needed because ASSERT_TRUE require gtest
 * to be installed and linked, and this could always not be the case.
 *
 * @example
 * TEST(FooTest, test_foo) {
 *   INSTANTIATE_LOG_TESTER(eprosima::utils::Log::Kind::Warning, 0, 0);
 *   // When test finishes, if any log warning has been raised, test will fail.
 * }
 */
#define INSTANTIATE_LOG_TESTER(threshold, expected, max) \
    std::unique_ptr< \
        eprosima::utils::testing::LogChecker, \
        std::function<void(eprosima::utils::testing::LogChecker*)>> \
    log_tester( \
        new eprosima::utils::testing::LogChecker(threshold, expected, max), \
        [](eprosima::utils::testing::LogChecker* t){ ASSERT_TRUE( t->check_valid()); delete t; \
        })

/**
 * This macro only calls \c INSTANTIATE_LOG_TESTER with parameters by default.
 * All positive tests cases should be able to run with this call (one day...).
 */
#define DEFAULT_LOG_TESTER INSTANTIATE_LOG_TESTER(eprosima::utils::Log::Kind::Warning, 0, 0)

} /* namespace testing */
} /* namespace utils */
} /* namespace eprosima */
