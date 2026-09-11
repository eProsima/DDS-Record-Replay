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

#include <algorithm>
#include <filesystem>
#include <regex>
#include <system_error>
#include <utility>
#include <vector>

#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/Formatter.hpp>
#include <cpp_utils/Log.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/utils.hpp>

#include <ddsrecorder_participants/recorder/exceptions/FullDiskException.hpp>
#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

namespace {

//! Escapes the characters of \c str that have a special meaning within a regular expression
std::string escape_regex_(
        const std::string& str)
{
    static const std::regex SPECIAL_CHARACTERS(R"([.^$|()\[\]{}*+?\\])");
    return std::regex_replace(str, SPECIAL_CHARACTERS, R"(\$&)");
}

} // namespace

std::string File::to_str() const
{
    return "file_" + std::to_string(id) + " (" + utils::from_bytes(size) + ")";
}

FileTracker::FileTracker(
        const OutputSettings& configuration)
    : configuration_(configuration)
{
    if (configuration_.resource_limits.include_existing_files_)
    {
        add_existing_files_nts_();
    }
}

FileTracker::~FileTracker()
{
    if (!current_file_.name.empty() && current_file_.size > 0)
    {
        close_file();
    }
}

void FileTracker::new_file(
        const std::uint64_t min_file_size)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (min_file_size > configuration_.resource_limits.max_file_size_)
    {
        EPROSIMA_LOG_ERROR(DDSRECORDER_FILE_TRACKER,
                "The minimum file size (" << utils::from_bytes(
                    min_file_size) << ") is greater than the maximum file size ("
                                          << utils::from_bytes(configuration_.resource_limits.max_file_size_) << ").");
        throw utils::InconsistencyException(utils::Formatter()
                      << "The minimum file size ("
                      << utils::from_bytes(min_file_size) << ") is greater than the maximum file size ("
                      << utils::from_bytes(configuration_.resource_limits.max_file_size_) << ").");
    }

    // NOTE: The tracked size may exceed the maximum size when the files already present in the output directory are
    // taken into account, so the free space is calculated with signed arithmetic to avoid an underflow.
    const std::int64_t free_space = static_cast<std::int64_t>(configuration_.resource_limits.max_size_) -
            static_cast<std::int64_t>(size_);
    const std::uint64_t available_space = free_space > 0 ? static_cast<std::uint64_t>(free_space) : 0;
    std::int64_t space_to_free = static_cast<std::int64_t>(min_file_size) - free_space;

    if (space_to_free > 0 && !configuration_.resource_limits.file_rotation_)
    {
        EPROSIMA_LOG_ERROR(DDSRECORDER_FILE_TRACKER,
                "The new file's size (" << utils::from_bytes(
                    min_file_size) << ") is greater than the available space ("
                                        << utils::from_bytes(available_space) << ").");
        throw FullDiskException(
                  "Not enough free space (" + utils::from_bytes(available_space) + ") to create a new file with a "
                  "minimum size of " + utils::from_bytes(min_file_size));
    }

    while (space_to_free > 0)
    {
        // Free space for the new file
        if (closed_files_.empty())
        {
            throw FullDiskException(
                      "After removing all files, there is not enough free space (" +
                      utils::from_bytes(available_space) + ") to create a new file with a minimum file size of " +
                      utils::from_bytes(min_file_size) + ".");
        }

        const auto oldest_file_size = remove_oldest_file_nts_();

        size_ -= oldest_file_size;
        space_to_free -= oldest_file_size;
    }

    EPROSIMA_LOG_INFO(DDSRECORDER_FILE_TRACKER,
            "Creating a new file with a minimum size of " + utils::from_bytes(min_file_size) + ".");

    // Generate the new file's ID.
    // NOTE: The closed files are sorted from the oldest to the newest, which does not necessarily match the order of
    // their ids when the files already present in the output directory are taken into account.
    std::uint64_t id = 0;

    for (const auto& closed_file : closed_files_)
    {
        id = std::max(id, closed_file.id + 1);
    }

    // Generate the new file's name
    const auto name = generate_filename_(id);
    const auto tmp_name = make_filename_tmp_(name);

    if (std::filesystem::exists(name))
    {
        EPROSIMA_LOG_ERROR(DDSRECORDER_FILE_TRACKER, "File " + name + " already exists.");
    }
    else if (std::filesystem::exists(tmp_name))
    {
        EPROSIMA_LOG_ERROR(DDSRECORDER_FILE_TRACKER, "File " + tmp_name + " already exists.");
    }

    // Save the new file
    current_file_ = {id, name, 0};
}

void FileTracker::close_file() noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);

    EPROSIMA_LOG_INFO(DDSRECORDER_FILE_TRACKER, "Closing file " + current_file_.to_str() + ".");

    if (current_file_.name.empty())
    {
        EPROSIMA_LOG_WARNING(DDSRECORDER_FILE_TRACKER, "No file to close.");
        return;
    }

    if (current_file_.size > configuration_.resource_limits.max_file_size_)
    {
        EPROSIMA_LOG_WARNING(DDSRECORDER_FILE_TRACKER,
                current_file_.to_str() + " has a greater file size than the maximum (" +
                utils::from_bytes(configuration_.resource_limits.max_file_size_) + ").");
    }

    // Save the current file as closed
    closed_files_.push_back(current_file_);
    size_ += current_file_.size;

    try
    {
        std::filesystem::rename(get_current_filename(), current_file_.name);
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        EPROSIMA_LOG_ERROR(DDSRECORDER_FILE_TRACKER,
                "Error renaming " + get_current_filename() + ": " << e.what());
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
    if (file_size > configuration_.resource_limits.max_file_size_)
    {
        EPROSIMA_LOG_WARNING(DDSRECORDER_FILE_TRACKER,
                "The file's size (" << utils::from_bytes(file_size) << ") is greater than the maximum file size ("
                                    << utils::from_bytes(configuration_.resource_limits.max_file_size_) << ").");
    }

    // If there's an overflow, the new size is too big
    const auto new_size = size_ + file_size;

    if (new_size > configuration_.resource_limits.max_size_ || new_size < size_)
    {
        EPROSIMA_LOG_WARNING(DDSRECORDER_FILE_TRACKER,
                "The aggregate output size ("
                << utils::from_bytes(size_) << ") plus the new file size ("
                << utils::from_bytes(file_size) << ") is greater than the maximum size ("
                << utils::from_bytes(configuration_.resource_limits.max_size_) << ").");
    }

    current_file_.size = file_size;
}

void FileTracker::add_existing_files_nts_() noexcept
{
    std::error_code error_code;
    const std::filesystem::path directory(configuration_.filepath);

    if (!std::filesystem::is_directory(directory, error_code))
    {
        EPROSIMA_LOG_WARNING(DDSRECORDER_FILE_TRACKER,
                "The output directory " << configuration_.filepath << " does not exist. No existing file is tracked.");
        return;
    }

    // The existing files are collected along with their modification time, so that they can be sorted afterwards
    std::vector<std::pair<std::filesystem::file_time_type, File>> existing_files;

    try
    {
        const auto filename_pattern = existing_filename_pattern_();

        for (const auto& entry : std::filesystem::directory_iterator(directory, error_code))
        {
            if (!entry.is_regular_file(error_code))
            {
                continue;
            }

            const auto filename = entry.path().filename().string();
            std::smatch matches;

            if (!std::regex_match(filename, matches, filename_pattern))
            {
                continue;
            }

            const auto file_size = std::filesystem::file_size(entry.path(), error_code);

            if (error_code)
            {
                EPROSIMA_LOG_WARNING(DDSRECORDER_FILE_TRACKER,
                        "Could not read the size of " << filename << ": " << error_code.message() << ".");
                continue;
            }

            const auto write_time = std::filesystem::last_write_time(entry.path(), error_code);

            if (error_code)
            {
                EPROSIMA_LOG_WARNING(DDSRECORDER_FILE_TRACKER,
                        "Could not read the modification time of " << filename << ": " << error_code.message() << ".");
                continue;
            }

            // Take the file's id from its name, so that the files created in this execution do not reuse it
            std::uint64_t id = 0;

            if (matches.size() > 1 && matches[1].matched)
            {
                try
                {
                    id = std::stoull(matches[1].str());
                }
                catch (const std::exception& e)
                {
                    EPROSIMA_LOG_WARNING(DDSRECORDER_FILE_TRACKER,
                            "Could not read the id of " << filename << ": " << e.what() << ".");
                }
            }

            existing_files.push_back({write_time, File{id, configuration_.filepath + "/" + filename, file_size}});
        }

        // Sort the existing files from the oldest to the most recently modified one
        std::stable_sort(existing_files.begin(), existing_files.end(),
                [](const std::pair<std::filesystem::file_time_type, File>& lhs,
                const std::pair<std::filesystem::file_time_type, File>& rhs)
                {
                    return lhs.first < rhs.first;
                });
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_ERROR(DDSRECORDER_FILE_TRACKER,
                "Error listing the existing files in " << configuration_.filepath << ": " << e.what());
        return;
    }

    for (const auto& existing_file : existing_files)
    {
        EPROSIMA_LOG_INFO(DDSRECORDER_FILE_TRACKER,
                "Tracking the existing file " << existing_file.second.name << ": "
                                              << existing_file.second.to_str() << ".");

        closed_files_.push_back(existing_file.second);
        size_ += existing_file.second.size;
    }

    if (!existing_files.empty())
    {
        EPROSIMA_LOG_INFO(DDSRECORDER_FILE_TRACKER,
                "Tracking " << existing_files.size() << " existing files in " << configuration_.filepath
                            << " with an aggregate size of " << utils::from_bytes(size_) << ".");
    }
}

std::regex FileTracker::existing_filename_pattern_() const
{
    static const std::string SEPARATOR = "_";

    std::string pattern = "^";

    if (configuration_.prepend_timestamp)
    {
        // The timestamp format is configurable, so any prefix ending with the separator is accepted
        pattern += "(?:.+" + escape_regex_(SEPARATOR) + ")?";
    }

    pattern += escape_regex_(configuration_.filename);

    if (configuration_.resource_limits.max_size_ > configuration_.resource_limits.max_file_size_)
    {
        // There may be multiple output files, so their name includes the file's id
        pattern += "(?:" + escape_regex_(SEPARATOR) + "([0-9]+))?";
    }

    pattern += escape_regex_(configuration_.extension) + "$";

    return std::regex(pattern);
}

std::uint64_t FileTracker::remove_oldest_file_nts_() noexcept
{
    EPROSIMA_LOG_INFO(DDSRECORDER_FILE_TRACKER, "Removing the oldest file.");

    if (closed_files_.empty())
    {
        EPROSIMA_LOG_WARNING(DDSRECORDER_FILE_TRACKER, "No files to remove.");
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
        EPROSIMA_LOG_WARNING(DDSRECORDER_FILE_TRACKER,
                "File " << oldest_file.to_str() << " doesn't exist and could not be deleted.");
        return 0;
    }

    EPROSIMA_LOG_INFO(DDSRECORDER_FILE_TRACKER, "File " << oldest_file.to_str() << " removed.");
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

    if (configuration_.resource_limits.max_size_ > configuration_.resource_limits.max_file_size_)
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
