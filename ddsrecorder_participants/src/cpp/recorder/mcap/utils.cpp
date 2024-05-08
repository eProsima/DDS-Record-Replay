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

#include <chrono>
#include <cstdint>

#include <ddsrecorder_participants/recorder/mcap/utils.hpp>


namespace eprosima {
namespace ddsrecorder {
namespace participants {

mcap::Timestamp to_mcap_timestamp(
        const fastrtps::rtps::Time_t& time)
{
    static constexpr std::uint64_t NS_PER_SEC = 1000000000;
    return time.seconds() * NS_PER_SEC + time.nanosec();
}

mcap::Timestamp to_mcap_timestamp(
        const utils::Timestamp& time)
{
    return mcap::Timestamp(std::chrono::duration_cast<std::chrono::nanoseconds>(time.time_since_epoch()).count());
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
