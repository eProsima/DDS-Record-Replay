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
constexpr const char* RECORDER_OUTPUT_LIBRARY_TAG("library");
constexpr const char* RECORDER_OUTPUT_LIBRARY_MCAP_TAG("mcap");
constexpr const char* RECORDER_OUTPUT_LIBRARY_SQL_TAG("sql");
constexpr const char* RECORDER_OUTPUT_PATH_FILE_TAG("path");
constexpr const char* RECORDER_OUTPUT_FILE_NAME_TAG("filename");
constexpr const char* RECORDER_OUTPUT_TIMESTAMP_FORMAT_TAG("timestamp-format");
constexpr const char* RECORDER_OUTPUT_LOCAL_TIMESTAMP_TAG("local-timestamp");
constexpr const char* RECORDER_OUTPUT_SAFETY_MARGIN_TAG("safety-margin");
constexpr const char* RECORDER_OUTPUT_RESOURCE_LIMITS_TAG("resource-limits");
constexpr const char* RECORDER_OUTPUT_RESOURCE_LIMITS_FILE_ROTATION_TAG("file-rotation");
constexpr const char* RECORDER_OUTPUT_RESOURCE_LIMITS_MAX_SIZE_TAG("max-size");
constexpr const char* RECORDER_OUTPUT_RESOURCE_LIMITS_MAX_FILE_SIZE_TAG("max-file-size");

// Advanced recorder configuration options
constexpr const char* RECORDER_BUFFER_SIZE_TAG("buffer-size");
constexpr const char* RECORDER_EVENT_WINDOW_TAG("event-window");
constexpr const char* RECORDER_LOG_PUBLISH_TIME_TAG("log-publish-time");
constexpr const char* RECORDER_ONLY_WITH_TYPE_TAG("only-with-type");
constexpr const char* RECORDER_RECORD_TYPES_TAG("record-types");
constexpr const char* RECORDER_ROS2_TYPES_TAG("ros2-types");

// Compression settings
constexpr const char* RECORDER_COMPRESSION_SETTINGS_TAG("compression");
constexpr const char* RECORDER_COMPRESSION_SETTINGS_ALGORITHM_TAG("algorithm");
constexpr const char* RECORDER_COMPRESSION_SETTINGS_ALGORITHM_NONE_TAG("none");
constexpr const char* RECORDER_COMPRESSION_SETTINGS_ALGORITHM_LZ4_TAG("lz4");
constexpr const char* RECORDER_COMPRESSION_SETTINGS_ALGORITHM_ZSTD_TAG("zstd");
constexpr const char* RECORDER_COMPRESSION_SETTINGS_LEVEL_TAG("level");
constexpr const char* RECORDER_COMPRESSION_SETTINGS_LEVEL_FASTEST_TAG("fastest");
constexpr const char* RECORDER_COMPRESSION_SETTINGS_LEVEL_FAST_TAG("fast");
constexpr const char* RECORDER_COMPRESSION_SETTINGS_LEVEL_DEFAULT_TAG("default");
constexpr const char* RECORDER_COMPRESSION_SETTINGS_LEVEL_SLOW_TAG("slow");
constexpr const char* RECORDER_COMPRESSION_SETTINGS_LEVEL_SLOWEST_TAG("slowest");
constexpr const char* RECORDER_COMPRESSION_SETTINGS_FORCE_TAG("force");

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
