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
 * @file IFileTracker.hpp
 */

#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include <ddsrecorder_participants/library/library_dll.h>


namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Interface to keep track of files and their sizes.
 */
class IFileTracker
{
public:

    DDSRECORDER_PARTICIPANTS_DllAPI
    virtual ~IFileTracker() = default;

    /**
     * @brief Adds a new file to the tracker.
     *
     * If the current file is not empty, it is saved as written.
     *
     * If file_rotation is set and the new file is too large to fit in the available space, the oldest files are
     * are removed until there is enough available space.
     * The new file is stored as the current file.
     *
     * @param min_file_size The minimum size of the new file.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    virtual void new_file(
            const std::uint64_t min_file_size) = 0;

    /**
     * @brief Closes the current file.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    virtual void close_file() = 0;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
