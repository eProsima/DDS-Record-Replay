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
 * @file McapSizeTracker.cpp
 */

#include <cstdio>

#include <ddsrecorder_participants/recorder/size_tracker/McapSizeTracker.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

McapSizeTracker::McapSizeTracker()
{
}

McapSizeTracker::~McapSizeTracker()
{
}

std::uint64_t McapSizeTracker::get_message_size(
        const uint64_t& msg_datasize)
{
    constexpr std::uint64_t NUMBER_OF_TIMES_COPIED = 1;

    std::uint64_t size = MCAP_MESSAGE_OVERHEAD;
    size += msg_datasize;
    size *= NUMBER_OF_TIMES_COPIED;

    return size;
}

std::uint64_t McapSizeTracker::get_schema_size(
        const mcap::Schema& schema)
{
    constexpr std::uint64_t NUMBER_OF_TIMES_COPIED = 2;
    constexpr std::uint64_t CONST_SCHEMA = 5;

    std::uint64_t size = MCAP_SCHEMAS_OVERHEAD;
    size += schema.name.size();
    size += schema.encoding.size();
    size += schema.data.size();
    size *= NUMBER_OF_TIMES_COPIED;

    size -= CONST_SCHEMA;

    return size;
}

std::uint64_t McapSizeTracker::get_blank_schema_size(
        const bool& ros2_types,
        const std::string& schema_name)
{
    constexpr int NUMBER_OF_TIMES_COPIED = 2;
    constexpr std::uint64_t CONST_SCHEMA = 5;

    std::string encoding = ros2_types ? "ros2msg" : "omgidl";

    std::uint64_t size = MCAP_SCHEMAS_OVERHEAD;
    size += schema_name.size();
    size += encoding.size();
    size *= NUMBER_OF_TIMES_COPIED;
    size -= CONST_SCHEMA;

    return size;
}

std::uint64_t McapSizeTracker::get_channel_size(
        const mcap::Channel& channel,
        const uint64_t& size_metadata)
{
    constexpr int NUMBER_OF_TIMES_COPIED = 2;

    std::uint64_t size = MCAP_CHANNEL_OVERHEAD;
    size += channel.topic.size();
    size += channel.messageEncoding.size();
    size += size_metadata;
    size *= NUMBER_OF_TIMES_COPIED;

    return size;
}

std::uint64_t McapSizeTracker::get_blank_channel_size(
        const std::string& channel_name)
{
    constexpr std::uint64_t FIXED_SIZE = MCAP_CHANNEL_OVERHEAD + 103; //Encoding: 3 + Metadata:100
    constexpr int NUMBER_OF_TIMES_COPIED = 2;

    std::uint64_t size = FIXED_SIZE;
    size += channel_name.size();
    size *= NUMBER_OF_TIMES_COPIED;

    return size;
}

std::uint64_t McapSizeTracker::get_attachment_size(
        const std::unique_ptr<fastrtps::rtps::SerializedPayload_t>& dynamic_attachment_serialized_payload)
{
    constexpr std::uint64_t NUMBER_OF_TIMES_COPIED = 1;

    std::uint64_t size = MCAP_ATTACHMENT_OVERHEAD;
    size += dynamic_attachment_serialized_payload->length;
    size *= NUMBER_OF_TIMES_COPIED;

    return size;
}

void McapSizeTracker::decrease_mcap_size(
        const std::uint64_t& size)
{
    assert(mcap_size_ >= size);
    mcap_size_ -= size;
}

void McapSizeTracker::check_and_increase_mcap_size_final(
        const std::uint64_t& size)
{
    if (!disk_full_)
    {
        if ((mcap_size_ + size) > space_available_when_open_)
        {
            disk_full_ = true;
            throw std::overflow_error(
                        STR_ENTRY << "Attempted to write an MCAP of size: " << mcap_size_ <<
                    ", but there is not enough space available on disk: " << space_available_when_open_);
        }
        else
        {
            mcap_size_ += size;
        }
    }
    else
    {
        logInfo(DDSRECORDER_MCAP_HANDLER,
                "Disk is full, adding data previously taken into account");
        assert((mcap_size_ + size) <= space_available_when_open_);
    }
}

void McapSizeTracker::check_and_increase_mcap_size(
        const uint64_t& msg_datasize)
{
    // Calculate message size
    std::uint64_t size = get_message_size(msg_datasize);

    check_and_increase_mcap_size_final(size);
}

void McapSizeTracker::check_and_increase_mcap_size(
        const mcap::Schema& schema)
{
    // Calculate schema size
    std::uint64_t size = get_schema_size(schema);

    check_and_increase_mcap_size_final(size);
}

void McapSizeTracker::check_and_increase_mcap_size(
        const mcap::Channel& channel,
        const uint64_t& size_metadata)
{
    // Calculate channel size
    std::uint64_t size = get_channel_size(channel, size_metadata);

    check_and_increase_mcap_size_final(size);
}

void McapSizeTracker::check_available_space(
        const std::string& file_path)
{
    // Check available space in disk when opening file
        std::filesystem::space_info space = std::filesystem::space(file_path);
        space_available_when_open_ = 50000;
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
