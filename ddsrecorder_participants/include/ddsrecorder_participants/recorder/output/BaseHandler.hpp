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
 * @file BaseHandler.hpp
 */

#pragma once

#include <condition_variable>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <utility>

#include <fastdds/dds/xtypes/type_representation/detail/dds_xtypes_typeobject.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

#include <cpp_utils/macros/custom_enumeration.hpp>

#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>

#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>
#include <ddspipe_participants/participant/dynamic_types/ISchemaHandler.hpp>

#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollection.hpp>
#include <ddsrecorder_participants/library/library_dll.h>
#include <ddsrecorder_participants/recorder/message/BaseMessage.hpp>
#include <ddsrecorder_participants/recorder/output/BaseHandlerConfiguration.hpp>
#include <ddsrecorder_participants/recorder/output/BaseWriter.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

//! State of the handler instance
ENUMERATION_BUILDER(
    BaseHandlerStateCode,
    STOPPED,                  //! Received messages are not processed.
    RUNNING,                  //! Messages are stored in buffer and dumped to disk when full.
    PAUSED                    //! Messages are stored in buffer and dumped to disk when event triggered.
    );

/**
 * @brief Base class with generic methods to interact with the \c DdsPipe ( \c SchemaParticipant ).
 *
 * More concretely, the \c BaseHandler manages:
 * - state transitions (start, stop, pause, trigger_event),
 * - buffer management (storing, discarding and dumping samples),
 * - pending samples management (samples received before the type is known),
 * - dynamic types collection (storing and serializing types).
 *
 * Payloads are efficiently passed from DDS Pipe to the output file without copying data (only references).
 *
 * @implements ISchemaHandler
 */
class BaseHandler : public ddspipe::participants::ISchemaHandler
{
public:

    /**
     * @brief Creates the \c BaseHandler instance with the given configuration.
     *
     * @param config:       Structure encapsulating all configuration options.
     * @param payload_pool: Pool of payloads to be used by the handler.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    BaseHandler(
            const BaseHandlerConfiguration& config,
            const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool);

    /**
     * @brief Destructor the \c BaseHandler instance.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    virtual ~BaseHandler();

    /**
     * @brief Initialize handler instance
     *
     * @warning Command methods ( \c start , \c pause , \c stop , and \c trigger_event ) are not thread safe
     * among themselves. This is, they are expected to be executed sequentially and all in the same thread.
     *
     * @note This method should be called by a derived class constructor to ensure the purely virtual methods have been
     * implemented.
     *
     * @param [in] init_state Initial state of the handler instance.
     * @param [in] on_disk_full_lambda Lambda to be executed when disk is full.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void init(
        const BaseHandlerStateCode& init_state = BaseHandlerStateCode::RUNNING,
        const std::function<void()>& on_disk_full_lambda = nullptr);

    /**
     * @brief Enable handler instance
     *
     * Enables the writer.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    virtual void enable();

    /**
     * @brief Disable handler instance
     *
     * Disables the writer.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    virtual void disable();

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

protected:

    //! Flag code controlling the event thread routine
    enum class EventCode
    {
        untriggered = 0,        //! Waiting for an event to be received.
        triggered,              //! Indicates that an event has been triggered.
        stopped,                //! Signals event thread to exit.
    };

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

    /**
     * @brief Stop event thread, and clear \c samples_buffer_ and \c pending_samples_paused_ structures
     *
     * A (locked) lock wrapping \c event_cv_mutex_ is passed so it can be released just before joining the thread.
     *
     * @param [in] event_lock Lock in locked state wrapping \c event_cv_mutex_
     */
    void stop_event_thread_nts_(
            std::unique_lock<std::mutex>& event_lock);

    /**
     * @brief Processes a received sample.
     *
     * The method is called when a new sample is received.
     * It either writes the sample directly to disk, adds it to \c samples_buffer or to a pending list, or discards it.
     *
     * @param [in] sample Sample to be processed.
     */
    void process_new_sample_nts_(
            std::shared_ptr<const BaseMessage> sample);

    /**
     * @brief Adds a sample to \c samples_buffer.
     *
     * @param sample Sample to be added.
     */
    void add_sample_to_buffer_nts_(
            std::shared_ptr<const BaseMessage> sample);

    /**
     * @brief Adds samples to \c samples_buffer.
     *
     * For each sample in \c samples, it writes it to disk and removes it from \c samples.
     * The method ends when \c samples is empty.
     *
     * @param [in] samples List of samples to be added.
     */
    void add_samples_to_buffer_nts_(
            std::list<std::shared_ptr<const BaseMessage>>& samples);

    /**
     * @brief Adds a sample to \c pending_samples_.
     *
     * If \c pending_samples_ is full, it adds the oldest sample to the buffer until there space for \c sample.
     *
     * @param sample Sample to be added.
     */
    void add_sample_to_pending_nts_(
            std::shared_ptr<const BaseMessage> sample);

    /**
     * @brief Dumps the pending samples corresponding to \c type_name.
     *
     * The function is called when the handler discovers a new type.
     *
     * If in PAUSED state, samples in \c pending_samples_paused_ structure for \c type_name are moved to the buffer,
     * so they will be written later on if an event is triggered.
     *
     * Samples in \c pending_samples_ structure for \c type_name are to be written irrespectively of the current state.
     * However, in RUNNING/STOPPED states these are moved to buffer (to be written together with the next batch),
     * while in PAUSED state they are directly written to the file (to avoid being deleted by event thread).
     * Note that in the last case, pending samples correspond to messages that were previously received in RUNNING
     * state, and hence should be stored regardless of whether or not an event is triggered.
     *
     * @param type_name Type name of the samples to be dumped.
     */
    void dump_pending_samples_nts_(
            const std::string& type_name);

    /**
     * @brief Writes \c samples to disk.
     *
     * For each sample in \c samples, it writes it to disk and removes it from \c samples.
     * The method ends when \c samples is empty.
     *
     * @param [in] samples List of samples to be written.
     */
    virtual void write_samples_(
            std::list<std::shared_ptr<const BaseMessage>>& samples) = 0;

    /**
     * @brief Remove samples older than [now - event_window].
     *
     * This method removes samples older than [now - event_window] from:
     * - \c samples_buffer_
     * - \c pending_samples_
     * - \c pending_samples_paused_
     */
    void remove_outdated_samples_nts_();

    /**
     * @brief Store a \c DynamicType and its dependencies in \c dynamic_types_.
     *
     * It calls \c store_dynamic_type_ with the type identifier and type object of each dependency of \c dynamic_type.
     * It calls \c store_dynamic_type_ with the type identifier and type object of \c type_name.
     *
     * @param [in] type_name Name of the type to be stored, used as key in \c dynamic_types map.
     * @param [in] type_identifier Type identifier to be serialized and stored.
     */
    void store_dynamic_type_(
            const std::string& type_name,
            const fastdds::dds::xtypes::TypeIdentifier& type_identifier);

    /**
     * @brief Create a \c DynamicType and insert it into \c dynamic_types_.
     *
     * @param [in] type_name Name of the type to be stored, used as key in \c dynamic_types map.
     * @param [in] type_identifier Type identifier to be serialized and stored.
     * @param [in] type_object Type object to be serialized and stored.
     */
    void store_dynamic_type_(
            const std::string& type_name,
            const fastdds::dds::xtypes::TypeIdentifier& type_identifier,
            const fastdds::dds::xtypes::TypeObject& type_object);

    //! Handler configuration
    const BaseHandlerConfiguration configuration_;

    //! Payload pool
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    //! Handler instance state
    BaseHandlerStateCode state_;

    //! Mutex synchronizing state transitions and access to object's data structures
    std::mutex mtx_;

    //////////////////////
    // EVENT MANAGEMENT //
    //////////////////////

    //! Event thread
    std::thread event_thread_;

    //! Event flag
    EventCode event_flag_ = EventCode::stopped;

    //! Event condition variable
    std::condition_variable event_cv_;

    //! Event condition variable mutex
    std::mutex event_cv_mutex_;

    ///////////////////////
    // BUFFER MANAGEMENT //
    ///////////////////////

    //! MCAP writer
    BaseWriter* writer_;

    //! Samples buffer
    std::list<std::shared_ptr<const BaseMessage>> samples_buffer_;

    //! Structure where messages (received in RUNNING state) with unknown type are kept
    std::map<std::string, std::list<std::shared_ptr<const BaseMessage>>> pending_samples_;

    //! Structure where messages (received in PAUSED state) with unknown type are kept
    std::map<std::string, std::list<std::shared_ptr<const BaseMessage>>> pending_samples_paused_;

    //////////////////////////////
    // DYNAMIC TYPES COLLECTION //
    //////////////////////////////

    //! Received types set
    std::map<std::string, fastdds::dds::DynamicType::_ref_type> received_types_;

    //! Dynamic types collection
    DynamicTypesCollection dynamic_types_;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
