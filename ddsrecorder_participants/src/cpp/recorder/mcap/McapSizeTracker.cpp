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
 * @file McapSizeTracker.cpp
 */

#include <filesystem>
#include <stdexcept>

#include <mcap/internal.hpp>

#include <cpp_utils/Formatter.hpp>
#include <cpp_utils/Log.hpp>
#include <cpp_utils/utils.hpp>

#include <ddsrecorder_participants/recorder/mcap/McapSizeTracker.hpp>
#include <ddsrecorder_participants/recorder/output/FullFileException.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

McapSizeTracker::McapSizeTracker()
{
}

McapSizeTracker::~McapSizeTracker()
{
}

void McapSizeTracker::init(
        const std::uint64_t& space_available,
        const std::uint64_t& safety_margin)
{
    logInfo(DDSRECORDER_MCAP_SIZE_TRACKER,
            "Initializing tracker with a size of " << utils::from_bytes(space_available));

    if (enabled_)
    {
        logWarning(DDSRECORDER_MCAP_SIZE_TRACKER,
                "Attempting to initialize already enabled tracker.");
        return;
    }

    disk_full_ = false;

    // TODO: add check that space is greater than file overhead

    potential_mcap_size_ = MCAP_FILE_OVERHEAD + safety_margin;
    written_mcap_size_ = MCAP_FILE_OVERHEAD + safety_margin;
    min_mcap_size_ = MCAP_FILE_OVERHEAD + safety_margin;

    space_available_ = space_available;

    enabled_ = true;
}

void McapSizeTracker::reset(
        const std::string& filepath)
{
    logInfo(DDSRECORDER_MCAP_SIZE_TRACKER,
            "Resetting tracker for: " << filepath);

    if (!enabled_)
    {
        logWarning(DDSRECORDER_MCAP_SIZE_TRACKER,
                "Attempting to reset disabled tracker.");
        return;
    }

    enabled_ = false;

    if (written_mcap_size_ > space_available_)
    {
        logWarning(DDSRECORDER_MCAP_SIZE_TRACKER,
                "Written size exceeds available space in disk.");
        // assert(false); // TODO: uncomment when estimation is exact
    }
    else if (potential_mcap_size_ != written_mcap_size_)
    {
        logWarning(DDSRECORDER_MCAP_SIZE_TRACKER,
                "Written size exceeds potential one.");
        // assert(false); // TODO: uncomment when estimation is exact
    }

    // TODO: perform this check only for the case without compression
    // // Check actual size of file is the same as the expected one
    // std::filesystem::path p{filepath};
    // auto actual_written_size = std::filesystem::file_size(p);
    // if (written_mcap_size_ != actual_written_size)
    // {
    //     logWarning(DDSRECORDER_MCAP_SIZE_TRACKER,
    //             "Actual written size is different from expected size: " << actual_written_size << " vs " << written_mcap_size_);
    //     // assert(false); // TODO: uncomment when estimation is exact
    // }
}

void McapSizeTracker::message_to_write(
        const uint64_t& data_size)
{
    check_and_increase_potential_mcap_size_(get_message_size_(data_size));
}

void McapSizeTracker::message_written(
        const uint64_t& data_size)
{
    check_and_increase_written_mcap_size_(get_message_size_(data_size));
}

void McapSizeTracker::schema_to_write(
        const mcap::Schema& schema)
{
    static constexpr bool INCREASE_MIN_MCAP_SIZE = true;
    check_and_increase_potential_mcap_size_(get_schema_size_(schema), INCREASE_MIN_MCAP_SIZE);
}

void McapSizeTracker::schema_written(
        const mcap::Schema& schema)
{
    check_and_increase_written_mcap_size_(get_schema_size_(schema));
}

void McapSizeTracker::channel_to_write(
        const mcap::Channel& channel)
{
    static constexpr bool INCREASE_MIN_MCAP_SIZE = true;
    check_and_increase_potential_mcap_size_(get_channel_size_(channel), INCREASE_MIN_MCAP_SIZE);
}

void McapSizeTracker::channel_written(
        const mcap::Channel& channel)
{
    check_and_increase_written_mcap_size_(get_channel_size_(channel));
}

void McapSizeTracker::attachment_to_write(
        const uint64_t& payload_size)
{
    static constexpr bool INCREASE_MIN_MCAP_SIZE = true;
    check_and_increase_potential_mcap_size_(get_attachment_size_(payload_size), INCREASE_MIN_MCAP_SIZE);
}

void McapSizeTracker::attachment_to_write(
        const uint64_t& payload_size_to_write,
        const uint64_t& payload_size_to_remove)
{
    if (!can_increase_potential_mcap_size_(
                get_attachment_size_(payload_size_to_write),
                get_attachment_size_(payload_size_to_remove)))
    {
        throw FullFileException(
                  STR_ENTRY << "Attempted attachment write of size: " << utils::from_bytes(payload_size_to_write) <<
                      ", but there is not enough space available on disk: " << utils::from_bytes(space_available_),
                      payload_size_to_write);
    }

    static constexpr bool DECREASE_MIN_MCAP_SIZE = true;
    decrease_potential_mcap_size_(get_attachment_size_(payload_size_to_remove), DECREASE_MIN_MCAP_SIZE);

    attachment_to_write(payload_size_to_write);
}

void McapSizeTracker::attachment_written(
        const uint64_t& payload_size)
{
    check_and_increase_written_mcap_size_(get_attachment_size_(payload_size));
}

void McapSizeTracker::metadata_to_write(
        const mcap::Metadata& metadata)
{
    static constexpr bool INCREASE_MIN_MCAP_SIZE = true;
    check_and_increase_potential_mcap_size_(get_metadata_size_(metadata), INCREASE_MIN_MCAP_SIZE);
}

void McapSizeTracker::metadata_written(
        const mcap::Metadata& metadata)
{
    check_and_increase_written_mcap_size_(get_metadata_size_(metadata));
}

std::uint64_t McapSizeTracker::get_potential_mcap_size() const
{
    return potential_mcap_size_;
}

std::uint64_t McapSizeTracker::get_written_mcap_size() const
{
    return written_mcap_size_;
}

std::uint64_t McapSizeTracker::get_min_mcap_size() const
{
    return min_mcap_size_;
}

bool McapSizeTracker::can_increase_potential_mcap_size_(
        const std::uint64_t& size)
{
    if (!enabled_ || disk_full_)
    {
        return false;
    }

    return (potential_mcap_size_ + size) <= space_available_;
}

bool McapSizeTracker::can_increase_potential_mcap_size_(
        const std::uint64_t& size_to_write,
        const std::uint64_t& size_to_remove)
{
    if (!enabled_ || disk_full_)
    {
        return false;
    }

    if (potential_mcap_size_ < size_to_remove)
    {
        logWarning(DDSRECORDER_MCAP_SIZE_TRACKER,
                "Attempting to decrease potential size more than possible.");
        // assert(false); // TODO: uncomment when estimation is exact
        return false;
    }

    return (potential_mcap_size_ - size_to_remove + size_to_write) <= space_available_;
}

void McapSizeTracker::check_and_increase_potential_mcap_size_(
        const std::uint64_t& size,
        const bool increase_min_mcap_size /* = false */)
{
    if (!enabled_)
    {
        logWarning(DDSRECORDER_MCAP_SIZE_TRACKER,
                "Attempting to increase potential size in disabled tracker.");
        return;
    }

    if (disk_full_ || !can_increase_potential_mcap_size_(size))
    {
        disk_full_ = true;
        throw FullFileException(
                  STR_ENTRY << "Attempted to write " << utils::from_bytes(size) << " on an MCAP of "
                            << utils::from_bytes(potential_mcap_size_) << " but there is not enough space available: "
                            << utils::from_bytes(space_available_ - potential_mcap_size_) << "."
                      , size);
    }

    potential_mcap_size_ += size;

    if (increase_min_mcap_size)
    {
        // NOTE: The minimum size must increase even if the potential size can't.
        min_mcap_size_ += size;
    }
}

void McapSizeTracker::decrease_potential_mcap_size_(
        const std::uint64_t& size,
        const bool decrease_min_mcap_size /* = false */)
{
    if (!enabled_)
    {
        logWarning(DDSRECORDER_MCAP_SIZE_TRACKER,
                "Attempting to decrease potential size in disabled tracker.");
        return;
    }

    if (potential_mcap_size_ < size)
    {
        logWarning(DDSRECORDER_MCAP_SIZE_TRACKER,
                "Attempting to decrease potential size more than possible.");
        // assert(false); // TODO: uncomment when estimation is exact
        return;
    }

    potential_mcap_size_ -= size;

    if (decrease_min_mcap_size)
    {
        min_mcap_size_ -= size;
    }
}

void McapSizeTracker::check_and_increase_written_mcap_size_(
        const std::uint64_t& size)
{
    if (!enabled_)
    {
        logWarning(DDSRECORDER_MCAP_SIZE_TRACKER,
                "Attempting to increase written size in disabled tracker.");
        return;
    }

    if ((written_mcap_size_ + size) > space_available_)
    {
        logWarning(DDSRECORDER_MCAP_SIZE_TRACKER,
                "Written size exceeds available space in disk.");
        // assert(false); // TODO: uncomment when estimation is exact
        return;
    }

    if ((written_mcap_size_ + size) > potential_mcap_size_)
    {
        logWarning(DDSRECORDER_MCAP_SIZE_TRACKER,
                "Written size exceeds potential one.");
        // assert(false); // TODO: uncomment when estimation is exact
        return;
    }

    written_mcap_size_ += size;
}

std::uint64_t McapSizeTracker::get_message_size_(
        const uint64_t& data_size)
{
    constexpr std::uint64_t NUMBER_OF_TIMES_COPIED = 1;

    std::uint64_t size = MCAP_MESSAGE_OVERHEAD;
    size += data_size;
    size *= NUMBER_OF_TIMES_COPIED;

    return size;
}

std::uint64_t McapSizeTracker::get_schema_size_(
        const mcap::Schema& schema)
{
    constexpr std::uint64_t NUMBER_OF_TIMES_COPIED = 2;
    constexpr std::uint64_t CONST_SCHEMA = 5;

    std::uint64_t size = MCAP_SCHEMA_OVERHEAD;
    size += schema.name.size();
    size += schema.encoding.size();
    size += schema.data.size();
    size *= NUMBER_OF_TIMES_COPIED;

    size -= CONST_SCHEMA;

    return size;
}

std::uint64_t McapSizeTracker::get_channel_size_(
        const mcap::Channel& channel)
{
    constexpr int NUMBER_OF_TIMES_COPIED = 2;

    std::uint64_t size = MCAP_CHANNEL_OVERHEAD;
    size += channel.topic.size();
    size += channel.messageEncoding.size();
    size += mcap::internal::KeyValueMapSize(channel.metadata);
    size *= NUMBER_OF_TIMES_COPIED;

    return size;
}

std::uint64_t McapSizeTracker::get_attachment_size_(
        const uint64_t& payload_size)
{
    constexpr std::uint64_t NUMBER_OF_TIMES_COPIED = 1;

    std::uint64_t size = MCAP_ATTACHMENT_OVERHEAD;
    size += payload_size;
    size *= NUMBER_OF_TIMES_COPIED;

    return size;
}

std::uint64_t McapSizeTracker::get_metadata_size_(
        const mcap::Metadata& metadata)
{
    constexpr std::uint64_t NUMBER_OF_TIMES_COPIED = 1;

    std::uint64_t size = MCAP_METADATA_OVERHEAD;
    size += metadata.name.size();
    size += mcap::internal::KeyValueMapSize(metadata.metadata);
    size += metadata.name.size(); // metadata index
    size *= NUMBER_OF_TIMES_COPIED;

    return size;
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
