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
 * @file yaml_configuration_tags.hpp
 */

#pragma once

#include <set>
#include <string>

namespace eprosima {
namespace ddsrecorder {
namespace yaml {

//////////////////////
// DDS related tags //
//////////////////////
constexpr const char* REPLAYER_DDS_TAG("dds");
constexpr const char* REPLAYER_PROFILE_TAG("replayer-profile");

/////////////////////////
// Replay related tags //
/////////////////////////
constexpr const char* REPLAYER_REPLAY_TAG("replayer");
constexpr const char* REPLAYER_REPLAY_INPUT_TAG("input-file");
constexpr const char* REPLAYER_REPLAY_BEGIN_TAG("begin-time");
constexpr const char* REPLAYER_REPLAY_END_TAG("end-time");
constexpr const char* REPLAYER_REPLAY_RATE_TAG("rate");
constexpr const char* REPLAYER_REPLAY_START_TIME_TAG("start-replay-time");
constexpr const char* REPLAYER_REPLAY_TYPES_TAG("replay-types");

} /* namespace yaml */
} /* namespace ddsrecorder */
} /* namespace eprosima */
