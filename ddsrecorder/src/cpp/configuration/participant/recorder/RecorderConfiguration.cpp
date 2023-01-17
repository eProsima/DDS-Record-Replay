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
 * @file RecorderConfiguration.cpp
 */

#include <cpp_utils/time/time_utils.hpp>

#include <ddsrecorder/configuration/participant/recorder/RecorderConfiguration.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {
namespace configuration {

RecorderConfiguration::RecorderConfiguration(
        const std::string& file_name,
        const std::string& path /* = "./" */,
        const std::string& extension /* = ".mcap" */,
        bool use_timestamp /* = true */) noexcept
    : ParticipantConfiguration(file_name, types::ParticipantKind::recorder, false)
    , file_name_(cat_file_name_(file_name, path, extension, use_timestamp))
{
    // Do nothing
}

std::string RecorderConfiguration::file_name() const noexcept
{
    return file_name_;
}

std::string RecorderConfiguration::cat_file_name_(
        const std::string& file_name,
        const std::string& path /* = "./" */,
        const std::string& extension /* = ".mcap" */,
        bool use_timestamp /* = true */) noexcept
{
    std::ostringstream os;

    os << path << "/" << file_name;
    if (use_timestamp)
    {
        os << "_" << utils::to_string(utils::now());
    }
    os << extension;

    return os.str();
}

} /* namespace configuration */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
