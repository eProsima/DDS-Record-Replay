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

#include <condition_variable>
#include <list>
#include <map>
#include <queue>
#include <thread>

#include <mcap/writer.hpp>

#include <cpp_utils/time/time_utils.hpp>

#include <fastrtps/types/DynamicTypePtr.h>

#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>
#include <ddspipe_core/types/data/RtpsPayloadData.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddspipe_participants/participant/dynamic_types/ISchemaHandler.hpp>
#include <ddsrecorder_participants/mcap/McapHandlerConfiguration.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

struct Message : public mcap::Message
{
    Message() = default;

    /**
     * Message copy constructor
     *
     * Copy message without copying payload through PayloadPool API (copy reference and increment counter).
     *
     * @note If using instead the default destructor and copy constructor, the destruction of the copied message would
     * free the newly constructed sample (payload's data attribute), thus rendering the latter useless.
     *
     */
    Message(const Message& msg);

    /**
     * Message destructor
     *
     * Releases internal payload, decrementing its reference count and freeing only when no longer referenced.
     *
     * @note Releasing the payload correctly sets payload's internal data attribute to \c nullptr , which eludes
     * the situation described in copy constructor's note.
     *
     */
    ~Message();

    ddspipe::core::types::Payload payload{};
    ddspipe::core::PayloadPool* payload_owner{nullptr};
};

/**
 * TODO
 */
class McapHandler : public ddspipe::participants::ISchemaHandler
{
public:

    enum class StateCode
    {
        stopped = 0,
        started,
        paused,
    };

    McapHandler(
            const McapHandlerConfiguration& config,
            const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
            const StateCode& init_state = StateCode::started);

    ~McapHandler();

    void add_schema(
            const fastrtps::types::DynamicType_ptr& dynamic_type) override;

    void add_data(
            const ddspipe::core::types::DdsTopic& topic,
            ddspipe::core::types::RtpsPayloadData& data) override;

    void start();

    void stop();

    void pause();

    void trigger_event();

    static mcap::Timestamp fastdds_timestamp_to_mcap_timestamp(const ddspipe::core::types::DataTime& time);

    static mcap::Timestamp std_timepoint_to_mcap_timestamp(const utils::Timestamp& time);

    static mcap::Timestamp now();

protected:

    enum class EventCode
    {
        untriggered = 0,
        triggered,
        stopped,
    };

    void add_data_nts_(
            const Message& msg);

    void add_pending_samples_nts_(
            const std::string& schema_name);

    void event_thread_routine_();

    void remove_outdated_samples_nts_();

    void stop_event_thread_nts_();

    void clear_all_nts_();

    void dump_data_nts_();

    mcap::ChannelId create_channel_id_nts_(
            const ddspipe::core::types::DdsTopic& topic);

    mcap::ChannelId get_channel_id_nts_(
            const ddspipe::core::types::DdsTopic& topic);

    mcap::SchemaId get_schema_id_nts_(
            const std::string& schema_name);

    static std::string tmp_filename_(const std::string& filename);

    McapHandlerConfiguration configuration_;

    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    StateCode state_;

    std::mutex mtx_;

    mcap::McapWriter mcap_writer_;

    std::map<std::string, mcap::Schema> schemas_;

    std::map<std::string, mcap::Channel> channels_;

    std::list<Message> samples_buffer_;

    std::map<std::string, std::queue<std::pair<std::string, Message>>> pending_samples_;

    std::thread event_thread_;
    EventCode event_flag_ = EventCode::stopped;
    std::condition_variable event_cv_;
    std::mutex event_cv_mutex_;

    unsigned int unique_sequence_number_{0};

    unsigned int downsampling_idx_{0};
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
