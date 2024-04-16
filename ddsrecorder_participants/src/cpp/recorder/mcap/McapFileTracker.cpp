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
 * @file McapFileTracker.cpp
 */

#include <filesystem>
#include <stdexcept>

#include <mcap/internal.hpp>

#include <cpp_utils/Formatter.hpp>
#include <cpp_utils/Log.hpp>
#include <cpp_utils/time/time_utils.hpp>

#include <ddsrecorder_participants/recorder/mcap/McapFileTracker.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

McapFileTracker::McapFileTracker(const McapOutputSettings& configuration)
    : configuration_(configuration)
{
}

McapFileTracker::~McapFileTracker()
{
    // Close the current file
    if (!current_file_.filename.empty() && current_file_.size > 0)
    {
        close_file();
    }
}

std::uint64_t McapFileTracker::get_total_size() const
{
    return size_;
}

std::string McapFileTracker::get_current_filename() const
{
    return make_filename_tmp_(current_file_.filename);
}

void McapFileTracker::set_current_file_size(const std::uint64_t& file_size)
{
    if (file_size > configuration_.max_file_size)
    {
        throw std::invalid_argument("Size is greater than the maximum file size.");
    }

    const auto size_diff = file_size - current_file_.size;

    if (size_ + size_diff > configuration_.max_size)
    {
        throw std::runtime_error("Size is greater than the maximum size.");
    }

    current_file_.size = file_size;
}

void McapFileTracker::new_file(const std::uint64_t& min_file_size)
{
    std::lock_guard<std::mutex> lock(mutex_);

    logInfo(DDSRECORDER_MCAP_FILE_TRACKER, "Creating a new file with a minimum size of " << min_file_size << " bytes.");

    if (min_file_size > configuration_.max_file_size)
    {
        throw std::invalid_argument("Minimum file size is greater than the maximum file size.");
    }

    const auto free_space = configuration_.max_size - size_;
    std::int64_t space_to_free = min_file_size - free_space;

    if (space_to_free > 0 && !configuration_.file_rotation)
    {
        throw std::runtime_error("Not enough free space to create a new file. Free space: " + std::to_string(free_space) +
            ", minimum file size: " + std::to_string(min_file_size));
    }

    while (space_to_free > 0)
    {
        // Free space for the new file
        if (closed_files_.empty())
        {
            throw std::runtime_error("All the files have been deleted and there is still not enough free space. "
                "Free space: " + std::to_string(free_space) + ", space to free: " + std::to_string(space_to_free));
        }

        const auto oldest_file_size = remove_oldest_file_nts_();

        size_ -= oldest_file_size;
        space_to_free -= oldest_file_size;
    }

    // Increase the file id
    file_id_++;

    // Generate the new file's name
    const auto filename = generate_filename_();
    const auto tmp_filename = make_filename_tmp_(filename);

    if (std::filesystem::exists(filename))
    {
        throw std::runtime_error("File " + filename + " already exists.");
    }
    else if (std::filesystem::exists(tmp_filename))
    {
        throw std::runtime_error("File " + tmp_filename + " already exists.");
    }

    // Save the new file
    current_file_ = {filename, 0};
}

void McapFileTracker::close_file()
{
    std::lock_guard<std::mutex> lock(mutex_);

    logInfo(DDSRECORDER_MCAP_FILE_TRACKER, "Closing file " << current_file_.filename << " of size " << current_file_.size << " bytes.")

    if (current_file_.filename.empty())
    {
        logError(DDSRECORDER_MCAP_FILE_TRACKER, "No file to close.");
        return;
    }

    if (current_file_.size == 0)
    {
        logError(DDSRECORDER_MCAP_FILE_TRACKER, "File " << current_file_.filename << " is empty.");
        return;
    }

    if (current_file_.size > configuration_.max_file_size)
    {
        throw std::runtime_error("File size is greater than the maximum file size.");
    }

    // Save the current file as written
    closed_files_.push_back(current_file_);
    size_ += current_file_.size;

    std::filesystem::rename(get_current_filename(), current_file_.filename);

    current_file_ = {"", 0};
}

std::uint64_t McapFileTracker::remove_oldest_file_nts_()
{
    logInfo(DDSRECORDER_MCAP_FILE_TRACKER, "Removing the oldest file.")

    if (closed_files_.empty())
    {
        logError(DDSRECORDER_MCAP_FILE_TRACKER, "No files to remove.");
        return 0;
    }

    // Find the oldest file
    const auto oldest_file = closed_files_.front();

    // Remove the oldest file from the list
    closed_files_.erase(closed_files_.begin());

    // Remove the oldest file
    const auto ret = std::filesystem::remove(oldest_file.filename);

    if (!ret)
    {
        logError(DDSRECORDER_MCAP_FILE_TRACKER, "File " << oldest_file.filename << " doesn't exist and could not be deleted.");
        return 0;
    }

    logInfo(DDSRECORDER_MCAP_FILE_TRACKER, "File " << oldest_file.filename << " of size " << oldest_file.size << " removed.");
    return oldest_file.size;
}

std::string McapFileTracker::generate_filename_() const
{
    static const std::string MCAP_EXTENSION = ".mcap";
    static const std::string SEPARATOR = "_";

    auto mcap_filename = configuration_.output_filepath + "/";

    if (configuration_.prepend_timestamp)
    {
        const auto timestamp = utils::timestamp_to_string(
            utils::now(), configuration_.output_timestamp_format,
            configuration_.output_local_timestamp);

        mcap_filename += timestamp + SEPARATOR;
    }

    mcap_filename += configuration_.output_filename;

    const auto is_filename_unique = configuration_.prepend_timestamp;
    const auto possibly_multiple_files = configuration_.max_size > configuration_.max_file_size;

    if (!is_filename_unique && possibly_multiple_files)
    {
        // Include the file's id to make its filename unique
        mcap_filename += SEPARATOR + std::to_string(file_id_);
    }

    mcap_filename += MCAP_EXTENSION;

    return mcap_filename;
}

std::string McapFileTracker::make_filename_tmp_(const std::string& filename) const
{
    static const std::string TMP_SUFFIX = ".tmp~";
    return filename + TMP_SUFFIX;
}


} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
