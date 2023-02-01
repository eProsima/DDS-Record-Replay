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

#include <efficiency/payload/FastPayloadPool.hpp>
#include <types/topic/dds/DdsTopic.hpp>
#include <types/dds/Data.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {
namespace recorder {

/**
 * TODO
 */
class FoxgloveWsHandler
{
public:

    FoxgloveWsHandler(
            const char* file_name,
            std::shared_ptr<PayloadPool> payload_pool);

    ~FoxgloveWsHandler();

    void run_server();

    void add_schema(
            const std::string& schema_name,
            const std::string& schema_text);

    void add_data(
            const types::DdsTopic& topic,
            std::unique_ptr<types::DataReceived>& data);

protected:

    foxglove::websocket::ChannelId create_channel_id_nts_(
            const types::DdsTopic& topic);

    foxglove::websocket::ChannelId get_channel_id_(
            const types::DdsTopic& topic);

    std::string get_schema_text_(
            const std::string& schema_name);

    uint64_t fastdds_timestamp_to_nanoseconds_since_epoch(const types::DataTime& time);

    std::shared_ptr<PayloadPool> payload_pool_;

    // NOTE: it cannot be used with Atomicable because Server is a final class
    foxglove::websocket::Server server_{8765, "C++ Fast DDS example server"};
    std::mutex server_mtx_;

    using SchemaMapType = utils::SharedAtomicable<std::map<std::string, std::string>>;
    SchemaMapType schemas_;

    using ChannelMapType = utils::SharedAtomicable<std::map<std::string, foxglove::websocket::ChannelId>>;
    ChannelMapType channels_;

    std::atomic<unsigned int> unique_sequence_number_{0};

    static constexpr const char* MCAP_FILE = "output.mcap";

    std::thread server_thread_;
};

} /* namespace recorder */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
