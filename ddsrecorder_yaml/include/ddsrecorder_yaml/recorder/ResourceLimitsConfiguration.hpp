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
 * @file ResourceLimitsConfiguration.hpp
 */

#pragma once

#include <cstdint>

#include <ddspipe_yaml/Yaml.hpp>

#include <ddsrecorder_participants/recorder/output/ResourceLimits.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace yaml {

using namespace eprosima::ddspipe::yaml;

/**
 * Class encapsulating the resource limits configuration options.
 */
class ResourceLimitsConfiguration
{
public:

    ResourceLimitsConfiguration() = default;

    ResourceLimitsConfiguration(
            const eprosima::Yaml& yml,
            const YamlReaderVersion& version);

    bool are_limits_valid(
            utils::Formatter& error_msg,
            bool safety_margin);

    participants::ResourceLimitsStruct resource_limits_struct;
};

} /* namespace yaml */
} /* namespace ddsrecorder */
} /* namespace eprosima */