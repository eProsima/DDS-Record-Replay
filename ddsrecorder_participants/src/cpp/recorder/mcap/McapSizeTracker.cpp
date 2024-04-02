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

#include <ddsrecorder_participants/recorder/mcap/McapSizeTracker.hpp>

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
        const std::string& path)
{
    // logInfo

    if (enabled_)
    {
        // logWarning
        return;
    }

    disk_full_ = false;

    // TODO: add check that space is greater than

    potential_mcap_size_ = MCAP_FILE_OVERHEAD;
    written_mcap_size_ = MCAP_FILE_OVERHEAD;

    // Check available space in disk when opening file
    std::filesystem::space_info space = std::filesystem::space(path);
    space_available_ = space.available;
    // space_available_ = 5000; // falla attachment
    // space_available_ = 10000; // falla channel
    // space_available_ = 15000;

    enabled_ = true;
}

void McapSizeTracker::reset(
        const std::string& filepath)
{
    // logInfo

    if (!enabled_)
    {
        // logWarning
        return;
    }

    enabled_ = false;

    if (written_mcap_size_ > space_available_)
    {
        // logWarning();
        // assert(false);
    }
    else if (potential_mcap_size_ != written_mcap_size_)
    {
        // logWarning();
        // assert(false);
    }

    // std::cout << "ESTIMATED SIZE: " << written_mcap_size_ << std::endl;

    std::filesystem::path p{filepath};
    auto written_size = std::filesystem::file_size(p);
    // std::cout << "The size of " << p.u8string() << " is " <<
    //     written_size << " bytes.\n";

    if (written_mcap_size_ != written_size)
    {
        // logWarning();
        // assert(false);
    }
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
    check_and_increase_potential_mcap_size_(get_schema_size_(schema));
}

void McapSizeTracker::schema_written(
        const mcap::Schema& schema)
{
    check_and_increase_written_mcap_size_(get_schema_size_(schema));
}

void McapSizeTracker::channel_to_write(
        const mcap::Channel& channel)
{
    check_and_increase_potential_mcap_size_(get_channel_size_(channel));
}

void McapSizeTracker::channel_written(
        const mcap::Channel& channel)
{
    check_and_increase_written_mcap_size_(get_channel_size_(channel));
}

void McapSizeTracker::attachment_to_write(
        const uint64_t& payload_size)
{
    check_and_increase_potential_mcap_size_(get_attachment_size_(payload_size));
}

void McapSizeTracker::attachment_to_write(
        const uint64_t& payload_size_to_write,
        const uint64_t& payload_size_to_remove)
{
    if (can_increase_potential_mcap_size_(get_attachment_size_(payload_size_to_write), get_attachment_size_(payload_size_to_remove)))
    {
        decrease_potential_mcap_size_(get_attachment_size_(payload_size_to_remove));
        attachment_to_write(payload_size_to_write);
    }
    else
    {
        // TODO: throw directly if disk_full set to true?
        throw std::overflow_error(
                    STR_ENTRY << "Attempted to write an MCAP of size: " << potential_mcap_size_ <<
                ", but there is not enough space available on disk: " << space_available_);
    }
}

void McapSizeTracker::attachment_written(
        const uint64_t& payload_size)
{
    check_and_increase_written_mcap_size_(get_attachment_size_(payload_size));
}

void McapSizeTracker::metadata_to_write(
        const mcap::Metadata& metadata)
{
    check_and_increase_potential_mcap_size_(get_metadata_size_(metadata));
}

void McapSizeTracker::metadata_written(
        const mcap::Metadata& metadata)
{
    check_and_increase_written_mcap_size_(get_metadata_size_(metadata));
}

bool McapSizeTracker::can_increase_potential_mcap_size_(
        const std::uint64_t& size)
{
    if (!enabled_ || disk_full_)
    {
        // logWarning
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
        // logWarning
        return false;
    }

    if (potential_mcap_size_ < size_to_remove)
    {
        // logWarning
        assert(false);
        return false;
    }
    else
    {
        return (potential_mcap_size_ - size_to_remove + size_to_write) <= space_available_;
    }
}

void McapSizeTracker::check_and_increase_potential_mcap_size_(
        const std::uint64_t& size)
{
    if (!enabled_)
    {
        // logWarning
        return;
    }

    if (!disk_full_)
    {
        if (can_increase_potential_mcap_size_(size))
        {
            potential_mcap_size_ += size;
        }
        else
        {
            disk_full_ = true;
            throw std::overflow_error(
                        STR_ENTRY << "Attempted to write an MCAP of size: " << potential_mcap_size_ <<
                    ", but there is not enough space available on disk: " << space_available_);
        }
    }
    else
    {
        logInfo(DDSRECORDER_MCAP_HANDLER,
                "Disk is full, adding data previously taken into account");
        // assert((potential_mcap_size_ + size) <= space_available_);

        // assert(false); // this should not happen // throw exception?
        throw std::overflow_error(
                    STR_ENTRY << "Attempted to write an MCAP of size: " << potential_mcap_size_ <<
                ", but there is not enough space available on disk: " << space_available_);
    }
}

void McapSizeTracker::decrease_potential_mcap_size_(
        const std::uint64_t& size)
{
    if (!enabled_)
    {
        // logWarning
        return;
    }

    if (potential_mcap_size_ < size)
    {
        // logWarning();
        assert(false);  // throw exception?
    }
    else
    {
        potential_mcap_size_ -= size;
    }
}

void McapSizeTracker::check_and_increase_written_mcap_size_(
        const std::uint64_t& size)
{
    if (!enabled_)
    {
        // logWarning
        return;
    }

    if ((written_mcap_size_ + size) > space_available_)
    {
        // logWarning();
        // assert(false);
    }
    else if ((written_mcap_size_ + size) > potential_mcap_size_)
    {
        // logWarning();
        // assert(false); // throw exception?
    }
    else
    {
        written_mcap_size_ += size;
    }
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
    // constexpr std::uint64_t NUMBER_OF_TIMES_COPIED = 2;
    constexpr std::uint64_t NUMBER_OF_TIMES_COPIED = 1;
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
    // constexpr int NUMBER_OF_TIMES_COPIED = 2;
    constexpr int NUMBER_OF_TIMES_COPIED = 1;

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
