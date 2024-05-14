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
 * @file utils.cpp
 */

#include <ctime>

#include <cpp_utils/time/time_utils.hpp>

#include <ddsrecorder_participants/recorder/sql/utils.hpp>


namespace eprosima {
namespace ddsrecorder {
namespace participants {

const auto SQL_TIMESTAMP_FORMAT = "%Y-%m-%d %H:%M:%S";

std::string to_sql_timestamp(
        const fastrtps::rtps::Time_t& time)
{
    char datetime_str[20];

    const std::time_t time_seconds = time.seconds();
    std::strftime(datetime_str, sizeof(datetime_str), SQL_TIMESTAMP_FORMAT, std::localtime(&time_seconds));

    return std::string(datetime_str);
}

std::string to_sql_timestamp(
        const utils::Timestamp& time)
{
    return utils::timestamp_to_string(time, SQL_TIMESTAMP_FORMAT, true);
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
