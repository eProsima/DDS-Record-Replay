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
 * @file McapMessage.hpp
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <memory>

#include <mcap/types.hpp>

#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>
#include <ddspipe_core/types/dds/Payload.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddsrecorder_participants/recorder/message/BaseMessage.hpp>


namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Structure extending a \c BaseMessage for the \c mcap library.
 */
struct McapMessage : public BaseMessage, public mcap::Message
{
    McapMessage() = default;

    /**
     * @brief Construct a \c McapMessage.
     */
    McapMessage(
        const ddspipe::core::types::RtpsPayloadData& data,
        std::shared_ptr<ddspipe::core::PayloadPool> payload_pool,
        const ddspipe::core::types::DdsTopic& topic,
        const mcap::ChannelId channel_id,
        const bool log_publish_time);

    // Number of McapMessages created
    static std::atomic<std::uint32_t> number_of_msgs;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
