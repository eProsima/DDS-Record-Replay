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
 * @file YamlReaderConfiguration.hpp
 */

#pragma once

#include <ddsrouter_core/configuration/DDSRouterConfiguration.hpp>
#include <ddsrouter_core/participants/participant/configuration/ParticipantConfiguration.hpp>
#include <ddsrouter_core/participants/participant/configuration/SimpleParticipantConfiguration.hpp>

#include <ddsrouter_yaml/Yaml.hpp>

#include <foxgloveweb_yaml/library/library_dll.h>

namespace eprosima {
namespace foxgloveweb {
namespace yaml {

/**
 * @brief Class that encapsulates specific methods to get a full DDSRouter Configuration from a yaml node.
 *
 * TODO: Add version configuration so it could load different versions
 */
class FOXGLOVEWEB_YAML_DllAPI Configuration
{
public:

    Configuration(const ddsrouter::yaml::Yaml& yml);

    Configuration(const std::string& file_path);

    ddsrouter::core::configuration::DDSRouterConfiguration configuration;

    std::shared_ptr<ddsrouter::participants::SimpleParticipantConfiguration> simple_configuration;

    std::shared_ptr<ddsrouter::participants::ParticipantConfiguration> foxglove_ws_configuration;

protected:

    void load_ddsrouter_configuration_(
            const ddsrouter::yaml::Yaml& yml);

    void load_ddsrouter_configuration_from_file_(
            const std::string& file_path);
};

} /* namespace yaml */
} /* namespace foxgloveweb */
} /* namespace eprosima */
