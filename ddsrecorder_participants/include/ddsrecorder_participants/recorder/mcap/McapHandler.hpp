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
#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <stdexcept>
#include <thread>

#include <mcap/mcap.hpp>

#include <cpp_utils/macros/custom_enumeration.hpp>
#include <cpp_utils/time/time_utils.hpp>

#include <fastrtps/types/DynamicTypePtr.h>

#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>
#include <ddspipe_core/types/data/RtpsPayloadData.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddspipe_participants/participant/dynamic_types/ISchemaHandler.hpp>

#include <ddsrecorder_participants/library/library_dll.h>
#include <ddsrecorder_participants/recorder/mcap/McapHandlerConfiguration.hpp>
#include <ddsrecorder_participants/recorder/mcap/McapWriter.hpp>
#include <ddsrecorder_participants/recorder/mcap/Message.hpp>
#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>

#if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v1/DynamicTypesCollection.hpp>
#else
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v2/DynamicTypesCollection.hpp>
#endif // if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13

namespace eprosima {
namespace ddsrecorder {
namespace participants {

//! State of the handler instance
ENUMERATION_BUILDER(
    McapHandlerStateCode,
    STOPPED,                  //! Received messages are not processed.
    RUNNING,                  //! Messages are stored in buffer and dumped to disk when full.
    PAUSED                    //! Messages are stored in buffer and dumped to disk when event triggered.
    );

/**
 * Class that manages the interaction between DDS Pipe (\c SchemaParticipant) and MCAP files through mcap library.
 * Payloads are efficiently passed from DDS Pipe to mcap without copying data (only references).
 *
 * @implements ISchemaHandler
 */
class McapHandler : public ddspipe::participants::ISchemaHandler
{
public:

    using pending_list = std::list<std::pair<ddspipe::core::types::DdsTopic, Message>>;

    /**
     * McapHandler constructor by required values.
     *
     * Creates McapHandler instance with given configuration, payload pool and initial state.
     * Opens temporal MCAP file where data is to be written.
     *
     * @throw InitializationException if creation fails (fail to open MCAP file).
     *
     * @warning Command methods (\c start , \c pause , \c stop , and \c trigger_event) are not thread safe
     * among themselves. This is, they are expected to be executed sequentially and all in the same thread.
     *
     * @param config:       Structure encapsulating all configuration options.
     * @param payload_pool: Owner of every payload contained in received messages.
     * @param init_state:   Initial instance state (RUNNING/PAUSED/STOPPED).
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    McapHandler(
            const McapHandlerConfiguration& config,
            const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
            std::shared_ptr<ddsrecorder::participants::FileTracker> file_tracker,
            const McapHandlerStateCode& init_state = McapHandlerStateCode::RUNNING);

    /**
     * @brief Destructor
     *
     * Closes temporal MCAP file, and renames it with filename given in configuration.
     * Before closing file, received dynamic types are serialized and stored as an attachment.
     *
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    ~McapHandler();

    /**
     * @brief Create and store in \c schemas_ an OMG IDL (.idl format) or ROS 2 (.msg format) schema.
     * Any samples following this schema that were received before the schema itself are moved to the memory buffer
     * to be written with the next batch.
     * Previously created channels (for this type) associated with a blank schema are updated to use the new one.
     *
     * @param [in] dynamic_type DynamicType containing the type information required to generate the schema.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void add_schema(
            const fastrtps::types::DynamicType_ptr& dynamic_type) override;

    /**
     * @brief Add a data sample, to be written through a mcap \c Channel associated to the given \c topic.
     *
     * If a channel with (non-blank) schema exists, the sample is saved in memory \c buffer_ .
     * Otherwise:
     *   if RUNNING -> the sample is inserted into \c pending_samples_ queue if max pending samples is not 0.
     *                 If 0, the sample is added to buffer without schema if allowed (only_with_schema not true),
     *                 and discarded otherwise.
     *   if PAUSED  -> the sample is inserted into \c pending_samples_paused_ queue.
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
     * If previous state was PAUSED, the event thread is stopped (and buffers are cleared).
     *
     * @warning Not thread safe with respect to other command methods ( \c start , \c pause , \c stop ,
     * and \c trigger_event). This is, they are expected to be executed sequentially and all in the same thread.
     *
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void start();

    /**
     * @brief Stop handler instance
     *
     * If previous state was RUNNING, data stored in buffer is dumped to disk.
     * If previous state was PAUSED, the event thread is stopped (and buffers are cleared).
     * In both cases, pending samples are stored without schema if allowed (only_with_schema not true).
     *
     * @param [in] on_destruction Whether this command is executed on object's destruction.
     *
     * @warning Not thread safe with respect to other command methods ( \c start , \c pause , \c stop ,
     * and \c trigger_event). This is, they are expected to be executed sequentially and all in the same thread.
     *
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void stop(
            bool on_destruction = false);

    /**
     * @brief Pause handler instance
     *
     * Creates event thread waiting for an event to dump samples in buffer.
     *
     * If previous state was RUNNING, data stored in buffer is dumped to disk.
     *
     * @warning Not thread safe with respect to other command methods ( \c start , \c pause , \c stop ,
     * and \c trigger_event). This is, they are expected to be executed sequentially and all in the same thread.
     *
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
     *
     * @warning Not thread safe with respect to other command methods ( \c start , \c pause , \c stop ,
     * and \c trigger_event). This is, they are expected to be executed sequentially and all in the same thread.
     *
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

    /**
     * @brief Set the callback that should be called whenever disk is full
     *
     * It sets \c on_disk_full_lambda_set_ to true
     *
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void set_on_disk_full_callback(
            std::function<void()> on_disk_full_lambda) noexcept;

protected:

    //! Flag code controlling the event thread routine
    enum class EventCode
    {
        untriggered = 0,        //! Waiting for an event to be received.
        triggered,              //! Indicates that an event has been triggered.
        stopped,                //! Signals event thread to exit.
    };

    /**
     * @brief Add message to \c buffer_ structure, or directly write to MCAP file.
     *
     * If after adding the new sample (when not directly writting to file) the buffer reaches its maximum size, the
     * content is dumped to disk.
     *
     * @param [in] msg Message to be added
     * @param [in] direct_write Whether to directly store in MCAP file
     */
    void add_data_nts_(
            const Message& msg,
            bool direct_write = false);

    /**
     * @brief Add message with given topic.
     *
     * First, it is attempted to get a channel given \c topic to be associated with the message.
     * If this fails, the sample is not added.
     *
     * @param [in] msg Message to be added
     * @param [in] topic Topic of message to be added
     * @param [in] direct_write Whether to directly store in MCAP file
     */
    void add_data_nts_(
            Message& msg,
            const ddspipe::core::types::DdsTopic& topic,
            bool direct_write = false);

    /**
     * @brief Add to pending samples collection.
     *
     * If pending samples collection is full, the oldest message is popped and written (if only_with_schema not true).
     *
     * @param [in] msg Message to be added
     * @param [in] topic Topic of message to be added
     */
    void add_to_pending_nts_(
            Message& msg,
            const ddspipe::core::types::DdsTopic& topic);

    /**
     * @brief Add any pending samples associated to \c schema_name
     *
     * If in PAUSED state, samples in \c pending_samples_paused_ structure for this schema are moved to the buffer,
     * so they will be written to file later on if event triggered.
     *
     * Samples in \c pending_samples_ structure for this schema are to be written irrespectively of the current state.
     * However, in RUNNING/STOPPED states these are moved to buffer (to be written together with the next batch),
     * while in PAUSED state they are directly written to the file (to avoid being deleted by event thread).
     * Note that in the last case, pending samples correspond to messages that were previously received in RUNNING
     * state, and hence should be stored regardless of whether or not an event is triggered.
     *
     * @param [in] schema_name Name of the schema for which pending samples using it are added.
     */
    void add_pending_samples_nts_(
            const std::string& schema_name);

    /**
     * @brief Add pending samples.
     *
     * Add/write and pop all pending samples from the given list.
     *
     * @param [in] pending_samples List of pending samples to be added
     * @param [in] direct_write Whether to directly store in MCAP file
     */
    void add_pending_samples_nts_(
            pending_list& pending_samples,
            bool direct_write = false);

    /**
     * @brief Add all samples stored in \c pending_samples_ structure, associating each of them to a blank schema.
     *
     */
    void add_pending_samples_nts_();

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

    //! Remove buffered samples older than [now - event_window]
    void remove_outdated_samples_nts_();

    /**
     * @brief Stop event thread, and clear \c samples_buffer_ and \c pending_samples_paused_ structures
     *
     * A (locked) lock wrapping \c event_cv_mutex_ is passed so it can be released just before joining the thread.
     *
     * @param [in] event_lock Lock in locked state wrapping \c event_cv_mutex_
     */
    void stop_event_thread_nts_(
            std::unique_lock<std::mutex>& event_lock);

    //! Write in disk samples stored in buffer
    void dump_data_nts_();

    /**
     * @brief Create and add to \c mcap_writer_ channel associated to given \c topic
     *
     * A channel with blank schema is created when none found, unless only_with_schema true.
     *
     * @throw InconsistencyException if creation fails (schema not found and only_with_schema true).
     *
     * @param [in] topic Topic associated to the channel to be created
     */
    mcap::ChannelId create_channel_id_nts_(
            const ddspipe::core::types::DdsTopic& topic);

    /**
     * @brief Attempt to get channel associated to given \c topic, and attempt to create one if not found.
     *
     * @throw InconsistencyException if not found, and creation fails (schema not found and only_with_schema true).
     *
     * @param [in] topic Topic associated to the channel to be created
     */
    mcap::ChannelId get_channel_id_nts_(
            const ddspipe::core::types::DdsTopic& topic);

    /**
     * @brief Update channels with \c old_schema_id to use \c new_schema_id instead.
     *
     * Its main purpose is to update channels previously created with blank schema after having received their
     * corresponding topic type.
     *
     * @param [in] old_schema_id Schema id used by the channels to be updated
     * @param [in] new_schema_id Schema id with which to update channels (using \c old_schema_id)
     */
    void update_channels_nts_(
            const mcap::SchemaId& old_schema_id,
            const mcap::SchemaId& new_schema_id);

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
     * @brief Serialize type identifier and object, and insert the result into a \c DynamicTypesCollection .
     *
     * @param [in] type_name Name of the type to be stored, used as key in \c dynamic_types map.
     */
    void store_dynamic_type_(
            const std::string& type_name,
            DynamicTypesCollection& dynamic_types) const;

    /**
     * @brief Serialize type identifier and object, and insert the result into a \c DynamicTypesCollection .
     *
     * @param [in] type_identifier Type identifier to be serialized and stored.
     * @param [in] type_object Type object to be serialized and stored.
     * @param [in] type_name Name of the type to be stored, used as key in \c dynamic_types map.
     * @param [in,out] dynamic_types Collection where to store serialized dynamic type.
     */
    void store_dynamic_type_(
            const eprosima::fastrtps::types::TypeIdentifier* type_identifier,
            const eprosima::fastrtps::types::TypeObject* type_object,
            const std::string& type_name,
            DynamicTypesCollection& dynamic_types) const;

    /**
     * @brief Serialize given \c DynamicTypesCollection into a \c SerializedPayload .
     *
     * @param [in] dynamic_types Dynamic types collection to be serialized.
     * @return Serialized payload for the given dynamic types collection.
     */
    fastrtps::rtps::SerializedPayload_t* serialize_dynamic_types_(
            DynamicTypesCollection& dynamic_types) const;

    /**
     * @brief Callback to be executed whenever disk is full
     *
     * It calls \c on_disk_full_lambda_ if set
     *
     */
    void on_disk_full_() const noexcept;

    /**
     * @brief Serialize a \c TopicQoS struct into a string.
     *
     * @param [in] qos TopicQoS to be serialized
     * @return Serialized TopicQoS string
     */
    static std::string serialize_qos_(
            const ddspipe::core::types::TopicQoS& qos);

    /**
     * @brief Serialize a \c TypeIdentifier into a string.
     *
     * @param [in] type_identifier TypeIdentifier to be serialized
     * @return Serialized TypeIdentifier string
     */
    static std::string serialize_type_identifier_(
            const eprosima::fastrtps::types::TypeIdentifier* type_identifier);

    /**
     * @brief Serialize a \c TypeObject into a string.
     *
     * @param [in] type_object TypeObject to be serialized
     * @return Serialized TypeObject string
     */
    static std::string serialize_type_object_(
            const eprosima::fastrtps::types::TypeObject* type_object);

    //! Handler configuration
    McapHandlerConfiguration configuration_;

    //! Payload pool
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    //! Handler instance state
    McapHandlerStateCode state_;

    //! MCAP writer
    McapWriter mcap_writer_;

    //! Schemas map
    std::map<std::string, mcap::Schema> schemas_;

    //! Received types set
    std::set<std::string> received_types_;

    //! Channels map
    std::map<ddspipe::core::types::DdsTopic, mcap::Channel> channels_;

    //! Samples buffer
    std::list<Message> samples_buffer_;

    //! Dynamic types collection
    DynamicTypesCollection dynamic_types_;

    //! Structure where messages (received in RUNNING state) with unknown type are kept
    std::map<std::string, pending_list> pending_samples_;

    //! Structure where messages (received in PAUSED state) with unknown type are kept
    std::map<std::string, pending_list> pending_samples_paused_;

    //! Mutex synchronizing state transitions and access to object's data structures
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

    //! Lambda to call when disk limit is reached
    std::function<void()> on_disk_full_lambda_;

    //! True if lambda callback is set
    bool on_disk_full_lambda_set_;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
