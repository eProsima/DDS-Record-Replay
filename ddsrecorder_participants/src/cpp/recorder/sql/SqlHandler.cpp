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
 * @file SqlHandler.cpp
 */

#define SQL_IMPLEMENTATION  // Define this in exactly one .cpp file

#include <ddsrecorder_participants/recorder/message/SqlMessage.hpp>
#include <ddsrecorder_participants/recorder/output/Serializer.hpp>
#include <ddsrecorder_participants/recorder/sql/SqlHandler.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

using namespace eprosima::ddspipe::core::types;

SqlHandler::SqlHandler(
        const SqlHandlerConfiguration& config,
        const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
        std::shared_ptr<ddsrecorder::participants::FileTracker> file_tracker,
        const BaseHandlerStateCode& init_state /* = BaseHandlerStateCode::RUNNING */,
        const std::function<void()>& on_disk_full_lambda /* = nullptr */)
    : BaseHandler(config, payload_pool)
    , sql_writer_(config.output_settings, file_tracker, config.record_types)
{
    logInfo(DDSRECORDER_SQL_HANDLER, "Creating SQL handler instance.");

    // Set the BaseHandler's writer
    writer_ = &sql_writer_;

    // Initialize the BaseHandler
    init(init_state, on_disk_full_lambda);
}

SqlHandler::~SqlHandler()
{
    logInfo(DDSRECORDER_SQL_HANDLER, "Destroying SQL handler.");

    // Stop handler prior to destruction
    stop(true);
}

void SqlHandler::add_schema(
        const fastrtps::types::DynamicType_ptr& dynamic_type)
{
    // NOTE: Process schemas even if in STOPPED state to avoid losing them (only sent/received once in discovery)
    std::lock_guard<std::mutex> lock(mtx_);

    if (dynamic_type == nullptr)
    {
        logWarning(DDSRECORDER_SQL_HANDLER, "Received nullptr dynamic type. Skipping...");
        return;
    }

    const auto type_name = dynamic_type->get_name();

    // Check if it exists already
    if (received_types_.find(type_name) != received_types_.end())
    {
        return;
    }

    // Add type to the list of received types
    received_types_[type_name] = dynamic_type;

    // Add type to the collection of dynamic types
    store_dynamic_type_(type_name);

    if (configuration_.record_types)
    {
        const auto dynamic_type = dynamic_types_.dynamic_types().back();
        sql_writer_.write(dynamic_type);
    }

    // Check if there are any pending samples for this new type. If so, dump them.
    if (pending_samples_.find(type_name) != pending_samples_.end() ||
            (state_ == BaseHandlerStateCode::PAUSED &&
            pending_samples_paused_.find(type_name) != pending_samples_paused_.end()))
    {
        dump_pending_samples_nts_(type_name);
    }
}

void SqlHandler::add_data(
        const DdsTopic& topic,
        RtpsPayloadData& data)
{
    std::unique_lock<std::mutex> lock(mtx_);

    process_new_sample_nts_(std::make_shared<const SqlMessage>(
            data, payload_pool_, topic, configuration_.log_publishTime));
}

void SqlHandler::write_samples_(
        std::list<std::shared_ptr<const BaseMessage>>& samples)
{
    logInfo(DDSRECORDER_SQL_HANDLER, "Writing samples to SQL file.");

    while (!samples.empty())
    {
        const auto sql_sample = static_cast<const SqlMessage*>(samples.front().get());

        if (sql_sample == nullptr)
        {
            logWarning(DDSRECORDER_SQL_HANDLER, "Error downcasting sample to SqlMessage. Skipping...");
            continue;
        }

        // Write the topic if it hasn't been written before
        const auto topic = sql_sample->topic;

        if (written_topics_.find(topic) == written_topics_.end())
        {
            sql_writer_.write(topic);
            written_topics_.insert(topic);
        }

        // Write the sample
        if (sql_sample->key.empty())
        {
            set_key_(*const_cast<SqlMessage*>(sql_sample));
        }

        sql_writer_.write(*sql_sample);

        samples.pop_front();
    }
}

void SqlHandler::set_key_(
        SqlMessage& sql_sample)
{
    if (keys_.find(sql_sample.instance_handle) != keys_.end())
    {
        // The key has already been calculated
        sql_sample.key = keys_[sql_sample.instance_handle];
        return;
    }

    if (received_types_.find(sql_sample.topic.type_name) == received_types_.end())
    {
        // The type is not known. The key can't be calculated
        return;
    }

    // Calculate the key
    sql_sample.set_key(received_types_[sql_sample.topic.type_name]);

    // Store the key
    keys_[sql_sample.instance_handle] = sql_sample.key;
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
