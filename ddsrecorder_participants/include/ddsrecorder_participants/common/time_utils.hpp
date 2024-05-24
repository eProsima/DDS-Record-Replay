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

#include <cstdint>
#include <string>

#include <mcap/types.hpp>

#include <fastdds/rtps/common/Time_t.h>

#include <cpp_utils/time/time_utils.hpp>

#include <ddsrecorder_participants/library/library_dll.h>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * @brief This method converts a mcap timestamp to standard format.
 *
 * @param [in] time Timestamp to be converted
 * @return Timestamp in standard format
 */
DDSRECORDER_PARTICIPANTS_DllAPI
utils::Timestamp to_std_timestamp(
        const mcap::Timestamp& time);

/**
 * @brief This method converts a string timestamp to standard format.
 *
 * @param [in] time Timestamp to be converted
 * @param [in] format Format of the timestamp
 */
utils::Timestamp to_std_timestamp(
        const std::string& time);

/**
 * @brief This method converts a timestamp in Fast DDS format to its mcap equivalent.
 *
 * @param [in] time Timestamp to be converted
 * @return Timestamp in mcap format
 */
DDSRECORDER_PARTICIPANTS_DllAPI
mcap::Timestamp to_mcap_timestamp(
        const fastrtps::rtps::Time_t& time);

/**
 * @brief This method converts a timestamp in standard format to its mcap equivalent.
 *
 * @param [in] time Timestamp to be converted
 * @return Timestamp in mcap format
 */
DDSRECORDER_PARTICIPANTS_DllAPI
mcap::Timestamp to_mcap_timestamp(
        const utils::Timestamp& time);

/**
 * @brief This method converts a timestamp in Fast DDS format to a SQL-friendly format.
 *
 * @param [in] time Timestamp to convert
 * @param [in] local_time Flag to indicate if the timestamp is in local time
 * @return String formatted as "YYYY-MM-DD HH:MM:SS"
 */
DDSRECORDER_PARTICIPANTS_DllAPI
std::string to_sql_timestamp(
        const fastrtps::rtps::Time_t& time,
        const bool local_time = false);

/**
 * @brief This method converts a timestamp in standard format to a SQL-friendly format.
 *
 * @param [in] time Timestamp to convert
 * @return String formatted as "YYYY-MM-DD HH:MM:SS"
 */
DDSRECORDER_PARTICIPANTS_DllAPI
std::string to_sql_timestamp(
        const utils::Timestamp& time);

/**
 * @brief This method converts a timestamp in standard format to nanoseconds.
 *
 * @param [in] time Timestamp to convert
 * @return Timestamp in nanoseconds
 */
DDSRECORDER_PARTICIPANTS_DllAPI
std::uint64_t to_ticks(
        const utils::Timestamp& time);

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
