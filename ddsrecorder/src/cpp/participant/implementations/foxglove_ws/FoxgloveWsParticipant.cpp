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
 * @file FoxgloveWsParticipant.cpp
 */

#include <ddsrecorder/types/participant/ParticipantKind.hpp>

#include <participant/implementations/foxglove_ws/FoxgloveWsParticipant.hpp>
#include <reader/implementations/auxiliar/BlankReader.hpp>
#include <writer/implementations/recorder/RecorderWriter.hpp>
#include <writer/implementations/foxglove_ws/FoxgloveWsWriter.hpp>
#include <writer/implementations/recorder/TypeObjectWriter.hpp>
#include <recorder/dynamic_types/types.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {

using namespace eprosima::ddsrecorder::core::types;

FoxgloveWsParticipant::FoxgloveWsParticipant(
        std::shared_ptr<configuration::RecorderConfiguration> participant_configuration,
        std::shared_ptr<PayloadPool> payload_pool,
        std::shared_ptr<DiscoveryDatabase> discovery_database)
    : BaseParticipant(participant_configuration, payload_pool, discovery_database)
    , foxglove_ws_handler_(std::make_shared<recorder::FoxgloveWsHandler>(participant_configuration->file_name().c_str(), payload_pool_))
    , participant_configuration_ref_(participant_configuration)
{
    // Simulate that there is a reader of type object to force this track creation
    discovery_database_->add_endpoint(
        simulate_endpoint_(recorder::type_object_topic())
    );

    // Force for every topic found to create track by creating simulated readers
    // NOTE: this could change for: in DDSRouter change that only readers create track
    discovery_database_->add_endpoint_discovered_callback(
        [this](types::Endpoint endpoint_discovered){
            if (endpoint_discovered.is_writer() && endpoint_discovered.discoverer_participant_id() != this->id())
            {
                discovery_database_->add_endpoint(
                    simulate_endpoint_(endpoint_discovered.topic())
                );
            }
        }
    );
}

FoxgloveWsParticipant::~FoxgloveWsParticipant()
{
    // Do nothing
}

std::shared_ptr<IWriter> FoxgloveWsParticipant::create_writer_(
        DdsTopic topic)
{
    if (recorder::is_type_object_topic(topic))
    {
        return std::make_shared<TypeObjectWriter>(id(), topic, payload_pool_, foxglove_ws_handler_);
    }
    else
    {
        return std::make_shared<FoxgloveWsWriter>(id(), topic, payload_pool_, foxglove_ws_handler_);
    }
}

std::shared_ptr<IReader> FoxgloveWsParticipant::create_reader_(
        DdsTopic topic)
{
    return std::make_shared<BlankReader>();
}

types::Endpoint FoxgloveWsParticipant::simulate_endpoint_(const types::DdsTopic& topic) const
{
    return types::Endpoint(
        types::EndpointKind::reader,
        recorder::new_unique_guid(),
        topic,
        this->id()
    );
}

} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
