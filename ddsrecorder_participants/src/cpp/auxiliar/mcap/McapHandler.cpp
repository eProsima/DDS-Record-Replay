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

#include <ddsrecorder_participants/auxiliar/mcap/McapHandler.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

using namespace eprosima::ddsrouter::core::types;

McapHandler::McapHandler(
        const char* file_name)
{
    auto status = mcap_writer_.open(file_name, mcap::McapWriterOptions("ros2"));
    if (!status.ok()) {
        throw utils::InitializationException(
            STR_ENTRY << "Failed to open MCAP file " << file_name << " for writing: " << status.message);
    }

    logInfo(DDSRECORDER_MCAP_HANDLER,
        "MCAP file <" << file_name << "> ."); // TODO(recorder) remove // TODO: change by logdebug? Or just leave it logInfo
}

McapHandler::~McapHandler()
{
    mcap_writer_.close();
}

void McapHandler::add_schema(const std::string& schema_name, const std::string& schema_text)
{
    // Check if it exists already
    // NOTE: must be unique mutex taken because it could write afterwards
    std::unique_lock<SchemaMapType> lock(schemas_);
    if (schemas_.find(schema_name) != schemas_.end())
    {
        return;
    }

    // TODO remove
    logInfo(DDSRECORDER_MCAP_HANDLER, "\nAdding schema with name " << schema_name << " :\n" << schema_text << "\n");

    // Create schema and add it to writer and to schemas map
    mcap::Schema new_schema(schema_name, "ros2msg", schema_text);

    {
        std::lock_guard<std::mutex> guard(write_mtx_);
        mcap_writer_.addSchema(new_schema);
    }

    schemas_.insert({schema_name, std::move(new_schema)});
    logInfo(DDSRECORDER_MCAP_HANDLER, "Schema created: " << schema_name << ".");
}

void McapHandler::add_data(
        const DdsTopic& topic,
        std::unique_ptr<DataReceived>& data)
{
    // Check if channel exists
    // NOTE: must be unique mutex taken because it could write afterwards

    auto channel_id = get_channel_id_(topic);

    // Add data to channel
    mcap::Message msg;
    msg.channelId = channel_id;
    msg.sequence = unique_sequence_number_++;
    msg.logTime = now();
    msg.publishTime = fastdds_timestamp_to_mcap_timestamp(data->properties.source_timestamp);
    msg.data = reinterpret_cast<std::byte*>(data->payload.data);
    msg.dataSize = data->payload.length;

    {
        std::lock_guard<std::mutex> guard(write_mtx_);
        auto status = mcap_writer_.write(msg);
        if (!status.ok())
        {
            throw utils::InconsistencyException(
                STR_ENTRY << "Error writting in MCAP a message in topic " << topic.topic_name
            );
        }
    }
}

mcap::ChannelId McapHandler::create_channel_id_nts_(
        const DdsTopic& topic)
{
    // Find schema
    auto schema_id = get_schema_id_(topic.type_name);

    // Create new channel
    mcap::Channel new_channel(topic.topic_name, "cdr", schema_id);
    mcap_writer_.addChannel(new_channel);
    auto channel_id = new_channel.id;
    channels_.insert({topic.topic_name, std::move(new_channel)});
    logInfo(DDSRECORDER_MCAP_HANDLER, "Channel created: " << topic << ".");

    return channel_id;
}

mcap::ChannelId McapHandler::get_channel_id_(
        const DdsTopic& topic)
{
    std::unique_lock<ChannelMapType> channels_lock(channels_);
    auto it = channels_.find(topic.topic_name);
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
