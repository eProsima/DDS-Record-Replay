// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file time_utils.cpp
 */

#include <chrono>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <sstream>

#include <cpp_utils/time/time_utils.hpp>

#include <ddsrecorder_participants/common/time_utils.hpp>


namespace eprosima {
namespace ddsrecorder {
namespace participants {

const auto SQL_TIMESTAMP_FORMAT = "%Y-%m-%d %H:%M:%S";
constexpr std::uint64_t NS_PER_SEC = 1000000000;

utils::Timestamp to_std_timestamp(
        const mcap::Timestamp& time)
{
    const auto timestamp = utils::Timestamp() + std::chrono::nanoseconds(time);
    return std::chrono::time_point_cast<utils::Timestamp::duration>(timestamp);
}

utils::Timestamp to_std_timestamp(
        const std::string& time)
{
    // Parse the nanoseconds from the fractional part of the timestamp
    const auto dot_pos = time.find('.');

    if (dot_pos == std::string::npos)
    {
        throw std::runtime_error("No dot found in the timestamp");
    }

    auto time_point = utils::string_to_timestamp(time.substr(0, dot_pos), SQL_TIMESTAMP_FORMAT);

    const auto decimals = time.substr(dot_pos + 1);
    std::uint32_t nanoseconds;
    std::istringstream(decimals) >> nanoseconds;

    if (decimals.empty() || std::istringstream(decimals).fail())
    {
        throw std::runtime_error("Failed to parse fractional part as an integer");
    }

    time_point += std::chrono::nanoseconds(nanoseconds);

    return time_point;
}

mcap::Timestamp to_mcap_timestamp(
        const fastrtps::rtps::Time_t& time)
{
    return time.seconds() * NS_PER_SEC + time.nanosec();
}

mcap::Timestamp to_mcap_timestamp(
        const utils::Timestamp& time)
{
    return mcap::Timestamp(to_ticks(time));
}

std::string to_sql_timestamp(
        const fastrtps::rtps::Time_t& time,
        const bool local_time /* = false */)
{
    char datetime_str[30];

    const std::time_t time_seconds = time.seconds();
    const auto time_seconds_zone = local_time ? std::localtime(&time_seconds) : std::gmtime(&time_seconds);
    std::strftime(datetime_str, sizeof(datetime_str), SQL_TIMESTAMP_FORMAT, time_seconds_zone);

    // Combine the date-time string with nanoseconds to form the full timestamp
    std::ostringstream oss;
    oss << datetime_str << '.' << std::setw(9) << std::setfill('0') << time.nanosec();

    return oss.str();
}

std::string to_sql_timestamp(
        const utils::Timestamp& time)
{
    std::ostringstream time_str;
    time_str << utils::timestamp_to_string(time, SQL_TIMESTAMP_FORMAT);

    auto nanoseconds = to_ticks(time) % NS_PER_SEC;
    time_str << '.' << std::setw(9) << std::setfill('0') << nanoseconds;

    return time_str.str();
}

std::uint64_t to_ticks(
        const utils::Timestamp& time)
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(time.time_since_epoch()).count();
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
