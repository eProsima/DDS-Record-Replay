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

#include <mcap/mcap.hpp>

#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>

#include <ddsrecorder_participants/library/library_dll.h>
#include <ddsrecorder_participants/replayer/McapReaderParticipantConfiguration.hpp>
#include <ddsrecorder_participants/replayer/BaseReaderParticipant.hpp>

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
            std::shared_ptr<McapReaderParticipantConfiguration> configuration,
            std::shared_ptr<ddspipe::core::PayloadPool> payload_pool,
            std::string& file_path);

    /**
     * @brief Read and send messages sequentially (according to timestamp).
     *
     * @throw utils::InconsistencyException if failed to read mcap file.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void process_file() override;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
