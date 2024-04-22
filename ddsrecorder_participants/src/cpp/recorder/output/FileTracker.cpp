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
 * @file FileTracker.cpp
 */

#include <filesystem>
#include <stdexcept>

#include <cpp_utils/Log.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/utils.hpp>

#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>
#include <ddsrecorder_participants/recorder/output/FullDiskException.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

std::string File::to_str() const
{
    return "file_" + std::to_string(id) + " (" + utils::from_bytes(size) + ")";
}

FileTracker::FileTracker(
        const OutputSettings& configuration)
    : configuration_(configuration)
{
}

FileTracker::~FileTracker()
{
    if (!current_file_.name.empty())
    {
        close_file();
    }
}

void FileTracker::new_file(
        const std::uint64_t min_file_size)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (min_file_size > configuration_.max_file_size)
    {
        throw std::invalid_argument(
                  "The minimum file size (" + utils::from_bytes(min_file_size) + ") is greater than the maximum file "
                  "size (" + utils::from_bytes(configuration_.max_file_size) + ").");
    }

    const std::uint64_t free_space = configuration_.max_size - size_;
    std::int64_t space_to_free = min_file_size - free_space;

    if (space_to_free > 0 && !configuration_.file_rotation)
    {
        throw FullDiskException(
                  "Not enough free space (" + utils::from_bytes(free_space) + ") to create a new file with a minimum "
                  "size of " + utils::from_bytes(min_file_size));
    }

    while (space_to_free > 0)
    {
        // Free space for the new file
        if (closed_files_.empty())
        {
            throw FullDiskException(
                      "After removing all files, there is not enough free space (" + utils::from_bytes(free_space) + ")"
                      " to create a new file with a minimum file size of " + utils::from_bytes(min_file_size) + ".");
        }

        const auto oldest_file_size = remove_oldest_file_nts_();

        size_ -= oldest_file_size;
        space_to_free -= oldest_file_size;
    }

    logInfo(DDSRECORDER_FILE_TRACKER,
            "Creating a new file with a minimum size of " + utils::from_bytes(min_file_size) + ".");

    // Generate the new file's ID
    const auto id = closed_files_.empty() ? 0 : closed_files_.back().id + 1;

    // Generate the new file's name
    const auto name = generate_filename_(id);
    const auto tmp_name = make_filename_tmp_(name);

    if (std::filesystem::exists(name))
    {
        logError(DDSRECORDER_FILE_TRACKER, "File " + name + " already exists.");
    }
    else if (std::filesystem::exists(tmp_name))
    {
        logError(DDSRECORDER_FILE_TRACKER, "File " + tmp_name + " already exists.");
    }

    // Save the new file
    current_file_ = {id, name, 0};
}

void FileTracker::close_file() noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);

    logInfo(DDSRECORDER_FILE_TRACKER, "Closing file " + current_file_.to_str() + ".");

    if (current_file_.name.empty())
    {
        logWarning(DDSRECORDER_FILE_TRACKER, "No file to close.");
        return;
    }

    if (current_file_.size > configuration_.max_file_size)
    {
        logWarning(DDSRECORDER_FILE_TRACKER,
                current_file_.to_str() + " has a greater file size than the maximum (" +
                utils::from_bytes(configuration_.max_file_size) + ").");
    }

    // Save the current file as closed
    closed_files_.push_back(current_file_);
    size_ += current_file_.size;

    try
    {
        std::filesystem::rename(get_current_filename(), current_file_.name);
    }
    catch(const std::filesystem::filesystem_error& e)
    {
        logError(DDSRECORDER_FILE_TRACKER,
                "Error renaming " + utils::from_bytes(configuration_.max_file_size) + ": " << e.what());
    }

    current_file_ = File();
}

std::uint64_t FileTracker::get_total_size() const noexcept
{
    return size_;
}

std::string FileTracker::get_current_filename() const noexcept
{
    return make_filename_tmp_(current_file_.name);
}

void FileTracker::set_current_file_size(
        const std::uint64_t file_size) noexcept
{
    if (file_size > configuration_.max_file_size)
    {
        logWarning(DDSRECORDER_FILE_TRACKER,
                   "The file's size (" << utils::from_bytes(file_size) << ") is greater than the maximum file size (" <<
                   utils::from_bytes(configuration_.max_file_size) << ").");
    }

    const auto size_diff = file_size - current_file_.size;

    if (size_ + size_diff > configuration_.max_size)
    {
        logWarning(DDSRECORDER_FILE_TRACKER,
            "The aggregate output size (" << utils::from_bytes(file_size) << ") is greater than the maximum size (" <<
            utils::from_bytes(configuration_.max_file_size) << ").");
    }

    current_file_.size = file_size;
}

std::uint64_t FileTracker::remove_oldest_file_nts_() noexcept
{
    logInfo(DDSRECORDER_FILE_TRACKER, "Removing the oldest file.")

    if (closed_files_.empty())
    {
        logWarning(DDSRECORDER_FILE_TRACKER, "No files to remove.");
        return 0;
    }

    // Find the oldest file
    const auto oldest_file = closed_files_.front();

    // Remove the oldest file from the list
    closed_files_.erase(closed_files_.begin());

    // Remove the oldest file
    const auto ret = std::filesystem::remove(oldest_file.name);

    if (!ret)
    {
        logWarning(DDSRECORDER_FILE_TRACKER,
                "File " << oldest_file.to_str() << " doesn't exist and could not be deleted.");
        return 0;
    }

    logInfo(DDSRECORDER_FILE_TRACKER, "File " << oldest_file.to_str() << " removed.");
    return oldest_file.size;
}

std::string FileTracker::generate_filename_(
        const std::uint64_t id) const noexcept
{
    static const std::string SEPARATOR = "_";

    auto filename = configuration_.filepath + "/";

    if (configuration_.prepend_timestamp)
    {
        const auto timestamp = utils::timestamp_to_string(
            utils::now(), configuration_.timestamp_format,
            configuration_.local_timestamp);

        filename += timestamp + SEPARATOR;
    }

    filename += configuration_.filename;

    if (configuration_.max_size > configuration_.max_file_size)
    {
        // There may be multiple output files. Include the file's id to make the filename unique.
        // NOTE: Appending the timestamp doesn't make the filename unique, since multiple can be created simultaneously.
        filename += SEPARATOR + std::to_string(id);
    }

    filename += configuration_.extension;

    return filename;
}

std::string FileTracker::make_filename_tmp_(
        const std::string& filename) const noexcept
{
    static const std::string TMP_SUFFIX = ".tmp~";
    return filename + TMP_SUFFIX;
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
