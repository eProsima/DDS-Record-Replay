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
 * @file RecorderParticipant.cpp
 */

#include <ddsrouter_core/participants/reader/auxiliar/BlankReader.hpp>

#include <auxiliar/dynamic_types/types.hpp>
#include <ddsrecorder_participants/writer/RecorderWriter.hpp>
#include <ddsrecorder_participants/writer/TypeObjectWriter.hpp>

#include <ddsrecorder_participants/participant/RecorderParticipant.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

using namespace eprosima::ddsrouter::core;
using namespace eprosima::ddsrouter::core::types;
using namespace eprosima::ddsrouter::participants;

RecorderParticipant::RecorderParticipant(
        std::shared_ptr<RecorderParticipantConfiguration> participant_configuration,
        std::shared_ptr<PayloadPool> payload_pool,
        std::shared_ptr<DiscoveryDatabase> discovery_database)
    : BaseParticipant(participant_configuration, payload_pool, discovery_database)
    , mcap_handler_(std::make_shared<McapHandler>(participant_configuration->file_name().c_str()))
    , participant_configuration_ref_(participant_configuration)
{
    // Simulate that there is a reader of type object to force this track creation
    discovery_database_->add_endpoint(
        simulate_endpoint_(detail::type_object_topic())
    );

    // Force for every topic found to create track by creating simulated readers
    // NOTE: this could change for: in DDSRouter change that only readers create track
    discovery_database_->add_endpoint_discovered_callback(
        [this](Endpoint endpoint_discovered){
            if (endpoint_discovered.is_writer() && endpoint_discovered.discoverer_participant_id() != this->id())
            {
                discovery_database_->add_endpoint(
                    simulate_endpoint_(endpoint_discovered.topic())
                );
            }
        }
    );
}

RecorderParticipant::~RecorderParticipant()
{
    // Do nothing
}

void RecorderParticipant::start()
{
    // TODO
}

std::shared_ptr<IWriter> RecorderParticipant::create_writer_(
        DdsTopic topic)
{
    if (detail::is_type_object_topic(topic))
    {
        return std::make_shared<TypeObjectWriter>(id(), topic, payload_pool_, mcap_handler_);
    }
    else
    {
        return std::make_shared<RecorderWriter>(id(), topic, payload_pool_, mcap_handler_);
    }
}

std::shared_ptr<IReader> RecorderParticipant::create_reader_(
        DdsTopic topic)
{
    return std::make_shared<BlankReader>();
}

Endpoint RecorderParticipant::simulate_endpoint_(const DdsTopic& topic) const
{
    return Endpoint(
        EndpointKind::reader,
        detail::new_unique_guid(),
        topic,
        this->id()
    );
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
