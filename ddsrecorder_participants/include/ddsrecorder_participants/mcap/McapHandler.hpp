// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file McapHandler.hpp
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <queue>
#include <thread>

#include <mcap/writer.hpp>

#include <cpp_utils/types/Atomicable.hpp>

#include <fastrtps/types/DynamicTypePtr.h>

#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>
#include <ddspipe_core/types/data/RtpsPayloadData.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddspipe_participants/participant/dynamic_types/ISchemaHandler.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

struct Message : public mcap::Message
{
    Message() = default;

    Message(
            const Message& msg)
    : mcap::Message(msg)
    {
        this->payload_owner = msg.payload_owner;
        auto payload_owner_ =
            const_cast<eprosima::fastrtps::rtps::IPayloadPool*>((eprosima::fastrtps::rtps::IPayloadPool*)msg.payload_owner);
        this->payload_owner->get_payload(
                msg.payload,
                payload_owner_,
                this->payload);
    }

    ~Message()
    {
        // If payload owner exists and payload has size, release it correctly in pool
        if (payload_owner && payload.length > 0)
        {
            payload_owner->release_payload(payload);
        }
    }

    ddspipe::core::types::Payload payload{};
    ddspipe::core::PayloadPool* payload_owner{nullptr};
};

/**
 * TODO
 */
class McapHandler : public ddspipe::participants::ISchemaHandler
{
public:

    McapHandler(
            const char* file_name,
            std::shared_ptr<ddspipe::core::PayloadPool> payload_pool,
            unsigned int max_pending_samples,
            unsigned int buffer_size,
            unsigned int downsampling,
            unsigned int event_window);
            // bool autostart = false);

    ~McapHandler();

    void add_schema(
            const fastrtps::types::DynamicType_ptr& dynamic_type) override;

    void add_data(
            const ddspipe::core::types::DdsTopic& topic,
            ddspipe::core::types::RtpsPayloadData& data) override;

    void start();

    void pause();

    void trigger_event();

protected:

    // enum StateCode
    // {
    //     UNSTARTED = 0,
    //     STARTED,
    //     PAUSED,
    // };

    void add_data_(
            const Message& msg);

    void add_pending_samples_(
            const std::string& schema_name);

    void event_thread_routine_();

    void stop_event_thread_();

    void clear_all_();

    void dump_data_();

    void dump_data_nts_();

    mcap::ChannelId create_channel_id_nts_(
            const ddspipe::core::types::DdsTopic& topic);

    mcap::ChannelId get_channel_id_(
            const ddspipe::core::types::DdsTopic& topic);

    mcap::SchemaId get_schema_id_(
            const std::string& schema_name);

    mcap::Timestamp now();
    mcap::Timestamp fastdds_timestamp_to_mcap_timestamp(const ddspipe::core::types::DataTime& time);

    // StateCode state_ = {UNSTARTED};

    // NOTE: it cannot be used with Atomicable because McapWriter is a final class
    mcap::McapWriter mcap_writer_;
    std::mutex write_mtx_;

    using SchemaMapType = utils::SharedAtomicable<std::map<std::string, mcap::Schema>>;
    SchemaMapType schemas_;

    using ChannelMapType = utils::SharedAtomicable<std::map<std::string, mcap::Channel>>;
    ChannelMapType channels_;

    using PendingSamplesMapType = utils::SharedAtomicable<std::map<std::string, std::queue<std::pair<std::string, Message>>>>;
    PendingSamplesMapType pending_samples_;
    unsigned int max_pending_samples_;

    std::queue<Message> samples_buffer_;
    unsigned int buffer_size_;

    unsigned int downsampling_;

    std::atomic<bool> paused_ = {false};
    unsigned int event_window_;
    std::thread event_thread_;
    bool event_triggered_ = false;
    std::condition_variable event_cv_;
    std::mutex event_cv_mutex_;

    std::atomic<unsigned int> unique_sequence_number_{0};

    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
