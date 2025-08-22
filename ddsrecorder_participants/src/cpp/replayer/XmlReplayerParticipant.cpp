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

#include <ddspipe_participants/reader/auxiliar/BlankReader.hpp>
#include <ddspipe_participants/configuration/XmlParticipantConfiguration.hpp>

#include <ddsrecorder_participants/replayer/XmlReplayerParticipant.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

using namespace eprosima::ddspipe::core;
using namespace eprosima::ddspipe::participants;
using namespace eprosima::ddspipe::participants::dds;

XmlReplayerParticipant::XmlReplayerParticipant(
        const std::shared_ptr<XmlParticipantConfiguration>& participant_configuration,
        const std::shared_ptr<PayloadPool>& payload_pool,
        const std::shared_ptr<DiscoveryDatabase>& discovery_database,
        const bool& replay_types)
    : XmlParticipant(
        participant_configuration,
        payload_pool,
        discovery_database)
    , replay_types_(replay_types)
{
}

std::shared_ptr<IReader> XmlReplayerParticipant::create_reader(
        const ITopic& /* topic */)
{
    return std::make_shared<BlankReader>();
}

void XmlReplayerParticipant::add_qos_properties_(
            fastdds::dds::DomainParticipantQos& qos) const
{
    CommonParticipant::add_qos_properties_(qos);

    if (replay_types_ == true)
    {
        qos.properties().properties().emplace_back(
            "fastdds.type_propagation",
            "enabled");
    }
    else
    {
        qos.properties().properties().emplace_back(
            "fastdds.type_propagation",
            "disabled");
    }
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
