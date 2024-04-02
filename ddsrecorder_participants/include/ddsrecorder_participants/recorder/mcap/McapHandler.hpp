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
#include <filesystem>
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

protected:

    //! Flag code controlling the event thread routine
    enum class EventCode
    {
        untriggered = 0,        //! Waiting for an event to be received.
        triggered,              //! Indicates that an event has been triggered.
        stopped,                //! Signals event thread to exit.
    };

    /**
     * @brief Open a new MCAP file according to configuration settings.
     *
     * @throw InitializationException if failing to open file.
     *
     * A temporal suffix is appended after the '.mcap' extension, and additionally a timestamp prefix if applies.
     *
     */
    void open_file_nts_();

    /**
     * @brief Close the file previously opened with \c open_file_nts_
     *
     * Before closure, the information relative to version and received dynamic types is written to file.
     *
     * After closure, the temporal file is renamed so no longer has a temporal suffix.
     *
     */
    void close_file_nts_();

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
     * @brief Write message to MCAP file.
     *
     * @throw InconsistencyException if failing to write message.
     *
     * @param [in] msg Message to be written
     */
    void write_message_nts_(
            const Message& msg);

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
     * @brief Rewrite all received schemas into currently open MCAP file.
     *
     */
    void rewrite_schemas_nts_();

    /**
     * @brief Generate dynamic type from type_name and save in in \c dynamic_types_ .
     *
     * @param [in] type_name Name of the dynamic type to generate
     */
    void save_dynamic_type_(
        const std::string& type_name);

    /**
     * @brief Serialize current dynamic types every time a new dynamic type is saved in \c save_dynamic_type_ .
     *
     */
    void serialize_dynamic_types_();

    /**
     * @brief Write in MCAP attachments.
     *
     * Its main purpose is to write the dynamic types associated to all added schemas, and their dependencies.
     *
     */
    void write_attachment_();

    /**
     * @brief Write version metadata (release and commit hash) in MCAP file.
     *
     */
    void write_version_metadata_();

    /**
     * @brief Get space needed to write message
     *
     */
    std::uint64_t get_message_size_(
            const Message& msg);

    /**
     * @brief Get space needed to write schema
     *
     */
    std::uint64_t get_schema_size_(
            const mcap::Schema& schema);

    /**
     * @brief Get space needed to write channel
     *
     */
    std::uint64_t get_channel_size_(
            const mcap::Channel& channel);

    /**
     * @brief Convert given \c filename to temporal format.
     *
     * @param [in] filename Filename to be converted.
     */
    static std::string tmp_filename_(
            const std::string& filename);

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

    //! Name of open MCAP file
    std::string mcap_filename_;

    //! Payload pool
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    //! Handler instance state
    McapHandlerStateCode state_;

    //! MCAP writer
    mcap::McapWriter mcap_writer_;

    //! Schemas map
    std::map<std::string, mcap::Schema> schemas_;

    //! Received types set
    std::set<std::string> received_types_;

    //! Channels map
    std::map<ddspipe::core::types::DdsTopic, mcap::Channel> channels_;

    //! Space available in disk
    std::uintmax_t space_available_when_open_;

    //! Samples buffer
    std::list<Message> samples_buffer_;

    //! Dynamic types
    DynamicTypesCollection dynamic_types_;

    //! MCAP file overhead
    /**
     * To reach this number, we use the following constants:
     *   - Header + Write Header = 18
     *   - Metadata + Write Metadata + Write MetadataIndex = 75 + 24 + 36
     *   - Write ChunkIndex = 73
     *   - Write Statistics = 55
     *   - Write DataEnd + Write SummaryOffSets = 13 + 26*6
     */
    static constexpr std::uint64_t MCAP_FILE_OVERHEAD{450};

    //! Additional overhead size for a MCAP message
    static constexpr std::uint64_t MCAP_MESSAGE_OVERHEAD{31 + 8 + 8}; // Write Message + TimeStamp + TimeOffSet

    //! Additional overhead size for a MCAP schema
    static constexpr std::uint64_t MCAP_SCHEMAS_OVERHEAD{23}; // Write Schemas

    //! Additional overhead size for a MCAP channel
    static constexpr std::uint64_t MCAP_CHANNEL_OVERHEAD{25 + 10 + 10}; // Write Channel + messageIndexOffsetsSize + channelMessageCountsSize

    //! Additional overhead size for a MCAP attachment
    static constexpr std::uint64_t MCAP_ATTACHMENT_OVERHEAD{58 + 70}; // Write Attachment + Write AttachmentIndex

    //! Dynamic types reserved storage
    std::uint64_t storage_dynamic_types_{0};

    //! Total file size
    std::uint64_t file_size_{MCAP_FILE_OVERHEAD}; // MCAP file size is initialized with MCAP_FILE_OVERHEAD

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
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
