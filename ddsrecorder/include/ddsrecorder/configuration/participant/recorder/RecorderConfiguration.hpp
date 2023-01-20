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
 * @file RecorderConfiguration.hpp
 */

#pragma once

#include <ddsrecorder/configuration/participant/ParticipantConfiguration.hpp>
#include <ddsrecorder/library/library_dll.h>
#include <ddsrecorder/types/participant/ParticipantId.hpp>
#include <ddsrecorder/types/participant/ParticipantKind.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {
namespace configuration {

struct RecorderConfiguration : public ParticipantConfiguration
{

    /////////////////////////
    // CONSTRUCTORS
    /////////////////////////

    DDSRECORDER_DllAPI RecorderConfiguration(
            const std::string& file_name,
            const std::string& path = "./",
            const std::string& extension = ".mcap",
            bool use_timestamp = true) noexcept;

    /////////////////////////
    // GETTER METHODS
    /////////////////////////

    std::string file_name() const noexcept;

protected:

    /////////////////////////
    // STATIC METHODS
    /////////////////////////

    static std::string cat_file_name_(
            const std::string& file_name,
            const std::string& path = "./",
            const std::string& extension = ".mcap",
            bool use_timestamp = true) noexcept;

    /////////////////////////
    // VARIABLES
    /////////////////////////

    std::string file_name_;
};

} /* namespace configuration */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
