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
 * @file OutputSettings.hpp
 */

#pragma once

#include <cstdint>
#include <string>

#include <cpp_utils/macros/custom_enumeration.hpp>

#include <ddsrecorder_participants/recorder/output/ResourceLimits.hpp>


namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Structure encapsulating all output configuration options.
 */
struct OutputSettings
{
    //! Path where output file is to be created
    std::string filepath;

    //! Name of the output file
    std::string filename;

    //! Extension of the output file
    std::string extension;

    ///////////////
    // TIMESTAMP //
    ///////////////

    //! Whether to prepend current timestamp when file is created
    bool prepend_timestamp{true};

    //! Format to use in timestamp prefix
    std::string timestamp_format;

    //! Whether to use local or global timestamp
    bool local_timestamp;

    /////////////////////
    // RESOURCE LIMITS //
    /////////////////////

    //! Resource limits configuration
    ResourceLimitsStruct resource_limits;

    bool set_resource_limits(
        const ResourceLimitsStruct& limits,
        std::uint64_t& space_available)
    {
        if(limits.max_size_ > space_available)
        {
            EPROSIMA_LOG_ERROR(DDSRECORDER, "The max size cannot be greater than the available space");
            return false;
        }
        resource_limits = limits;
        return true;
    }

    void set_resource_limits_by_default(
        const ResourceLimitsStruct& limits,
        std::uint64_t space_available)
    {
        resource_limits = limits;
        resource_limits.max_file_size_ = space_available;
        resource_limits.max_size_ = space_available;
    }
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
