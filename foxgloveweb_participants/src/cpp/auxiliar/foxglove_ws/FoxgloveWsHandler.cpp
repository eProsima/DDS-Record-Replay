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
 * @file FoxgloveWsHandler.cpp
 */

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/exception/InconsistencyException.hpp>

#include <foxgloveweb_participants/auxiliar/foxglove_ws/FoxgloveWsHandler.hpp>

namespace eprosima {
namespace foxgloveweb {
namespace participants {

using namespace eprosima::ddsrouter::core::types;

FoxgloveWsHandler::FoxgloveWsHandler()
{
    server_thread_ = std::thread(&FoxgloveWsHandler::run_server_, this);

    logInfo(FOXGLOVEWEB_FOXGLOVE_WS_HANDLER, "Websocket Server running...");
}

FoxgloveWsHandler::~FoxgloveWsHandler()
{
    for(auto& channel : channels_)
    {
        server_.removeChannel(channel.second);
    }
    server_.stop();
    server_thread_.join();
}

void FoxgloveWsHandler::add_schema(const std::string& schema_name, const std::string& schema_text)
{
    // Check if it exists already
    // NOTE: must be unique mutex taken because it could write afterwards
    std::unique_lock<SchemaMapType> lock(schemas_);
    if (schemas_.find(schema_name) != schemas_.end())
    {
        return;
    }

    // TODO remove
    logInfo(FOXGLOVEWEB_FOXGLOVE_WS_HANDLER, "\nAdding schema with name " << schema_name << " :\n" << schema_text << "\n");

    schemas_.insert({schema_name, schema_text});
    logInfo(FOXGLOVEWEB_FOXGLOVE_WS_HANDLER, "Schema created: " << schema_name << ".");
}

void FoxgloveWsHandler::add_data(
        const DdsTopic& topic,
        std::unique_ptr<DataReceived>& data)
{
    // Check if channel exists
    // NOTE: must be unique mutex taken because it could write afterwards

    auto channel_id = get_channel_id_(topic);

    // Add data to channel
    {
        std::lock_guard<std::mutex> guard(server_mtx_);
        server_.sendMessage(
            channel_id,
            fastdds_timestamp_to_nanoseconds_since_epoch(data->properties.source_timestamp),
            std::string_view{reinterpret_cast<const char*>(data->payload.data),
                    data->payload.length});
    }
}

void FoxgloveWsHandler::run_server_()
{
    server_.run();
}

foxglove::websocket::ChannelId FoxgloveWsHandler::create_channel_id_nts_(
        const DdsTopic& topic)
{
    // Find schema
    auto schema_text = get_schema_text_(topic.type_name);

    // Create new channel
    foxglove::websocket::ChannelId channel_id = server_.addChannel({
        .topic = topic.topic_name,
        .encoding = "cdr",
        .schemaName = topic.type_name,
        .schema = schema_text,
    });
    channels_.insert({topic.topic_name, channel_id});
    logInfo(FOXGLOVEWEB_FOXGLOVE_WS_HANDLER, "Channel created: " << topic << ".");

    return channel_id;
}

foxglove::websocket::ChannelId FoxgloveWsHandler::get_channel_id_(
        const DdsTopic& topic)
{
    std::unique_lock<ChannelMapType> channels_lock(channels_);
    auto it = channels_.find(topic.topic_name);
    if (it != channels_.end())
    {
        return it->second;
    }

    // If it does not exist yet, create it (call it with mutex taken)
    return create_channel_id_nts_(topic);
}

std::string FoxgloveWsHandler::get_schema_text_(
        const std::string& schema_name)
{
    std::unique_lock<SchemaMapType> lock(schemas_);
    auto it = schemas_.find(schema_name);
    if (it != schemas_.end())
    {
        return it->second;
    }
    else
    {
        throw utils::InconsistencyException(
                STR_ENTRY << "Schema " << schema_name << " is not registered.");
    }
}

uint64_t FoxgloveWsHandler::fastdds_timestamp_to_nanoseconds_since_epoch(const DataTime& time)
{
    uint64_t nanoseconds_since_epoch = time.seconds();
    nanoseconds_since_epoch *= 1000000000;
    return nanoseconds_since_epoch + time.nanosec();
}

} /* namespace participants */
} /* namespace foxgloveweb */
} /* namespace eprosima */
