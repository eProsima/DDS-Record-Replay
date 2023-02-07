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

// Recorder related tags
constexpr const char* RECORDER_PATH_FILE_TAG("path");
constexpr const char* RECORDER_EXTENSION_FILE_TAG("extension");
constexpr const char* RECORDER_USE_TIMESTAMP_FILE_NAME_TAG("use-timestamp");
constexpr const char* RECORDER_FILE_NAME_TAG("filename");

} /* namespace yaml */
} /* namespace ddsrecorder */
} /* namespace eprosima */
