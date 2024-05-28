// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#pragma once

#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/types/Fuzzy.hpp>

#include <ddspipe_core/configuration/IConfiguration.hpp>
#include <ddspipe_participants/configuration/ParticipantConfiguration.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Class that encapsulates all configuration parameters of a \c BaseReaderParticipant .
 */
struct BaseReaderParticipantConfiguration : ddspipe::participants::ParticipantConfiguration
{
    utils::Fuzzy<utils::Timestamp> begin_time{};
    utils::Fuzzy<utils::Timestamp> end_time{};
    float rate{1};
    utils::Fuzzy<utils::Timestamp> start_replay_time{};
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
