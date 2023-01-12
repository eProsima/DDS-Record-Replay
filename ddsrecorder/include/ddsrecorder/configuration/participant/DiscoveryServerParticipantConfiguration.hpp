// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DiscoveryServerParticipantConfiguration.hpp
 */

#ifndef _DDSRECORDER_CONFIGURATION_PARTICIPANT_DISCOVERYSERVERPARTICIPANTCONFIGURATION_HPP_
#define _DDSRECORDER_CONFIGURATION_PARTICIPANT_DISCOVERYSERVERPARTICIPANTCONFIGURATION_HPP_

#include <ddsrecorder/configuration/participant/SimpleParticipantConfiguration.hpp>
#include <ddsrecorder/library/library_dll.h>
#include <ddsrecorder/types/security/tls/TlsConfiguration.hpp>
#include <ddsrecorder/types/address/Address.hpp>
#include <ddsrecorder/types/address/DiscoveryServerConnectionAddress.hpp>
#include <ddsrecorder/types/dds/DomainId.hpp>
#include <ddsrecorder/types/dds/GuidPrefix.hpp>
#include <ddsrecorder/types/participant/ParticipantKind.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {
namespace configuration {

/**
 * This class joins Discovery Server Participant Configuration features and gives methods to interact with it.
 */
struct DiscoveryServerParticipantConfiguration : public SimpleParticipantConfiguration
{

    /////////////////////////
    // CONSTRUCTORS
    /////////////////////////

    DDSRECORDER_DllAPI DiscoveryServerParticipantConfiguration() = default;

    DDSRECORDER_DllAPI DiscoveryServerParticipantConfiguration(
            const types::ParticipantId& id,
            const types::ParticipantKind& kind,
            const bool is_repeater,
            const types::DomainId& domain_id,
            const types::GuidPrefix& discovery_server_guid_prefix,
            const std::set<types::Address>& listening_addresses,
            const std::set<types::DiscoveryServerConnectionAddress>& connection_addresses,
            const types::security::TlsConfiguration& tls_configuration);

    /////////////////////////
    // METHODS
    /////////////////////////

    DDSRECORDER_DllAPI virtual bool is_valid(
            utils::Formatter& error_msg) const noexcept override;

    DDSRECORDER_DllAPI bool operator ==(
            const DiscoveryServerParticipantConfiguration& other) const noexcept;

    /////////////////////////
    // VARIABLES
    /////////////////////////

    types::GuidPrefix discovery_server_guid_prefix = types::GuidPrefix();

    std::set<types::Address> listening_addresses = {};

    std::set<types::DiscoveryServerConnectionAddress> connection_addresses = {};

    types::security::TlsConfiguration tls_configuration = types::security::TlsConfiguration();
};

} /* namespace configuration */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */

#endif /* _DDSRECORDER_CONFIGURATION_PARTICIPANT_DISCOVERYSERVERPARTICIPANTCONFIGURATION_HPP_ */
