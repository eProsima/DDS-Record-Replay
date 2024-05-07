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

#include <mcap/types.hpp>

#include <ddspipe_core/types/dds/Payload.hpp>
#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Structure extending \c mcap::Message with Fast DDS payload and its owner (a \c PayloadPool).
 */
struct McapMessage : public mcap::Message
{
    McapMessage() = default;

    /**
     * Message copy constructor
     *
     * Copy message without copying payload through PayloadPool API (copy reference and increment counter).
     *
     * @note If using instead the default destructor and copy constructor, the destruction of the copied message would
     * free the newly constructed sample (payload's data attribute), thus rendering the latter useless.
     *
     */
    McapMessage(
            const McapMessage& msg);

    /**
     * Message destructor
     *
     * Releases internal payload, decrementing its reference count and freeing only when no longer referenced.
     *
     * @note Releasing the payload correctly sets payload's internal data attribute to \c nullptr , which eludes
     * the situation described in copy constructor's note.
     *
     */
    ~McapMessage();

    //! Serialized payload
    ddspipe::core::types::Payload payload{};

    //! Payload owner (reference to \c PayloadPool which created/reserved it)
    ddspipe::core::PayloadPool* payload_owner{nullptr};
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
