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
 * @file DDSRouterConfiguration.hpp
 */

#ifndef _DDSRECORDER_CONFIGURATION_DDSRECORDERCONFIGURATION_HPP_
#define _DDSRECORDER_CONFIGURATION_DDSRECORDERCONFIGURATION_HPP_

#include <memory>
#include <set>

#include <cpp_utils/Formatter.hpp>

#include <ddsrecorder/configuration/DDSRouterReloadConfiguration.hpp>
#include <ddsrecorder/configuration/SpecsConfiguration.hpp>
#include <ddsrecorder/configuration/participant/ParticipantConfiguration.hpp>
#include <ddsrecorder/library/library_dll.h>
#include <ddsrecorder/types/topic/filter/DdsFilterTopic.hpp>
#include <ddsrecorder/types/topic/dds/DdsTopic.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {
namespace configuration {

/**
 * This data struct joins every DDSRouter feature configuration such as:
 * - Modifiable values (from \c DDSRouterReloadConfiguration ).
 * - Participant configurations.
 * - Advanced configurations.
 */
struct DDSRouterConfiguration : public DDSRouterReloadConfiguration
{

    /////////////////////////
    // CONSTRUCTORS
    /////////////////////////

    //! Default constructor
    DDSRECORDER_DllAPI DDSRouterConfiguration() = default;

    /**
     * @brief Constructor with arguments to fill new object.
     *
     * @todo use const & references or even eliminate this constructor
     */
    DDSRECORDER_DllAPI DDSRouterConfiguration(
            std::set<std::shared_ptr<types::DdsFilterTopic>> allowlist,
            std::set<std::shared_ptr<types::DdsFilterTopic>> blocklist,
            std::set<std::shared_ptr<types::DdsTopic>> builtin_topics,
            std::set<std::shared_ptr<ParticipantConfiguration>> participants_configurations,
            const SpecsConfiguration& advanced_options);

    /////////////////////////
    // METHODS
    /////////////////////////

    //! Set internal values with the values reloaded
    DDSRECORDER_DllAPI void reload(
            const DDSRouterReloadConfiguration& new_configuration);

    //! Override \c is_valid method.
    DDSRECORDER_DllAPI bool is_valid(
            utils::Formatter& error_msg) const noexcept override;

    /////////////////////////
    // VARIABLES
    /////////////////////////

    //! Participant configurations
    std::set<std::shared_ptr<ParticipantConfiguration>> participants_configurations = {};

    //! Advanced configurations
    SpecsConfiguration advanced_options;

protected:

    //! Auxiliar method to validate that class type of the participants are compatible with their kinds.
    static bool check_correct_configuration_object_(
            const std::shared_ptr<ParticipantConfiguration> configuration);

};

} /* namespace configuration */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */

#endif /* _DDSRECORDER_CONFIGURATION_DDSRECORDERCONFIGURATION_HPP_ */
