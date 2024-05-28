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

#pragma once

#include <set>
#include <string>

#include <mcap/reader.hpp>

#include <cpp_utils/memory/Heritable.hpp>

#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>

#include <ddsrecorder_participants/library/library_dll.h>
#include <ddsrecorder_participants/replayer/BaseReaderParticipant.hpp>
#include <ddsrecorder_participants/replayer/BaseReaderParticipantConfiguration.hpp>

#if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v1/DynamicTypesCollection.hpp>
#else
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v2/DynamicTypesCollection.hpp>
#endif // if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Participant that reads MCAP files and passes its messages to other DDS Pipe participants.
 *
 * @implements BaseReaderParticipant
 */
class McapReaderParticipant : public BaseReaderParticipant
{
public:

    /**
     * McapReaderParticipant constructor by required values.
     *
     * Creates McapReaderParticipant instance with given configuration, payload pool and input file path.
     *
     * @param config:       Structure encapsulating all configuration options.
     * @param payload_pool: Owner of every payload contained in sent messages.
     * @param file_path:    Path to the MCAP file with the messages to be read and sent.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    McapReaderParticipant(
            const std::shared_ptr<BaseReaderParticipantConfiguration>& configuration,
            const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
            const std::string& file_path);

    /**
     * @brief Process the MCAP file summary.
     *
     * Fills the topics with the MCAP file's channels and schemas.
     * Fills the types with the MCAP file's attachment.
     *
     * @param topics: Set of topics to be filled with the information from the MCAP file.
     * @param types:  DynamicTypesCollection instance to be filled with the types' information from the MCAP file.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void process_summary(
        std::set<utils::Heritable<ddspipe::core::types::DdsTopic>>& topics,
        DynamicTypesCollection& types) override;

    /**
     * @brief Read and send messages sequentially (according to timestamp).
     *
     * Reads the MCAP file messages and sends them to the participants.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void process_messages() override;

protected:

    /**
     * @brief Open the MCAP file.
     *
     * @throws \c InitializationException if failed to open MCAP file.
     */
    void open_file_();

    /**
     * @brief Close the MCAP file.
     */
    void close_file_();

    /**
     * @brief Read the MCAP file summary.
     *
     * Reads the MCAP file summary.
     * Checks if the version of the MCAP file is supported.
     */
    void read_mcap_summary_();

    /**
     * @brief Read the MCAP file messages.
     *
     * @return A \c LinearMessageView instance with the messages read.
     */
    mcap::LinearMessageView read_mcap_messages_();

    //! MCAP reader instance.
    mcap::McapReader mcap_reader_;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
