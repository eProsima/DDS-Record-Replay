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

    //! Safety margin on file's size estimation
    std::uint64_t safety_margin;

    //! Maximum size of the output file
    std::uint64_t max_file_size;

    //! Maximum aggregate size of the output files
    std::uint64_t max_size;

    //! Whether to rotate output files after reaching the max-size
    bool file_rotation{false};
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
