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
 * @file ParticipantConfiguration.hpp
 */

#ifndef _DDSRECORDER_CONFIGURATION_PARTICIPANT_PARTICIPANTCONFIGURATION_HPP_
#define _DDSRECORDER_CONFIGURATION_PARTICIPANT_PARTICIPANTCONFIGURATION_HPP_

#include <ddsrecorder/configuration/BaseConfiguration.hpp>
#include <ddsrecorder/library/library_dll.h>
#include <ddsrecorder/types/participant/ParticipantId.hpp>
#include <ddsrecorder/types/participant/ParticipantKind.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {
namespace configuration {

struct ParticipantConfiguration : public BaseConfiguration
{

    /////////////////////////
    // CONSTRUCTORS
    /////////////////////////

    DDSRECORDER_DllAPI ParticipantConfiguration() = default;

    DDSRECORDER_DllAPI ParticipantConfiguration(
            const types::ParticipantId& id,
            const types::ParticipantKind& kind,
            const bool is_repeater) noexcept;

    /////////////////////////
    // METHODS
    /////////////////////////

    /**
     * @brief Equal comparator
     *
     * @param [in] other: ParticipantConfiguration to compare.
     * @return True if both configurations are the same, False otherwise.
     */
    DDSRECORDER_DllAPI bool operator ==(
            const ParticipantConfiguration& other) const noexcept;

    DDSRECORDER_DllAPI virtual bool is_valid(
            utils::Formatter& error_msg) const noexcept override;

    //! Participant Id associated with this configuration
    types::ParticipantId id;

    //! Participant Kind of the Participant that this configuration refers.
    types::ParticipantKind kind;

    //! Whether this Participant should connect its readers with its writers.
    bool is_repeater = false;
};

} /* namespace configuration */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */

#endif /* _DDSRECORDER_CONFIGURATION_PARTICIPANT_PARTICIPANTCONFIGURATION_HPP_ */
