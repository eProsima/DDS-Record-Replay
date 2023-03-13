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
constexpr const char* RECORDER_DDS_TAG("dds");

///////////////////////////
// Recorder related tags //
///////////////////////////
constexpr const char* RECORDER_RECORDER_TAG("recorder");

// Output related tags
constexpr const char* RECORDER_OUTPUT_TAG("output");
constexpr const char* RECORDER_PATH_FILE_TAG("path");
constexpr const char* RECORDER_FILE_NAME_TAG("filename");

// Advanced recorder configuration options
constexpr const char* RECORDER_BUFFER_SIZE_TAG("buffer-size");
constexpr const char* RECORDER_EVENT_WINDOW_TAG("event-window");
constexpr const char* RECORDER_LOG_PUBLISH_TIME_TAG("log-publish-time");

////////////////////////////////////
// Remote controller related tags //
////////////////////////////////////
constexpr const char* RECORDER_REMOTE_CONTROLLER_TAG("remote-controller");
constexpr const char* RECORDER_REMOTE_CONTROLLER_ENABLE_TAG("enable");
constexpr const char* RECORDER_REMOTE_CONTROLLER_INITIAL_STATE_TAG("initial-state");
constexpr const char* RECORDER_REMOTE_CONTROLLER_COMMAND_TOPIC_NAME_TAG("command-topic-name");
constexpr const char* RECORDER_REMOTE_CONTROLLER_STATUS_TOPIC_NAME_TAG("status-topic-name");

////////////////
// Specs tags //
////////////////
constexpr const char* RECORDER_SPECS_MAX_PENDING_SAMPLES_TAG("max-pending-samples");
constexpr const char* RECORDER_SPECS_CLEANUP_PERIOD_TAG("cleanup-period");

} /* namespace yaml */
} /* namespace ddsrecorder */
} /* namespace eprosima */
