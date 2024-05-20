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
 * @file utils.hpp
 */

#pragma once

#include <string>

#include <fastdds/rtps/common/Time_t.h>

#include <cpp_utils/time/time_utils.hpp>

#include <ddsrecorder_participants/library/library_dll.h>


namespace eprosima {
namespace ddsrecorder {
namespace participants {


/**
 * @brief This method converts a timestamp in Fast DDS format to a SQL-friendly format.
 *
 * @param [in] time Timestamp to convert
 * @return String formatted as "YYYY-MM-DD HH:MM:SS"
 */
DDSRECORDER_PARTICIPANTS_DllAPI
std::string to_sql_timestamp(
        const fastrtps::rtps::Time_t& time);

/**
 * @brief This method converts a timestamp in standard format to a SQL-friendly format.
 *
 * @param [in] time Timestamp to convert
 * @return String formatted as "YYYY-MM-DD HH:MM:SS"
 */
DDSRECORDER_PARTICIPANTS_DllAPI
std::string to_sql_timestamp(
        const utils::Timestamp& time);


} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */