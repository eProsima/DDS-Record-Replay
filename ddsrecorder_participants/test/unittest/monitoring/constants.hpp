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

#include <fastdds/rtps/common/Time_t.hpp>
#include <ddspipe_core/types/dds/DomainId.hpp>
#include <ddspipe_core/types/participant/ParticipantId.hpp>

namespace test {
namespace monitor {

using namespace eprosima;
using namespace eprosima::ddspipe::core::types;

// Period at which the Monitor should log data.
constexpr std::uint32_t PERIOD_MS = 500;
constexpr double PERIOD_SECS = (double) PERIOD_MS / 1000;

// Maximum time to wait for a sample to arrive.
const auto MAX_WAITING_TIME = fastdds::dds::Duration_t(PERIOD_SECS * 3);

// The domain and topic the Monitor should publish its data in.
constexpr DomainIdType DOMAIN = 84;
const std::string TOPIC_NAME = "DdsMonitoringTopicsTopic";

// The id of participant subscribed to the Monitor's topic.
const ParticipantId PARTICIPANT_ID = "MonitorTopicsTestParticipant";

// The fake topic the test will use to call the macros.
const std::string MOCK_TOPIC_NAME = "MonitoredTopic";
const std::string MOCK_TYPE_NAME = "MonitoredTopicType";

// The fake participant the test will use to call the macros.
const ParticipantId MOCK_PARTICIPANT_ID = "MonitoredParticipant";

} // namespace monitor
} // namespace test
