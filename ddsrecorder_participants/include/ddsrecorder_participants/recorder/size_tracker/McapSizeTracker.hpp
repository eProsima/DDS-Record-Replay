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
 * @file McapSizeTracker.hpp
 */

#pragma once

#include <filesystem>
#include <stdexcept>

#include <mcap/mcap.hpp>

#include <cpp_utils/macros/custom_enumeration.hpp>

#include <fastrtps/types/DynamicTypePtr.h>

#include <ddspipe_participants/participant/dynamic_types/ISchemaHandler.hpp>

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
     * @brief Get space needed to write message
     *
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    std::uint64_t get_message_size(
            const uint64_t& msg_datasize);

    /**
     * @brief Get space needed to write schema
     *
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    std::uint64_t get_schema_size(
            const mcap::Schema& schema);

    /**
     * @brief Get space needed to write blank schema
     *
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    std::uint64_t get_blank_schema_size(
            const bool& ros2_types,
            const std::string& schema_name);

    /**
     * @brief Get space needed to write channel
     *
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    std::uint64_t get_channel_size(
            const mcap::Channel& channel,
            const uint64_t& size_metadata);

    /**
     * @brief Get space needed to write blank channel
     *
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    std::uint64_t get_blank_channel_size(
            const std::string& channel_name);

    /**
     * @brief Get space needed to write attachment
     *
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    std::uint64_t get_attachment_size(
            const std::unique_ptr<fastrtps::rtps::SerializedPayload_t>& serialized_payload);

    /**
     * @brief Check if there is enough space to write the current file
     *
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void decrease_mcap_size(
            const std::uint64_t& size);

    /**
     * @brief Check if there is enough space to write the current file
     *
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void check_and_increase_mcap_size_final(
            const std::uint64_t& size);

    /**
     * @brief Check if there is enough space to write the current file when adding a message
     *
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void check_and_increase_mcap_size(
            const uint64_t& msg_datasize);

    /**
     * @brief Check if there is enough space to write the current file when adding a schema
     *
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void check_and_increase_mcap_size(
            const mcap::Schema& schema);

    /**
     * @brief Check if there is enough space to write the current file when adding a channel
     *
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void check_and_increase_mcap_size(
            const mcap::Channel& channel,
            const uint64_t& size_metadata);

    /**
     * @brief Check available space on disk
     *
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void check_available_space(
        const std::string& file_path);

protected:

    //! Total file size
    std::uint64_t mcap_size_{MCAP_FILE_OVERHEAD}; // MCAP file size is initialized with MCAP_FILE_OVERHEAD

    //! Space available in disk
    std::uintmax_t space_available_when_open_;

    //! Boolean that describes if disk is full or not
    bool disk_full_ = false;

    //! MCAP file overhead
    /**
     * To reach this number, we use the following constants:
     *   - Header + Write Header = 18
     *   - Metadata + Write Metadata + Write MetadataIndex = 75 + 24 + 36
     *   - Write ChunkIndex = 73
     *   - Write Statistics = 55
     *   - Write DataEnd + Write SummaryOffSets = 13 + 26*6
     */
    static constexpr std::uint64_t MCAP_FILE_OVERHEAD{450};

    //! Additional overhead size for a MCAP message
    static constexpr std::uint64_t MCAP_MESSAGE_OVERHEAD{31 + 8 + 8}; // Write Message + TimeStamp + TimeOffSet

    //! Additional overhead size for a MCAP schema
    static constexpr std::uint64_t MCAP_SCHEMAS_OVERHEAD{23}; // Write Schemas

    //! Additional overhead size for a MCAP channel
    static constexpr std::uint64_t MCAP_CHANNEL_OVERHEAD{25 + 10 + 10}; // Write Channel + messageIndexOffsetsSize + channelMessageCountsSize

    //! Additional overhead size for a MCAP attachment
    static constexpr std::uint64_t MCAP_ATTACHMENT_OVERHEAD{58 + 70}; // Write Attachment + Write AttachmentIndex

};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
