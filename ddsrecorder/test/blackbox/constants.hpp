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

/**
 * @file constants.hpp
 *
 */

#pragma once

#include <cstdint>
#include <string>

#include <fastdds/rtps/common/Time_t.h>
#include <ddspipe_core/types/dds/DomainId.hpp>
#include <ddspipe_core/types/participant/ParticipantId.hpp>


namespace test {

using namespace eprosima;
using namespace eprosima::ddspipe::core::types;

// The domain and topic to publish in.
constexpr DomainIdType DOMAIN = 84;
const std::string TOPIC_NAME = "DdsRecorderBlackboxTestTopic";

// The id of the publishing participant.
const ParticipantId PARTICIPANT_ID = "DdsRecorderBlackboxTestParticipant";

// The maximum amount of time (in seconds) to wait for the subscriber to acknowledge messages.
const fastrtps::Duration_t MAX_WAITING_TIME(10);

} // namespace test
