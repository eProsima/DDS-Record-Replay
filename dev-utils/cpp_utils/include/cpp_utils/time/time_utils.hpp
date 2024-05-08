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

/**
 * @file time_utils.hpp
 */

#pragma once

#include <chrono>
#include <string>

#include <cpp_utils/library/library_dll.h>

namespace eprosima {
namespace utils {

//! Type of Duration in milliseconds
using Duration_ms = uint32_t;

/**
 * Type used to represent time points
 */
using Timestamp = std::chrono::time_point<std::chrono::system_clock>;

/**
 * @brief Now time
 *
 * @return Timestamp refering to the moment it is called
 */
CPP_UTILS_DllAPI Timestamp now() noexcept;

//! Returns the maximum time available for \c Timestamp
CPP_UTILS_DllAPI Timestamp the_end_of_time() noexcept;

//! Returns the minimum time available for \c Timestamp
CPP_UTILS_DllAPI Timestamp the_beginning_of_time() noexcept;

//! Construct a \c Timestamp given a date and time.
CPP_UTILS_DllAPI Timestamp date_to_timestamp(
        unsigned int year,
        unsigned int month,
        unsigned int day,
        unsigned int hour = 0,
        unsigned int minute = 0,
        unsigned int second = 0);

//! Construct a \c Timestamp given a time (uses current date).
CPP_UTILS_DllAPI Timestamp time_to_timestamp(
        unsigned int hour = 0,
        unsigned int minute = 0,
        unsigned int second = 0);

/**
 * @brief Convert a \c Timestamp to a string following a specific format.
 *
 * @param timestamp value of the timestamp to parse.
 * @param format string formatting the date.
 * @param local_time whether to use the local time zone or UTC.
 *
 * @return string with the timestamp in the format given
 *
 * @note check how to use the string format here: https://en.cppreference.com/w/cpp/io/manip/put_time
 */
CPP_UTILS_DllAPI std::string timestamp_to_string(
        const Timestamp& timestamp,
        const std::string& format = "%Y-%m-%d_%H-%M-%S",
        bool local_time = false);

/**
 * @brief Convert to \c Timestamp a string following a specific format.
 *
 * @param timestamp string to parse.
 * @param format string formatting the date.
 * @param local_time whether to use the local time zone or UTC.
 *
 * @return timestamp from the string in the format given
 */
CPP_UTILS_DllAPI Timestamp string_to_timestamp(
        const std::string& timestamp,
        const std::string& format = "%Y-%m-%d_%H-%M-%S",
        bool local_time = false);

CPP_UTILS_DllAPI std::chrono::milliseconds duration_to_ms(
        const Duration_ms& duration) noexcept;

CPP_UTILS_DllAPI void sleep_for(
        const Duration_ms& sleep_time) noexcept;

} /* namespace utils */
} /* namespace eprosima */
