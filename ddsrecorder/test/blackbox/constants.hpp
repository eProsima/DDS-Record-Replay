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

#include <ddsrecorder_participants/recorder/sql/SqlHandlerConfiguration.hpp>

namespace test {

using namespace eprosima;
using namespace eprosima::ddspipe::core::types;

// The domain and topic to publish in.
constexpr DomainIdType DOMAIN = 84;
const std::string TOPIC_NAME = "DdsRecorderBlackboxTestTopic";
const std::string ROS2_TOPIC_NAME = "rt/blackbox_test_topic";

// The id of the publishing participant.
const ParticipantId PARTICIPANT_ID = "DdsRecorderBlackboxTestParticipant";

// The maximum amount of time (in seconds) to wait for the subscriber to acknowledge messages.
const fastdds::Duration_t MAX_WAITING_TIME(10);

namespace handler {

constexpr auto MAX_PENDING_SAMPLES = 5000;
constexpr auto BUFFER_SIZE = 100;
constexpr auto EVENT_WINDOW = 20;
constexpr auto CLEANUP_PERIOD = 0;
constexpr auto LOG_PUBLISH_TIME = false;
constexpr auto ONLY_WITH_SCHEMA = false;
constexpr auto RECORD_TYPES = false;
constexpr auto ROS2_TYPES = false;
constexpr auto DATA_FORMAT = ddsrecorder::participants::DataFormat::both;

}

namespace limits {

constexpr std::uint32_t MAX_SIZE = 30000;
constexpr std::uint32_t MAX_FILE_SIZE = 7500;
constexpr std::uint32_t MAX_FILES = MAX_SIZE / MAX_FILE_SIZE;

constexpr double ACCEPTABLE_ERROR = 0.05;
constexpr std::uint32_t MAX_ACCEPTABLE_FILE_SIZE = MAX_FILE_SIZE * (1 + ACCEPTABLE_ERROR);
constexpr std::uint32_t MIN_ACCEPTABLE_FILE_SIZE = MAX_FILE_SIZE * (1 - ACCEPTABLE_ERROR);

// The minimum number of messages that cause the DDS Recorder to create a new file
constexpr std::uint32_t FILE_OVERFLOW_THRESHOLD = 110;

} // namespace limits
} // namespace test
