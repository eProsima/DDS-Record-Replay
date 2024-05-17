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
 * @file SqlHandler.hpp
 */

#pragma once

#include <functional>
#include <list>
#include <memory>
#include <set>

#include <fastrtps/types/DynamicType.h>

#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>
#include <ddspipe_core/types/data/RtpsPayloadData.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddsrecorder_participants/library/library_dll.h>
#include <ddsrecorder_participants/recorder/message/BaseMessage.hpp>
#include <ddsrecorder_participants/recorder/output/BaseHandler.hpp>
#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>
#include <ddsrecorder_participants/recorder/sql/SqlHandlerConfiguration.hpp>
#include <ddsrecorder_participants/recorder/sql/SqlWriter.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Class that manages the interaction between DDS Pipe (\c SchemaParticipant) and MCAP files through sql library.
 * Payloads are efficiently passed from DDS Pipe to sql without copying data (only references).
 *
 * @implements BaseHandler
 */
class SqlHandler : public BaseHandler
{
public:

    /**
     * SqlHandler constructor by required values.
     *
     * Creates SqlHandler instance with given configuration, payload pool and initial state.
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
    SqlHandler(
            const SqlHandlerConfiguration& config,
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
    virtual ~SqlHandler();

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
     * @brief Add a data sample to the given \c topic.
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
     * @param [in] data SqlMessage to be added.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void add_data(
            const ddspipe::core::types::DdsTopic& topic,
            ddspipe::core::types::RtpsPayloadData& data) override;

protected:

    /**
     * @brief Writes \c samples to disk.
     *
     * For each sample in \c samples, it downcasts it to \c SqlMessage, writes it to disk, and removes it from
     * \c samples.
     * The method ends when \c samples is empty.
     *
     * @param [in] samples List of samples to be written.
     */
    void write_samples_(
            std::list<std::shared_ptr<const BaseMessage>>& samples) override;

    //! SQL writer
    SqlWriter sql_writer_;

    //! Topics that the SQL writer has written
    std::set<ddspipe::core::types::DdsTopic> written_topics_;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
