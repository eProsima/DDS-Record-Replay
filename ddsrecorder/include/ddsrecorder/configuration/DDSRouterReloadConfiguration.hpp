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
 * @file DDSRouterReloadConfiguration.hpp
 */

#ifndef _DDSRECORDER_CONFIGURATION_DDSRECORDERRELOADCONFIGURATION_HPP_
#define _DDSRECORDER_CONFIGURATION_DDSRECORDERRELOADCONFIGURATION_HPP_

#include <memory>
#include <set>

#include <cpp_utils/Formatter.hpp>

#include <ddsrecorder/configuration/BaseConfiguration.hpp>
#include <ddsrecorder/configuration/participant/ParticipantConfiguration.hpp>
#include <ddsrecorder/library/library_dll.h>
#include <ddsrecorder/types/topic/filter/DdsFilterTopic.hpp>
#include <ddsrecorder/types/topic/dds/DdsTopic.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {
namespace configuration {

/**
 * This class joins every DDSRouter feature configuration and includes methods
 * to interact with this configuration.
 */
struct DDSRouterReloadConfiguration : public BaseConfiguration
{

    /////////////////////////
    // CONSTRUCTORS
    /////////////////////////

    DDSRECORDER_DllAPI DDSRouterReloadConfiguration() = default;

    DDSRECORDER_DllAPI DDSRouterReloadConfiguration(
            std::set<std::shared_ptr<types::DdsFilterTopic>> allowlist,
            std::set<std::shared_ptr<types::DdsFilterTopic>> blocklist,
            std::set<std::shared_ptr<types::DdsTopic>> builtin_topics);

    /////////////////////////
    // METHODS
    /////////////////////////

    DDSRECORDER_DllAPI bool is_valid(
            utils::Formatter& error_msg) const noexcept override;

    /////////////////////////
    // VARIABLES
    /////////////////////////

    std::set<std::shared_ptr<types::DdsFilterTopic>> allowlist = {};

    std::set<std::shared_ptr<types::DdsFilterTopic>> blocklist = {};

    std::set<std::shared_ptr<types::DdsTopic>> builtin_topics = {};
};

} /* namespace configuration */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */

#endif /* _DDSRECORDER_CONFIGURATION_DDSRECORDERRELOADCONFIGURATION_HPP_ */
