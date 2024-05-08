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
 * @file LogChecker.cpp
 *
 */

#include <cpp_utils/testing/LogChecker.hpp>

namespace eprosima {
namespace utils {
namespace testing {

LogChecker::LogChecker(
        utils::Log::Kind threshold, /* Log::Kind::Warning */
        uint32_t expected_severe_logs /* = 0 */,
        uint32_t max_severe_logs /* = 0 */)
    : log_consumer_(
        [](utils::Log::Entry entry)
        {
        },                              // dummy function
        threshold)
    , expected_severe_logs_(expected_severe_logs)
    , max_severe_logs_(std::max(max_severe_logs, expected_severe_logs)) // Use max to avoid forcing set both args
{
}

bool LogChecker::check_valid()
{
    eprosima::utils::Log::Flush();
    return
        (log_consumer_.event_count() >= expected_severe_logs_) &&
        (log_consumer_.event_count() <= max_severe_logs_);
}

} /* namespace testing */
} /* namespace utils */
} /* namespace eprosima */
