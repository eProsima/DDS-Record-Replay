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
 * @file McapMessage.cpp
 */

#include <ddsrecorder_participants/recorder/mcap/utils.hpp>
#include <ddsrecorder_participants/recorder/message/McapMessage.hpp>


namespace eprosima {
namespace ddsrecorder {
namespace participants {

std::atomic<std::uint32_t> McapMessage::number_of_msgs = 0;

McapMessage::McapMessage(
    const ddspipe::core::types::RtpsPayloadData& data,
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool,
    const ddspipe::core::types::DdsTopic& topic,
    const mcap::ChannelId channel_id,
    const bool log_publish_time)
    : BaseMessage(data, payload_pool, topic, log_publish_time)
    , mcap::Message()
{
    sequence = number_of_msgs.fetch_add(1);
    channelId = channel_id;

    this->data = get_data();
    dataSize = get_data_size();

    publishTime = to_mcap_timestamp(publish_time);
    logTime = to_mcap_timestamp(log_time);
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
