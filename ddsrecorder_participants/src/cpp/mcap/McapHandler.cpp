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

#include <cstdio>

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/exception/InconsistencyException.hpp>

#include <fastrtps/types/DynamicType.h>

#include <ddspipe_core/types/dynamic_types/schema.hpp>

#include <ddsrecorder_participants/mcap/McapHandler.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

using namespace eprosima::ddspipe::core::types;

Message::Message(
        const Message& msg)
    : mcap::Message(msg)
{
    this->payload_owner = msg.payload_owner;
    auto payload_owner_ =
            const_cast<eprosima::fastrtps::rtps::IPayloadPool*>((eprosima::fastrtps::rtps::IPayloadPool*)msg.
                    payload_owner);
    this->payload_owner->get_payload(
        msg.payload,
        payload_owner_,
        this->payload);
}

Message::~Message()
{
    // If payload owner exists and payload has size, release it correctly in pool
    if (payload_owner && payload.length > 0)
    {
        payload_owner->release_payload(payload);
    }
}

McapHandler::McapHandler(
        const McapHandlerConfiguration& config,
        const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
        const McapHandlerStateCode& init_state /* = McapHandlerStateCode::RUNNING */)
    : configuration_(config)
    , payload_pool_(payload_pool)
    , state_(McapHandlerStateCode::STOPPED)
{
    std::string tmp_filename = tmp_filename_(config.file_name);
    auto status = mcap_writer_.open(tmp_filename.c_str(), mcap::McapWriterOptions("ros2"));
    if (!status.ok())
    {
        throw utils::InitializationException(
                  STR_ENTRY << "Failed to open MCAP file " << tmp_filename << " for writing: " << status.message);
    }

    logInfo(DDSRECORDER_MCAP_HANDLER,
            "MCAP file <" << config.file_name << "> .");

    if (init_state == McapHandlerStateCode::RUNNING)
    {
        start();
    }
    else if (init_state == McapHandlerStateCode::PAUSED)
    {
        pause();
    }
}

McapHandler::~McapHandler()
{
    logInfo(DDSRECORDER_MCAP_HANDLER, "Destroying handler.");

    // Stop handler prior to destruction
    stop();

    // Close writer and output file
    mcap_writer_.close();

    // Rename temp file to configuration file_name
    std::string tmp_filename = tmp_filename_(configuration_.file_name);
    if (std::rename(tmp_filename.c_str(), configuration_.file_name.c_str()))
    {
        logError(
            DDSRECORDER_MCAP_HANDLER,
            "Failed to rename " << tmp_filename << " into " << configuration_.file_name << " on handler destruction.");
    }
}

void McapHandler::add_schema(
        const fastrtps::types::DynamicType_ptr& dynamic_type)
{
    std::lock_guard<std::mutex> lock(mtx_);

    if (state_ == McapHandlerStateCode::STOPPED)
    {
        logWarning(
            DDSRECORDER_MCAP_HANDLER,
            "Attempting to add schema through a stopped handler, dropping...");
        return;
    }

    assert(nullptr != dynamic_type);
    std::string type_name = dynamic_type->get_name();
    {
        // Check if it exists already
        if (schemas_.find(type_name) != schemas_.end())
        {
            return;
        }

        // Schema not found, generate from dynamic type and store
        std::string schema_text = generate_ros2_schema(dynamic_type);

        logInfo(DDSRECORDER_MCAP_HANDLER, "\nAdding schema with name " << type_name << " :\n" << schema_text << "\n");

        // Create schema and add it to writer and to schemas map
        mcap::Schema new_schema(type_name, "ros2msg", schema_text);
        // WARNING: passing as non-const to MCAP library
        mcap_writer_.addSchema(new_schema);
        schemas_.insert({type_name, std::move(new_schema)});
    }

    logInfo(DDSRECORDER_MCAP_HANDLER, "Schema created: " << type_name << ".");

    // Check if there are any pending samples for this new schema. If so, add them to buffer.
    auto it = pending_samples_.find(type_name);
    if (it != pending_samples_.end())
    {
        add_pending_samples_nts_(type_name);
        pending_samples_.erase(it);
    }
}

void McapHandler::add_data(
        const DdsTopic& topic,
        RtpsPayloadData& data)
{
    std::lock_guard<std::mutex> lock(mtx_);

    logInfo(
        DDSRECORDER_MCAP_HANDLER,
        "Adding data in topic " << topic);

    if (state_ == McapHandlerStateCode::STOPPED)
    {
        logWarning(
            DDSRECORDER_MCAP_HANDLER,
            "Attempting to add sample through a stopped handler, dropping...");
        return;
    }

    // Add data to channel
    Message msg;
    msg.sequence = unique_sequence_number_++;
    msg.publishTime = fastdds_timestamp_to_mcap_timestamp(data.source_timestamp);
    if (configuration_.log_publishTime)
    {
        msg.logTime = msg.publishTime;
    }
    else
    {
        msg.logTime = now();
    }
    msg.dataSize = data.payload.length;

    if (data.payload.length > 0)
    {
        auto payload_owner =
                const_cast<eprosima::fastrtps::rtps::IPayloadPool*>((eprosima::fastrtps::rtps::IPayloadPool*)data.
                        payload_owner);

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
            throw utils::InconsistencyException(
                      STR_ENTRY << "Payload owner not found in data received."
                      );
        }
    }
    else
    {
        throw utils::InconsistencyException(
                  STR_ENTRY << "Received sample with no payload."
                  );
    }

    try
    {
        auto channel_id = get_channel_id_nts_(topic);
        msg.channelId = channel_id;
    }
    catch (const utils::Exception&)
    {
        logWarning(
            DDSRECORDER_MCAP_HANDLER,
            "Schema for topic " << topic << " not yet available, inserting to pending samples queue.");

        if (pending_samples_[topic.type_name].size() == configuration_.max_pending_samples)
        {
            pending_samples_[topic.type_name].pop();
        }
        pending_samples_[topic.type_name].push({topic.m_topic_name, msg});

        throw;
    }

    try
    {
        add_data_nts_(msg);
    }
    catch (const utils::Exception&)
    {
        throw utils::InconsistencyException(
                  STR_ENTRY << "Error writting in MCAP a message in topic " << topic.m_topic_name
                  );
    }
}

void McapHandler::start()
{
    std::lock_guard<std::mutex> lock(state_mtx_);

    // Store previous state to act differently depending on its value
    McapHandlerStateCode prev_state = state_;
    state_ = McapHandlerStateCode::RUNNING;

    if (prev_state == McapHandlerStateCode::RUNNING)
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

        if (prev_state == McapHandlerStateCode::PAUSED)
        {
            // Stop event routine (cleans buffer)
            stop_event_thread_nts_();
        }
    }
}

void McapHandler::stop()
{
    std::lock_guard<std::mutex> lock(state_mtx_);

    // Store previous state to act differently depending on its value
    McapHandlerStateCode prev_state = state_;
    state_ = McapHandlerStateCode::STOPPED;

    if (prev_state == McapHandlerStateCode::RUNNING)
    {
        std::lock_guard<std::mutex> _(mtx_);
        dump_data_nts_();
    }
    else if (prev_state == McapHandlerStateCode::PAUSED)
    {
        // Stop event routine (cleans buffer)
        stop_event_thread_nts_();
    }
}

void McapHandler::pause()
{
    std::lock_guard<std::mutex> lock(state_mtx_);

    // Store previous state to act differently depending on its value
    McapHandlerStateCode prev_state = state_;
    state_ = McapHandlerStateCode::PAUSED;

    if (prev_state == McapHandlerStateCode::PAUSED)
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

        if (prev_state == McapHandlerStateCode::RUNNING)
        {
            std::lock_guard<std::mutex> _(mtx_);
            // Write data stored in buffer
            dump_data_nts_();
            // Remove pending samples
            clear_all_nts_();
        }

        // Launch event thread routine
        event_flag_ = EventCode::untriggered;  // No need to take event mutex (protected by state_mtx_)
        event_thread_ = std::thread(&McapHandler::event_thread_routine_, this);
    }
}

void McapHandler::trigger_event()
{
    std::lock_guard<std::mutex> lock(state_mtx_);

    if (state_ != McapHandlerStateCode::PAUSED)
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
            event_flag_ = EventCode::triggered;
        }
        event_cv_.notify_one();
    }
}

mcap::Timestamp McapHandler::fastdds_timestamp_to_mcap_timestamp(
        const DataTime& time)
{
    uint64_t mcap_time = time.seconds();
    mcap_time *= 1000000000;
    return mcap_time + time.nanosec();
}

mcap::Timestamp McapHandler::std_timepoint_to_mcap_timestamp(
        const utils::Timestamp& time)
{
    return mcap::Timestamp(std::chrono::duration_cast<std::chrono::nanoseconds>(time.time_since_epoch()).count());
}

mcap::Timestamp McapHandler::now()
{
    return std_timepoint_to_mcap_timestamp(utils::now());
}

void McapHandler::add_data_nts_(
        const Message& msg)
{
    samples_buffer_.push_back(msg);
    if (state_ == McapHandlerStateCode::RUNNING && samples_buffer_.size() == configuration_.buffer_size)
    {
        logInfo(DDSRECORDER_MCAP_HANDLER, "Full buffer, writting to disk...");
        dump_data_nts_();
    }
}

void McapHandler::add_pending_samples_nts_(
        const std::string& schema_name)
{
    assert(pending_samples_.find(schema_name) != pending_samples_.end());
    auto& pending_queue = pending_samples_[schema_name];

    logInfo(DDSRECORDER_MCAP_HANDLER, "Sending pending samples of type: " << schema_name << ".");

    DdsTopic sample_topic;
    mcap::ChannelId channel_id;
    while (!pending_queue.empty())
    {
        auto& sample = pending_queue.front();
        sample_topic.m_topic_name = sample.first;
        sample_topic.type_name = schema_name;
        channel_id = get_channel_id_nts_(sample_topic);
        auto& msg = sample.second;
        msg.channelId = channel_id;
        try
        {
            add_data_nts_(msg);
        }
        catch (const utils::Exception&)
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
    while (true)
    {
        bool timeout;
        {
            auto exit_time = std::chrono::time_point<std::chrono::system_clock>::max();
            auto cleanup_period_ = std::chrono::seconds(configuration_.cleanup_period);
            if (cleanup_period_ < std::chrono::seconds::max())
            {
                auto now = std::chrono::system_clock::now();
                exit_time = now + cleanup_period_;
            }

            std::unique_lock<std::mutex> lock(event_cv_mutex_);

            if (event_flag_ != EventCode::untriggered)
            {
                // Flag set before taking mutex, no need to wait
                timeout = false;
            }
            else
            {
                timeout = !event_cv_.wait_until(
                    lock,
                    exit_time,
                    [&]
                    {
                        return event_flag_ != EventCode::untriggered;
                    });
            }

            if (event_flag_ == EventCode::stopped)
            {
                logInfo(DDSRECORDER_MCAP_HANDLER, "Finishing event thread routine.");
                return;
            }
            else
            {
                // Reset and wait for next event
                event_flag_ = EventCode::untriggered;
            }
        }

        std::lock_guard<std::mutex> lock(mtx_);

        // Delete outdated samples if timeout, and also before dumping (event triggered case)
        remove_outdated_samples_nts_();

        if (timeout)
        {
            logInfo(DDSRECORDER_MCAP_HANDLER, "Event thread timeout.");
        }
        else
        {
            logInfo(DDSRECORDER_MCAP_HANDLER, "Event triggered: dumping buffered data.");
            dump_data_nts_();
        }
    }
}

void McapHandler::remove_outdated_samples_nts_()
{
    logInfo(DDSRECORDER_MCAP_HANDLER, "Removing outdated samples.");

    auto threshold = std_timepoint_to_mcap_timestamp(utils::now() - std::chrono::seconds(configuration_.event_window));
    samples_buffer_.remove_if([&](auto& sample)
            {
                return sample.logTime < threshold;
            });
}

void McapHandler::stop_event_thread_nts_()
{
    // WARNING: state must have been set different to PAUSED before calling this method
    assert(state_ != McapHandlerStateCode::PAUSED);

    logInfo(DDSRECORDER_MCAP_HANDLER, "Stopping event thread.");
    if (event_thread_.joinable())
    {
        {
            std::lock_guard<std::mutex> lock(event_cv_mutex_);
            event_flag_ = EventCode::stopped;
        }
        event_cv_.notify_one();
        event_thread_.join();
    }
    std::lock_guard<std::mutex> _(mtx_);
    clear_all_nts_();
}

void McapHandler::clear_all_nts_()
{
    logInfo(DDSRECORDER_MCAP_HANDLER, "Cleaning all buffers.");

    // Clear samples buffer
    samples_buffer_.clear();

    // Clear pending samples
    pending_samples_.clear();
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
        samples_buffer_.pop_front();
    }
}

mcap::ChannelId McapHandler::create_channel_id_nts_(
        const DdsTopic& topic)
{
    // Find schema
    auto schema_id = get_schema_id_nts_(topic.type_name);

    // Create new channel
    mcap::Channel new_channel(topic.m_topic_name, "cdr", schema_id);
    mcap_writer_.addChannel(new_channel);
    auto channel_id = new_channel.id;
    channels_.insert({topic.m_topic_name, std::move(new_channel)});
    logInfo(DDSRECORDER_MCAP_HANDLER, "Channel created: " << topic << ".");

    return channel_id;
}

mcap::ChannelId McapHandler::get_channel_id_nts_(
        const DdsTopic& topic)
{
    auto it = channels_.find(topic.m_topic_name);
    if (it != channels_.end())
    {
        return it->second.id;
    }

    // If it does not exist yet, create it (call it with mutex taken)
    return create_channel_id_nts_(topic);
}

mcap::SchemaId McapHandler::get_schema_id_nts_(
        const std::string& schema_name)
{
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

std::string McapHandler::tmp_filename_(
        const std::string& filename)
{
    static const std::string TMP_SUFFIX = ".tmp~";
    return filename + TMP_SUFFIX;
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
