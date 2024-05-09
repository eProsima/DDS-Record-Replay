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
 * @file BaseMessage.cpp
 */

#include <fastdds/rtps/history/IPayloadPool.h>

#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/time/time_utils.hpp>

#include <ddsrecorder_participants/recorder/message/BaseMessage.hpp>


namespace eprosima {
namespace ddsrecorder {
namespace participants {

BaseMessage::BaseMessage(
    const ddspipe::core::types::Payload& payload,
    ddspipe::core::PayloadPool* payload_owner)
    : payload_owner(payload_owner)
{
    if (payload.length == 0)
    {
        throw utils::InconsistencyException(
                    STR_ENTRY << "Received sample with no payload."
                    );
    }

    auto ipayload_owner = static_cast<fastrtps::rtps::IPayloadPool*>(payload_owner);

    if (ipayload_owner == nullptr)
    {
        throw utils::InconsistencyException(
                    STR_ENTRY << "Payload owner not found in data received."
                    );
    }

    payload_owner->get_payload(
        payload,
        ipayload_owner,
        this->payload);
}

BaseMessage::BaseMessage(
    const ddspipe::core::types::RtpsPayloadData& data,
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool,
    const ddspipe::core::types::DdsTopic& topic,
    const bool log_publish_time)
    : BaseMessage(data.payload, payload_pool.get())
{
    this->topic = topic;
    publish_time = data.source_timestamp;

    if (log_publish_time)
    {
        log_time = publish_time;
    }
    else
    {
        ddspipe::core::types::DataTime::now(log_time);
    }
}

BaseMessage::BaseMessage(
        const BaseMessage& msg)
    : BaseMessage(msg.payload, msg.payload_owner)
{
    topic = msg.topic;
    log_time = msg.log_time;
    publish_time = msg.publish_time;
}

BaseMessage::~BaseMessage()
{
    // If payload owner exists and payload has size, release it correctly in pool
    if (payload_owner && payload.length > 0)
    {
        payload_owner->release_payload(payload);
    }
}

std::byte* BaseMessage::get_data() const
{
    return reinterpret_cast<std::byte*>(payload.data);
}

std::uint32_t BaseMessage::get_data_size() const
{
    return payload.length;
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
