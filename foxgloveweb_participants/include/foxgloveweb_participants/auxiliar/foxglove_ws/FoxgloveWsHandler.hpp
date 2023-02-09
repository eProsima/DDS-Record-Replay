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
 * @file FoxgloveWsHandler.hpp
 */

#pragma once

#include <websocket/server.hpp>

#include <cpp_utils/types/Atomicable.hpp>

#include <ddsrouter_core/types/dds/Data.hpp>
#include <ddsrouter_core/types/topic/dds/DdsTopic.hpp>

#include <ddsrouter_core/participants/auxiliar/schema/ISchemaHandler.hpp>

namespace eprosima {
namespace foxgloveweb {
namespace participants {

/**
 * TODO
 */
class FoxgloveWsHandler : public ddsrouter::participants::ISchemaHandler
{
public:

    FoxgloveWsHandler();

    ~FoxgloveWsHandler();

    void add_schema(
            const std::string& schema_name,
            const std::string& schema_text) override;

    void add_data(
            const ddsrouter::core::types::DdsTopic& topic,
            std::unique_ptr<ddsrouter::core::types::DataReceived>& data) override;

protected:

    void run_server_();

    foxglove::websocket::ChannelId create_channel_id_nts_(
            const ddsrouter::core::types::DdsTopic& topic);

    foxglove::websocket::ChannelId get_channel_id_(
            const ddsrouter::core::types::DdsTopic& topic);

    std::string get_schema_text_(
            const std::string& schema_name);

    uint64_t fastdds_timestamp_to_nanoseconds_since_epoch(const ddsrouter::core::types::DataTime& time);

    // NOTE: it cannot be used with Atomicable because Server is a final class
    foxglove::websocket::Server server_{8765, "C++ Fast DDS example server"};
    std::mutex server_mtx_;

    using SchemaMapType = utils::SharedAtomicable<std::map<std::string, std::string>>;
    SchemaMapType schemas_;

    using ChannelMapType = utils::SharedAtomicable<std::map<std::string, foxglove::websocket::ChannelId>>;
    ChannelMapType channels_;

    std::thread server_thread_;
};

} /* namespace participants */
} /* namespace foxgloveweb */
} /* namespace eprosima */
