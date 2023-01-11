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

#include <mcap/writer.hpp>

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
class McapHandler
{
public:

    McapHandler(
            const char* file_name,
            std::shared_ptr<PayloadPool> payload_pool);

    ~McapHandler();

    void add_schema(
            const std::string& schema_name,
            const std::string& schema_text);

    void add_data(
            const types::DdsTopic& topic,
            std::unique_ptr<types::DataReceived>& data);

protected:

    mcap::ChannelId get_channel_id_(
            const types::DdsTopic& topic);

    mcap::SchemaId get_schema_id_(
            const std::string& schema_name);

    mcap::Timestamp now();
    mcap::Timestamp fastdds_timestamp_to_mcap_timestamp(const types::DataTime& time);

    std::shared_ptr<PayloadPool> payload_pool_;

    mcap::McapWriter mcap_writer_;
    // NOTE: it cannot be used with Atomicabl because McapWriter is a final class
    std::mutex write_mtx_;

    using SchemaMapType = utils::SharedAtomicable<std::map<std::string, mcap::Schema>>;
    SchemaMapType schemas_;

    using ChannelMapType = utils::SharedAtomicable<std::map<std::string, mcap::Channel>>;
    ChannelMapType channels_;

    std::atomic<unsigned int> unique_sequence_number_{0};

    static constexpr const char* MCAP_FILE = "output.mcap";
};

} /* namespace recorder */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
