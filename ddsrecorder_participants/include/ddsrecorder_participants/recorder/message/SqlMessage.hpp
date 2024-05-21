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
 * @file SqlMessage.hpp
 */

#pragma once

#include <memory>
#include <string>

#include <fastrtps/types/DynamicType.h>

#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>
#include <ddspipe_core/types/data/RtpsPayloadData.hpp>
#include <ddspipe_core/types/dds/Payload.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddsrecorder_participants/recorder/message/BaseMessage.hpp>


namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Structure extending a \c BaseMessage for SQLite.
 */
struct SqlMessage : public BaseMessage
{
    SqlMessage() = default;

    /**
     * @brief Construct a \c SqlMessage.
     */
    SqlMessage(
        const ddspipe::core::types::RtpsPayloadData& payload,
        std::shared_ptr<ddspipe::core::PayloadPool> payload_pool,
        const ddspipe::core::types::DdsTopic& topic,
        const bool log_publish_time,
        const std::string& key = "");

    /**
     * @brief Set the key of the message.
     *
     * The following steps are performed:
     * - Deserialize the payload into a DynamicData.
     * - Find the key fields in the DynamicData.
     * - Find the values of the key fields.
     * - Serialize the values into a JSON string.
     *
     * @param dynamic_type DynamicType of the message.
     */
    void set_key(
            fastrtps::types::DynamicType_ptr dynamic_type);

    // Hashed value identifying the instance
    ddspipe::core::types::InstanceHandle instance_handle;

    // String containing the JSON-serialized instance key
    std::string key;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
