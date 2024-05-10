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

#include <functional>
#include <list>
#include <map>

#include <mcap/mcap.hpp>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>

#include <cpp_utils/macros/custom_enumeration.hpp>

#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>
#include <ddspipe_core/types/data/RtpsPayloadData.hpp>
#include <ddspipe_core/types/dds/Payload.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddsrecorder_participants/library/library_dll.h>
#include <ddsrecorder_participants/recorder/mcap/McapHandlerConfiguration.hpp>
#include <ddsrecorder_participants/recorder/mcap/McapWriter.hpp>
#include <ddsrecorder_participants/recorder/message/BaseMessage.hpp>
#include <ddsrecorder_participants/recorder/message/McapMessage.hpp>
#include <ddsrecorder_participants/recorder/output/BaseHandler.hpp>
#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>

#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollection.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Class that manages the interaction between DDS Pipe (\c SchemaParticipant) and MCAP files through mcap library.
 * Payloads are efficiently passed from DDS Pipe to mcap without copying data (only references).
 *
 * @implements BaseHandler
 */
class McapHandler : public BaseHandler
{
public:

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
     * @param file_tracker: File tracker to be used to create and manage MCAP files.
     * @param init_state:   Initial instance state (RUNNING/PAUSED/STOPPED).
     * @param on_disk_full_lambda: Lambda to be executed when the disk is full.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    McapHandler(
            const McapHandlerConfiguration& config,
            const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
            std::shared_ptr<ddsrecorder::participants::FileTracker> file_tracker,
            const BaseHandlerStateCode& init_state = BaseHandlerStateCode::RUNNING,
            const std::function<void()>& on_disk_full_lambda = nullptr);

    /**
     * @brief Destructor
     *
     * Closes temporal MCAP file, and renames it with filename given in configuration.
     * Before closing file, received dynamic types are serialized and stored as an attachment.
     *
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    virtual ~McapHandler();

    /**
     * @brief Enable handler instance
     *
     * Enables the \c mcap_writer_ instance.
     */
    void enable() override;

    /**
     * @brief Disable handler instance
     *
     * Disables the \c mcap_writer_ instance.
     */
    void disable() override;

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
            const fastdds::dds::DynamicType::_ref_type& dynamic_type,
            const std::string& type_name,
            const fastdds::dds::xtypes::TypeIdentifier& type_id) override;

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
     * @param [in] data McapMessage to be added.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void add_data(
            const ddspipe::core::types::DdsTopic& topic,
            ddspipe::core::types::RtpsPayloadData& data) override;

protected:

    /**
     * @brief Writes \c samples to disk.
     *
     * For each sample in \c samples, it downcasts it to \c McapMessage, writes it to disk, and removes it from
     * \c samples.
     * The method ends when \c samples is empty.
     *
     * @param [in] samples List of samples to be written.
     */
    void write_samples_(
            std::list<std::shared_ptr<const BaseMessage>>& samples) override;

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

    //! MCAP writer
    McapWriter mcap_writer_;

    //! Schemas map
    std::map<std::string, mcap::Schema> schemas_;

    //! Channels map
    std::map<ddspipe::core::types::DdsTopic, mcap::Channel> channels_;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
