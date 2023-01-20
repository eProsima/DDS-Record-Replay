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

#include <ddsrecorder/configuration/participant/recorder/RecorderConfiguration.hpp>

#include <participant/implementations/auxiliar/BaseParticipant.hpp>
#include <reader/implementations/auxiliar/DummyReader.hpp>
#include <writer/implementations/auxiliar/DummyWriter.hpp>
#include <recorder/mcap/McapHandler.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {

/**
 * TODO(recorder)
 */
class RecorderParticipant : public BaseParticipant
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
            std::shared_ptr<configuration::RecorderConfiguration> participant_configuration,
            std::shared_ptr<PayloadPool> payload_pool,
            std::shared_ptr<DiscoveryDatabase> discovery_database);

    /**
     * @brief Destroy the Recorder Participant object
     *
     * Remove its reference from the static map
     */
    virtual ~RecorderParticipant();

protected:

    types::Endpoint simulate_endpoint_(const types::DdsTopic& topic) const;

    //! Override create_writer_() BaseParticipant method
    std::shared_ptr<IWriter> create_writer_(
            types::DdsTopic topic) override;

    //! Override create_reader_() BaseParticipant method
    std::shared_ptr<IReader> create_reader_(
            types::DdsTopic topic) override;

    std::shared_ptr<recorder::McapHandler> mcap_handler_;

    //! Reference to the configuration to avoid cast every time is used.
    std::shared_ptr<configuration::RecorderConfiguration>& participant_configuration_ref_;

    static constexpr const char* MCAP_FILE = "output.mcap";
};

} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
