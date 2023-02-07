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
 * @file RecorderParticipant.hpp
 */

#pragma once

#include <ddsrecorder_participants/auxiliar/mcap/McapHandler.hpp>
#include <ddsrecorder_participants/configuration/RecorderParticipantConfiguration.hpp>

#include <ddsrouter_core/participants/participant/auxiliar/BaseParticipant.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * TODO(recorder)
 */
class RecorderParticipant : public ddsrouter::participants::BaseParticipant
{
public:

    /**
     * @brief Construct a new Recorder Participant object
     *
     * It uses the \c BaseParticipant constructor.
     * Apart from BaseParticipant, it adds this new object to a static variable so it could be reached from outside
     * the DDSRouter.
     */
    RecorderParticipant(
            std::shared_ptr<RecorderParticipantConfiguration> participant_configuration,
            std::shared_ptr<ddsrouter::core::PayloadPool> payload_pool,
            std::shared_ptr<ddsrouter::core::DiscoveryDatabase> discovery_database);

    /**
     * @brief Destroy the Recorder Participant object
     *
     * Remove its reference from the static map
     */
    virtual ~RecorderParticipant();

    virtual void start() override;

protected:

    ddsrouter::core::types::Endpoint simulate_endpoint_(const ddsrouter::core::types::DdsTopic& topic) const;

    //! Override create_writer_() BaseParticipant method
    std::shared_ptr<ddsrouter::core::IWriter> create_writer_(
            ddsrouter::core::types::DdsTopic topic) override;

    //! Override create_reader_() BaseParticipant method
    std::shared_ptr<ddsrouter::core::IReader> create_reader_(
            ddsrouter::core::types::DdsTopic topic) override;

    std::shared_ptr<McapHandler> mcap_handler_;

    //! Reference to the configuration to avoid cast every time is used.
    std::shared_ptr<RecorderParticipantConfiguration>& participant_configuration_ref_;

    static constexpr const char* MCAP_FILE = "output.mcap";
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
