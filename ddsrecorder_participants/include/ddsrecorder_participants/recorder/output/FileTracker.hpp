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
 * @file FileTracker.hpp
 */

#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include <cpp_utils/utils.hpp>

#include <ddsrecorder_participants/library/library_dll.h>
#include <ddsrecorder_participants/recorder/output/IFileTracker.hpp>
#include <ddsrecorder_participants/recorder/output/OutputSettings.hpp>


namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Structure encapsulating a tracked MCAP file.
 */
struct File
{
    DDSRECORDER_PARTICIPANTS_DllAPI
    std::string to_str() const;

    std::uint64_t id;
    std::string name;
    std::uint64_t size;
};


/**
 * Class to keep track of files and their sizes.
 */
class FileTracker : IFileTracker
{
public:

    DDSRECORDER_PARTICIPANTS_DllAPI
    FileTracker(
            const OutputSettings& configuration);

    DDSRECORDER_PARTICIPANTS_DllAPI
    virtual ~FileTracker();

    /**
     * @brief Adds a new file to the tracker.
     *
     * If the current file is not empty, it is saved as written.
     *
     * If \c file_rotation is set and the new file is too large to fit in the available space, the oldest files are
     * are removed until there is enough available space.
     * The new file is stored as the current file.
     *
     * @param min_file_size The minimum size of the new file.
     * @throws \c FullDiskException if the disk is full.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void new_file(
            const std::uint64_t min_file_size) override;

    /**
     * @brief Closes the current file.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void close_file() noexcept override;

    /**
     * @brief Adds up the size of all the files in the tracker.
     *
     * It adds up the size of the closed files and of the current file.
     *
     * @return The total size of the files in the tracker.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    std::uint64_t get_total_size() const noexcept;

    /**
     * @brief Calculates the temporary filename of the current file.
     *
     * @return The temporary filename of the current file.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    std::string get_current_filename() const noexcept;

    /**
     * @brief Updates the size of the current file.
     *
     * @param size The new size of the current file.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void set_current_file_size(
            const std::uint64_t size) noexcept;

protected:

    /**
     * @brief Removes the oldest file from the tracker.
     *
     * @return The size of the removed file.
     */
    std::uint64_t remove_oldest_file_nts_() noexcept;

    /**
     * @brief Generates a filename for the current file id.
     *
     * @return The generated filename.
     */
    std::string generate_filename_(
            const std::uint64_t id) const noexcept;

    /**
     * @brief Generates a temporary filename for the given filename.
     *
     * @param filename The filename to generate the temporary filename for.
     * @return The generated temporary filename.
     */
    std::string make_filename_tmp_(
            const std::string& filename) const noexcept;

    // Configuration options
    const OutputSettings configuration_;

    // Mutex to protect the list of files
    std::mutex mutex_;

    // The list of files that have been closed
    std::vector<File> closed_files_;

    // The file that is currently being written
    File current_file_;

    // The total size of all files in the tracker
    std::uint64_t size_{0};
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
