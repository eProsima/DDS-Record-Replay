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
 * @file McapSizeTracker.hpp
 */

#pragma once

#include <cstdint>
#include <memory>

#include <mcap/mcap.hpp>

#include <fastdds/rtps/common/SerializedPayload.h>

#include <ddsrecorder_participants/library/library_dll.h>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Class that tracks the MCAP size during recorder execution.
 *
 */
class McapSizeTracker
{
public:

    /**
     * @brief Constructor
     *
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    McapSizeTracker();

    /**
     * @brief Destructor
     *
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    ~McapSizeTracker();

    /**
     * @brief Initialize the tracker with a given \c space_available and \c safety_margin.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void init(
            const std::uint64_t& space_available,
            const std::uint64_t& safety_margin);

    DDSRECORDER_PARTICIPANTS_DllAPI
    void reset(
            const std::string& filepath);

    DDSRECORDER_PARTICIPANTS_DllAPI
    void message_to_write(
            const uint64_t& data_size);

    DDSRECORDER_PARTICIPANTS_DllAPI
    void message_written(
            const uint64_t& data_size);

    DDSRECORDER_PARTICIPANTS_DllAPI
    void schema_to_write(
            const mcap::Schema& schema);

    DDSRECORDER_PARTICIPANTS_DllAPI
    void schema_written(
            const mcap::Schema& schema);

    DDSRECORDER_PARTICIPANTS_DllAPI
    void channel_to_write(
            const mcap::Channel& channel);

    DDSRECORDER_PARTICIPANTS_DllAPI
    void channel_written(
            const mcap::Channel& channel);

    DDSRECORDER_PARTICIPANTS_DllAPI
    void attachment_to_write(
            const uint64_t& payload_size);

    DDSRECORDER_PARTICIPANTS_DllAPI
    void attachment_to_write(
            const uint64_t& payload_size_to_write,
            const uint64_t& payload_size_to_remove);

    DDSRECORDER_PARTICIPANTS_DllAPI
    void attachment_written(
            const uint64_t& payload_size);

    DDSRECORDER_PARTICIPANTS_DllAPI
    void metadata_to_write(
            const mcap::Metadata& metadata);

    DDSRECORDER_PARTICIPANTS_DllAPI
    void metadata_written(
            const mcap::Metadata& metadata);

    DDSRECORDER_PARTICIPANTS_DllAPI
    std::uint64_t get_potential_mcap_size() const;

    DDSRECORDER_PARTICIPANTS_DllAPI
    std::uint64_t get_written_mcap_size() const;

    DDSRECORDER_PARTICIPANTS_DllAPI
    std::uint64_t get_min_mcap_size() const;

protected:

    bool can_increase_potential_mcap_size_(
            const std::uint64_t& size);

    bool can_increase_potential_mcap_size_(
            const std::uint64_t& size_to_write,
            const std::uint64_t& size_to_remove);

    void check_and_increase_potential_mcap_size_(
            const std::uint64_t& size,
            const bool increase_min_mcap_size = false);

    void decrease_potential_mcap_size_(
            const std::uint64_t& size,
            const bool decrease_min_mcap_size = false);

    void check_and_increase_written_mcap_size_(
            const std::uint64_t& size);

    /**
     * @brief Get space needed to write message
     *
     */
    std::uint64_t get_message_size_(
            const uint64_t& data_size);

    /**
     * @brief Get space needed to write schema
     *
     */
    std::uint64_t get_schema_size_(
            const mcap::Schema& schema);

    /**
     * @brief Get space needed to write channel
     *
     */
    std::uint64_t get_channel_size_(
            const mcap::Channel& channel);

    /**
     * @brief Get space needed to write attachment
     *
     */
    std::uint64_t get_attachment_size_(
            const std::uint64_t& payload_size);

    /**
     * @brief Get space needed to write metadata
     *
     */
    std::uint64_t get_metadata_size_(
            const mcap::Metadata& metadata);

    //! Potential (estimated) file size, that takes into account objects to be written (not yet written)
    std::uint64_t potential_mcap_size_{MCAP_FILE_OVERHEAD}; // TODO: move initialization to init, and also set disk_full_ to false

    //! Written (estimated) file size, that takes into account written objects
    std::uint64_t written_mcap_size_{MCAP_FILE_OVERHEAD};

    //! The minimum size of an MCAP file without data
    std::uint64_t min_mcap_size_{MCAP_FILE_OVERHEAD};

    //! Space available in disk
    std::uintmax_t space_available_;

    //! Boolean that describes if disk is full or not
    bool disk_full_;

    bool enabled_ = false;

    //! MCAP file overhead
    /**
     * To reach this number, we use the following constants:
     *   - Header + Write Header = 18
     *   - Write ChunkIndex = 73
     *   - Write Statistics = 55
     *   - Write DataEnd + Write SummaryOffSets = 13 + 26*6
     */
    static constexpr std::uint64_t MCAP_FILE_OVERHEAD{315};

    //! Additional overhead size for a MCAP message
    static constexpr std::uint64_t MCAP_MESSAGE_OVERHEAD{31 + 8 + 8}; // Write Message + TimeStamp + TimeOffSet

    //! Additional overhead size for a MCAP schema
    static constexpr std::uint64_t MCAP_SCHEMA_OVERHEAD{23}; // Write Schema

    //! Additional overhead size for a MCAP channel
    static constexpr std::uint64_t MCAP_CHANNEL_OVERHEAD{25 + 10 + 10}; // Write Channel + messageIndexOffsetsSize + channelMessageCountsSize

    //! Additional overhead size for a MCAP attachment
    static constexpr std::uint64_t MCAP_ATTACHMENT_OVERHEAD{58 + 70}; // Write Attachment + Write AttachmentIndex

    //! Additional overhead size for a MCAP metadata
    static constexpr std::uint64_t MCAP_METADATA_OVERHEAD{17 + 29}; // Write Metadata + Write MetadataIndex

};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
