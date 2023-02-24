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
 * @file McapHandler.cpp
 */

#define MCAP_IMPLEMENTATION  // Define this in exactly one .cpp file

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/exception/InconsistencyException.hpp>

#include <fastrtps/types/DynamicType.h>

#include <ddspipe_core/types/dynamic_types/schema.hpp>

#include <ddsrecorder_participants/mcap/McapHandler.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

using namespace eprosima::ddspipe::core::types;

McapHandler::McapHandler(
        const char* file_name,
        std::shared_ptr<ddspipe::core::PayloadPool> payload_pool,
        unsigned int max_pending_samples,
        unsigned int buffer_size,
        unsigned int downsampling,
        unsigned int event_window)
        // bool autostart /* = false */)
    : payload_pool_(payload_pool)
    , max_pending_samples_(max_pending_samples)
    , buffer_size_(buffer_size)
    , downsampling_(downsampling)
    , event_window_(event_window)
{
    auto status = mcap_writer_.open(file_name, mcap::McapWriterOptions("ros2"));
    if (!status.ok()) {
        throw utils::InitializationException(
            STR_ENTRY << "Failed to open MCAP file " << file_name << " for writing: " << status.message);
    }

    logInfo(DDSRECORDER_MCAP_HANDLER,
        "MCAP file <" << file_name << "> .");
}

McapHandler::~McapHandler()
{
    logInfo(DDSRECORDER_MCAP_HANDLER, "Destroying handler.");

    if (paused_)
    {
        stop_event_thread_();
    }
    else
    {
        dump_data_();
    }
    mcap_writer_.close();
}

void McapHandler::add_schema(const fastrtps::types::DynamicType_ptr& dynamic_type)
{
    assert(nullptr != dynamic_type);
    std::string type_name = dynamic_type->get_name();
    {
        // Check if it exists already
        // NOTE: must be unique mutex taken because it could write afterwards
        std::unique_lock<SchemaMapType> lock(schemas_);
        if (schemas_.find(type_name) != schemas_.end())
        {
            return;
        }

        // Schema not found, generate from dynamic type and store
        std::string schema_text = generate_ros2_schema(dynamic_type);

        // TODO remove
        logInfo(DDSRECORDER_MCAP_HANDLER, "\nAdding schema with name " << type_name << " :\n" << schema_text << "\n");

        // Create schema and add it to writer and to schemas map
        mcap::Schema new_schema(type_name, "ros2msg", schema_text);

        {
            std::lock_guard<std::mutex> guard(write_mtx_);
            mcap_writer_.addSchema(new_schema);
        }

        schemas_.insert({type_name, std::move(new_schema)});
    }

    logInfo(DDSRECORDER_MCAP_HANDLER, "Schema created: " << type_name << ".");

    std::lock_guard<PendingSamplesMapType> lock(pending_samples_);

    auto it = pending_samples_.find(type_name);
    if (it != pending_samples_.end())
    {
        add_pending_samples_(type_name);
        pending_samples_.erase(it);
    }
}

void McapHandler::add_data(
        const DdsTopic& topic,
        RtpsPayloadData& data)
{
    // Check if channel exists
    // NOTE: must be unique mutex taken because it could write afterwards

    // Add data to channel
    Message msg;
    msg.sequence = unique_sequence_number_++;
    msg.logTime = now();
    msg.publishTime = fastdds_timestamp_to_mcap_timestamp(data.source_timestamp);
    msg.dataSize = data.payload.length;

    if (data.payload.length > 0)
    {
        auto payload_owner =
            const_cast<eprosima::fastrtps::rtps::IPayloadPool*>((eprosima::fastrtps::rtps::IPayloadPool*)data.payload_owner);

        if (payload_owner)
        {
            payload_pool_->get_payload(
                data.payload,
                payload_owner,
                msg.payload);

            msg.payload_owner = payload_pool_.get();
            msg.data = reinterpret_cast<std::byte*>(msg.payload.data);
        }
        else
        {
            logWarning(
                DDSRECORDER_MCAP_HANDLER,
                "Payload owner not found in data received.");
        }
    }
    else
    {
        logWarning(
            DDSRECORDER_MCAP_HANDLER,
            "Received sample with no payload.");
    }

    try
    {
        auto channel_id = get_channel_id_(topic);
        msg.channelId = channel_id;
    }
    catch(const utils::Exception& e)
    {
        logWarning(
            DDSRECORDER_MCAP_HANDLER,
            "Schema for topic " << topic << " not yet available, inserting to pending samples queue.");

        std::lock_guard<PendingSamplesMapType> lock(pending_samples_);

        if (pending_samples_[topic.type_name].size() == max_pending_samples_)
        {
            pending_samples_[topic.type_name].pop();
        }
        pending_samples_[topic.type_name].push({topic.m_topic_name, msg});

        throw;
    }

    try
    {
        add_data_(msg);
    }
    catch(const utils::Exception& e)
    {
        throw utils::InconsistencyException(
            STR_ENTRY << "Error writting in MCAP a message in topic " << topic.m_topic_name
        );
    }
}

void McapHandler::start()
{
    if (!paused_)
    {
        logWarning(
            DDSRECORDER_MCAP_HANDLER,
            "Ignoring start command, instance already started.");
    }
    else
    {
        logInfo(
            DDSRECORDER_MCAP_HANDLER,
            "Starting handler.");

        paused_.store(false);
        stop_event_thread_();
    }
}

void McapHandler::pause()
{
    if (paused_)
    {
        logWarning(
            DDSRECORDER_MCAP_HANDLER,
            "Ignoring pause command, instance already paused.");
    }
    else
    {
        logInfo(
            DDSRECORDER_MCAP_HANDLER,
            "Pausing handler.");

        dump_data_();

        paused_.store(true);
        event_thread_ = std::thread(&McapHandler::event_thread_routine_, this);
    }
}

void McapHandler::trigger_event()
{
    if (!paused_)
    {
        logWarning(
            DDSRECORDER_MCAP_HANDLER,
            "Ignoring trigger event command, instance is not paused.");
    }
    else
    {
        logInfo(
            DDSRECORDER_MCAP_HANDLER,
            "Triggering event.");
        {
            std::lock_guard<std::mutex> lock(event_cv_mutex_);
            event_triggered_ = true;
        }
        event_cv_.notify_one();
    }
}

void McapHandler::add_data_(
        const Message& msg)
{
    std::lock_guard<std::mutex> guard(write_mtx_);

    samples_buffer_.push(msg);
    if (!paused_ && samples_buffer_.size() == buffer_size_)
    {
        logInfo(DDSRECORDER_MCAP_HANDLER, "Full buffer, writting to disk...");
        dump_data_nts_();
    }
}

void McapHandler::add_pending_samples_(
        const std::string& schema_name)
{
    assert(pending_samples_.find(schema_name) != pending_samples_.end());
    auto& pending_queue = pending_samples_[schema_name];

    logInfo(DDSRECORDER_MCAP_HANDLER, "Sending pending samples of type: " << schema_name << ".");

    while (!pending_queue.empty())
    {
        auto& sample = pending_queue.front();
        DdsTopic sample_topic;
        sample_topic.m_topic_name = sample.first;
        sample_topic.type_name = schema_name;
        auto channel_id = get_channel_id_(sample_topic);
        auto& msg = sample.second;
        msg.channelId = channel_id;
        try
        {
            add_data_(msg);
        }
        catch(const utils::Exception& e)
        {
            throw utils::InconsistencyException(
                STR_ENTRY << "Error writting in MCAP a message in topic " << sample.first
            );
        }
        pending_queue.pop();
    }
}

void McapHandler::event_thread_routine_()
{
    while (paused_)
    {
        clear_all_();

        bool timeout;
        {
            std::unique_lock<std::mutex> lock(event_cv_mutex_);

            timeout = !event_cv_.wait_for(
                lock,
                std::chrono::seconds(event_window_),
                [&]
                {
                    return event_triggered_;
                });
            event_triggered_ = false;
        }

        if (timeout)
        {
            logInfo(DDSRECORDER_MCAP_HANDLER, "Event thread timeout.");
        }
        else
        {
            if (paused_)
            {
                logInfo(DDSRECORDER_MCAP_HANDLER, "Event triggered: dumping buffered data.");
                dump_data_();

            }
            else
            {
                logInfo(DDSRECORDER_MCAP_HANDLER, "Finishing event thread routine.");
                clear_all_();
                break;
            }
        }
    }
}

void McapHandler::stop_event_thread_()
{
    logInfo(DDSRECORDER_MCAP_HANDLER, "Stopping event thread.");
    paused_.store(false);
    if (event_thread_.joinable())
    {
        {
            std::lock_guard<std::mutex> lock(event_cv_mutex_);
            event_triggered_ = true;
        }
        event_cv_.notify_one();
        event_thread_.join();
    }
}

void McapHandler::clear_all_()
{
    logInfo(DDSRECORDER_MCAP_HANDLER, "Cleaning all buffers.");

    {
        // Clear samples buffer
        std::lock_guard<std::mutex> guard(write_mtx_);
        std::queue<Message> empty;
        std::swap(samples_buffer_, empty);
    }

    {
        // Clear pending samples
        std::lock_guard<PendingSamplesMapType> lock(pending_samples_);
        pending_samples_.clear();
    }
}

void McapHandler::dump_data_()
{
    std::lock_guard<std::mutex> guard(write_mtx_);
    dump_data_nts_();
}

void McapHandler::dump_data_nts_()
{
    logInfo(DDSRECORDER_MCAP_HANDLER, "Writing data stored in buffer.");

    while (!samples_buffer_.empty())
    {
        auto& sample = samples_buffer_.front();
        auto status = mcap_writer_.write(sample);
        if (!status.ok())
        {
            throw utils::InconsistencyException(
                STR_ENTRY << "Error writting in MCAP"
            );
        }
        samples_buffer_.pop();
    }
}

mcap::ChannelId McapHandler::create_channel_id_nts_(
        const DdsTopic& topic)
{
    // Find schema
    auto schema_id = get_schema_id_(topic.type_name);

    // Create new channel
    mcap::Channel new_channel(topic.m_topic_name, "cdr", schema_id);
    mcap_writer_.addChannel(new_channel);
    auto channel_id = new_channel.id;
    channels_.insert({topic.m_topic_name, std::move(new_channel)});
    logInfo(DDSRECORDER_MCAP_HANDLER, "Channel created: " << topic << ".");

    return channel_id;
}

mcap::ChannelId McapHandler::get_channel_id_(
        const DdsTopic& topic)
{
    std::unique_lock<ChannelMapType> channels_lock(channels_);
    auto it = channels_.find(topic.m_topic_name);
    if (it != channels_.end())
    {
        return it->second.id;
    }

    // If it does not exist yet, create it (call it with mutex taken)
    return create_channel_id_nts_(topic);
}

mcap::SchemaId McapHandler::get_schema_id_(
        const std::string& schema_name)
{
    std::unique_lock<SchemaMapType> lock(schemas_);
    auto it = schemas_.find(schema_name);
    if (it != schemas_.end())
    {
        return it->second.id;
    }
    else
    {
        throw utils::InconsistencyException(
                STR_ENTRY << "Schema " << schema_name << " is not registered.");
    }
}

mcap::Timestamp McapHandler::now()
{
  return mcap::Timestamp(std::chrono::duration_cast<std::chrono::nanoseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count());
}

mcap::Timestamp McapHandler::fastdds_timestamp_to_mcap_timestamp(const DataTime& time)
{
    uint64_t mcap_time = time.seconds();
    mcap_time *= 1000000000;
    return mcap_time + time.nanosec();
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
