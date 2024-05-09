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
 * @file BaseMessage.hpp
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include <ddspipe_core/types/data/RtpsPayloadData.hpp>
#include <ddspipe_core/types/dds/Payload.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>
#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>


namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Structure implementing a DDS Message with a Fast-DDS payload and its owner (a \c PayloadPool).
 */
struct BaseMessage
{
    BaseMessage() = default;

    /**
     * @brief TODO
     */
    BaseMessage(
            const ddspipe::core::types::Payload& payload,
            ddspipe::core::PayloadPool* payload_owner);

    /**
     * @brief TODO
     */
    BaseMessage(
            const ddspipe::core::types::RtpsPayloadData& data,
            std::shared_ptr<ddspipe::core::PayloadPool> payload_pool,
            const ddspipe::core::types::DdsTopic& topic,
            const bool log_publish_time);

    /**
     * @brief Message copy constructor
     *
     * Copy message without copying payload through PayloadPool API (copy reference and increment counter).
     *
     * @note If using instead the default destructor and copy constructor, the destruction of the copied message would
     * free the newly constructed sample (payload's data attribute), thus rendering the latter useless.
     */
    BaseMessage(
            const BaseMessage& msg);

    /**
     * @brief Message destructor
     *
     * Releases internal payload, decrementing its reference count and freeing only when no longer referenced.
     *
     * @note Releasing the payload correctly sets payload's internal data attribute to \c nullptr , which eludes
     * the situation described in copy constructor's note.
     */
    virtual ~BaseMessage();

    /**
     * @brief Get the message's payload data
     */
    std::byte* get_data() const;

    /**
     * @brief Get the message's payload size
     */
    std::uint32_t get_data_size() const;

    //! Serialized payload
    ddspipe::core::types::Payload payload{};

    //! Payload owner (reference to \c PayloadPool which created/reserved it)
    ddspipe::core::PayloadPool* payload_owner{nullptr};

    //! Topic in which the payload was published
    ddspipe::core::types::DdsTopic topic;

    //! When the message was recorded or received for recording
    ddspipe::core::types::DataTime log_time;

    //! When the message was initially published
    ddspipe::core::types::DataTime publish_time;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
