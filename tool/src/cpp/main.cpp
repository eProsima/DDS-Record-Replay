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
 * @file main.cpp
 *
 */

#include <cpp_utils/Log.hpp>
#include <cpp_utils/time/time_utils.hpp>

#include <ddsrecorder/configuration/DDSRouterConfiguration.hpp>
#include <ddsrecorder/configuration/participant/ParticipantConfiguration.hpp>
#include <ddsrecorder/configuration/participant/SimpleParticipantConfiguration.hpp>
#include <ddsrecorder/core/DDSRouter.hpp>

#include "user_interface/arguments_configuration.hpp"
#include "user_interface/ProcessReturnCode.hpp"

using namespace eprosima::ddsrecorder;

int main(
        int argc,
        char** argv)
{
    // TODO(recorder)
    // This is a very simple an manual implementation to check functionality.
    // Extend with:
    // * arguments
    // * signal handlers
    // * yaml?

    // eprosima::utils::Log::SetVerbosity(eprosima::utils::Log::Kind::Info);

    std::cout << "Starting DDS Recorder" << std::endl;

    // Generate Configuration with 2 participants, 1 simple in domain 0 and 1 recorder
    auto simple_conf = std::make_shared<core::configuration::SimpleParticipantConfiguration>(
        core::types::ParticipantId("Simple0"),
        core::types::ParticipantKind::simple_rtps,
        false,
        core::types::DomainId(100u)
    );
    auto recorder_conf = std::make_shared<core::configuration::ParticipantConfiguration>(
        core::types::ParticipantId("Recorder"),
        core::types::ParticipantKind::recorder,
        false
    );

    core::configuration::DDSRouterConfiguration router_configuration;
    router_configuration.participants_configurations.insert(simple_conf);
    router_configuration.participants_configurations.insert(recorder_conf);

    std::cout << "Configuration Created" << std::endl;

    // Create DDS Router
    core::DDSRouter router(router_configuration);
    router.start();

    std::cout << "Router Started" << std::endl;

    // Wait N seconds and close
    eprosima::utils::sleep_for(10000);
    router.stop();

    std::cout << "Router Stopped" << std::endl;

    // Force print every log before closing
    eprosima::utils::Log::Flush();

    return static_cast<int>(ui::ProcessReturnCode::success);
}
