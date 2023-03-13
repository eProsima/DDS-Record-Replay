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

#include <cpp_utils/macros/custom_enumeration.hpp>
#include <cpp_utils/time/time_utils.hpp>

#include <fastrtps/types/DynamicTypePtr.h>

#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>
#include <ddspipe_core/types/data/RtpsPayloadData.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddspipe_participants/participant/dynamic_types/ISchemaHandler.hpp>

#include <ddsrecorder_participants/library/library_dll.h>
#include <ddsrecorder_participants/mcap/McapHandlerConfiguration.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

//! State of the handler instance
ENUMERATION_BUILDER(
    McapHandlerStateCode,
    STOPPED,                  //! Received messages and schemas are not processed.
    RUNNING,                  //! Messages are stored in buffer and dumped to disk when full.
    PAUSED                    //! Messages are stored in buffer and dumped to disk when event triggered.
    );

/**
 * Structure extending \c mcap::Message with Fast DDS payload and its owner (a \c PayloadPool).
 */
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
    Message(
            const Message& msg);

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

    //! Serialized payload
    ddspipe::core::types::Payload payload{};

    //! Payload owner (reference to \c PayloadPool which created/reserved it)
    ddspipe::core::PayloadPool* payload_owner{nullptr};
};

/**
 * Class that manages the interaction between DDS Pipe (\c SchemaParticipant) and MCAP files through mcap library.
 * Payloads are efficiently passed from DDS Pipe to mcap without copying data (only references).
 *
 * @implements ISchemaHandler
 */
class McapHandler : public ddspipe::participants::ISchemaHandler
{
public:

    /**
     * McapHandler constructor by required values.
     *
     * Creates McapHandler instance with given configuration, payload pool and initial state.
     * Opens temporal MCAP file where data is to be written.
     *
     * @param config:       Structure encapsulating all configuration options.
     * @param payload_pool: Owner of every payload contained in received messages.
     * @param init_state:   Initial instance state (RUNNING/PAUSED/STOPPED).
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    McapHandler(
            const McapHandlerConfiguration& config,
            const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
            const McapHandlerStateCode& init_state = McapHandlerStateCode::RUNNING);

    /**
     * @brief Destructor
     *
     * Closes temporal MCAP file, and renames it with filename given in configuration.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    ~McapHandler();

    /**
     * @brief Create and store in \c schemas_ a ROS 2 schema (.msg format).
     * Any samples following this schema that were received before the schema itself are moved to the memory buffer
     * to be written with the next batch.
     *
     * If instance is STOPPED, received schema is not processed.
     *
     * @param [in] dynamic_type DynamicType containing the type information required to generate the schema.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void add_schema(
            const fastrtps::types::DynamicType_ptr& dynamic_type) override;

    /**
     * @brief Save in memory (\c buffer_) the received sample, to be written through a mcap \c Channel associated to
     * the given \c topic.
     *
     * If channel does not exist, its creation is attempted. If this fails (schema not available yet), the sample is
     * inserted into \c pending_samples_ queue until its schema is received.
     *
     * If instance is STOPPED, received data is not processed.
     *
     * @param [in] topic DDS topic associated to this sample.
     * @param [in] data Message to be added.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void add_data(
            const ddspipe::core::types::DdsTopic& topic,
            ddspipe::core::types::RtpsPayloadData& data) override;

    /**
     * @brief Start handler instance
     *
     * If previous state was PAUSED, the event thread is stopped (and buffer is cleared).
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void start();

    /**
     * @brief Stop handler instance
     *
     * If previous state was RUNNING, data stored in buffer is dumped to disk.
     * If previous state was PAUSED, the event thread is stopped (and buffer is cleared).
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void stop();

    /**
     * @brief Pause handler instance
     *
     * Creates event thread waiting for an event to dump samples in buffer.
     *
     * If previous state was RUNNING, data stored in buffer is dumped to disk and pending samples cleared.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void pause();

    /**
     * @brief Trigger event.
     *
     * When an event is triggered, data stored in buffer (containing samples received during the last event_window
     * seconds) is written to disk.
     *
     * This method is ineffective if instance state is different than PAUSED.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void trigger_event();

    /**
     * @brief This method converts a timestamp in Fast DDS format to its mcap equivalent.
     *
     * @param [in] time Timestamp to be converted
     * @return Timestamp in mcap format
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    static mcap::Timestamp fastdds_timestamp_to_mcap_timestamp(
            const ddspipe::core::types::DataTime& time);

    /**
     * @brief This method converts a timestamp in standard format to its mcap equivalent.
     *
     * @param [in] time Timestamp to be converted
     * @return Timestamp in mcap format
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    static mcap::Timestamp std_timepoint_to_mcap_timestamp(
            const utils::Timestamp& time);

    /**
     * @brief Get current time point in mcap format.
     *
     * @return Current time in mcap format
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    static mcap::Timestamp now();

protected:

    //! Flag code controlling the event thread routine
    enum class EventCode
    {
        untriggered = 0,        //! Waiting for an event to be received.
        triggered,              //! Indicates that an event has been triggered.
        stopped,                //! Signals event thread to exit.
    };

    /**
     * @brief Add message to \c buffer_ structure.
     *
     * If after adding the new sample the buffer reaches its maximum size, the content is dumped to disk.
     *
     * @param [in] msg Message to be added
     */
    void add_data_nts_(
            const Message& msg);

    /**
     * @brief Add any samples stored in \c pending_samples_ structure associated to \c schema_name
     *
     * @param [in] schema_name Name of the schema for which pending samples using it are added.
     */
    void add_pending_samples_nts_(
            const std::string& schema_name);

    /**
     * @brief Wait for an event trigger to write in disk samples from buffer.
     *
     * Every \c cleanup_period seconds, and before dumping data to disk, samples older than [now - event_window] are
     * removed. This way, when an event is triggered, only the samples received in the last \c event_window seconds
     * are kept.
     *
     * The loop is exited when \c event_flag_ is set to \c stopped.
     *
     */
    void event_thread_routine_();

    //! Remove samples in buffer older than [now - event_window]
    void remove_outdated_samples_nts_();

    //! Stop event thread, and clear \c samples_buffer_ and \c pending_samples_ structures
    void stop_event_thread_nts_();

    //! Clear \c samples_buffer_ and \c pending_samples_ structures
    void clear_all_nts_();

    //! Write in disk samples stored in buffer
    void dump_data_nts_();

    /**
     * @brief Create and add to \c mcap_writer_ channel associated to given \c topic
     *
     * @throw InconsistencyException if creation fails (schema not found).
     *
     * @param [in] topic Topic associated to the channel to be created
     */
    mcap::ChannelId create_channel_id_nts_(
            const ddspipe::core::types::DdsTopic& topic);

    /**
     * @brief Attempt to get channel associated to given \c topic, and attempt to create one if not found.
     *
     * @throw InconsistencyException if not found, and creation fails (schema not found).
     *
     * @param [in] topic Topic associated to the channel to be created
     */
    mcap::ChannelId get_channel_id_nts_(
            const ddspipe::core::types::DdsTopic& topic);

    /**
     * @brief Attempt to get schema with name \c schema_name.
     *
     * @throw InconsistencyException if not found.
     *
     * @param [in] schema_name Name of the schema to get.
     */
    mcap::SchemaId get_schema_id_nts_(
            const std::string& schema_name);

    /**
     * @brief Convert given \c filename to temporal format.
     *
     * @param [in] filename Filename to be converted.
     */
    static std::string tmp_filename_(
            const std::string& filename);

    //! Handler configuration
    McapHandlerConfiguration configuration_;

    //! Payload pool
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    //! Handler instance state
    McapHandlerStateCode state_;

    //! Mutex synchronizing state transitions
    std::mutex state_mtx_;

    //! MCAP writer
    mcap::McapWriter mcap_writer_;

    //! Schemas map
    std::map<std::string, mcap::Schema> schemas_;

    //! Channels map
    std::map<std::string, mcap::Channel> channels_;

    //! Samples buffer
    std::list<Message> samples_buffer_;

    //! Pending samples map
    std::map<std::string, std::queue<std::pair<std::string, Message>>> pending_samples_;

    //! Mutex synchronizing access to object's data structures
    std::mutex mtx_;

    //! Event thread
    std::thread event_thread_;

    //! Event flag
    EventCode event_flag_ = EventCode::stopped;

    //! Event condition variable
    std::condition_variable event_cv_;

    //! Event condition variable mutex
    std::mutex event_cv_mutex_;

    //! Unique sequence number assigned to received messages. It is incremented with every sample added.
    unsigned int unique_sequence_number_{0};
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
