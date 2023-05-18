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

#include <ddspipe_participants/configuration/SimpleParticipantConfiguration.hpp>
#include <ddspipe_participants/participant/rtps/SimpleParticipant.hpp>

#include <ddsrecorder_participants/library/library_dll.h>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Participant kind in charge of replaying messages read by a \c McapReaderParticipant.
 * It is mostly equivalent to a \c rtps::SimpleParticipant without reader, only writer.
 */
class ReplayerParticipant : public ddspipe::participants::rtps::SimpleParticipant
{
public:

    /**
     * ReplayerParticipant constructor by required values.
     *
     * Creates ReplayerParticipant instance with given configuration, payload pool and discovery database.
     *
     * @param participant_configuration:  Structure encapsulating all configuration options.
     * @param payload_pool:               Owner of every payload contained in messages to be sent.
     * @param discovery_database:         Reference to a \c DiscoveryDatabase instance.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    ReplayerParticipant(
            const std::shared_ptr<ddspipe::participants::SimpleParticipantConfiguration>& participant_configuration,
            const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
            const std::shared_ptr<ddspipe::core::DiscoveryDatabase>& discovery_database);

    //! Override create_reader_() IParticipant method
    DDSRECORDER_PARTICIPANTS_DllAPI
    std::shared_ptr<ddspipe::core::IReader> create_reader(
            const ddspipe::core::ITopic& topic) override;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
