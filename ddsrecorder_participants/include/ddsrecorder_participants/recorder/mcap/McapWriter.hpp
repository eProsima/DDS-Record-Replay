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
 * @file McapWriter.hpp
 */

#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include <mcap/mcap.hpp>

#include <ddsrecorder_participants/constants.hpp>
#include <ddsrecorder_participants/library/library_dll.h>
#include <ddsrecorder_participants/recorder/mcap/McapFullException.hpp>
#include <ddsrecorder_participants/recorder/mcap/McapHandlerConfiguration.hpp>
#include <ddsrecorder_participants/recorder/mcap/McapSizeTracker.hpp>
#include <ddsrecorder_participants/recorder/mcap/Message.hpp>
#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

class McapWriter
{
public:

    DDSRECORDER_PARTICIPANTS_DllAPI
    McapWriter(
            const OutputSettings& configuration,
            const mcap::McapWriterOptions& mcap_configuration,
            std::shared_ptr<FileTracker>& file_tracker,
            const bool record_types = true);

    DDSRECORDER_PARTICIPANTS_DllAPI
    ~McapWriter();

    /**
     * @brief Enable the writer.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void enable();

    /**
     * @brief Disable the writer.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void disable();

    /**
     * @brief Writes data to the MCAP file.
     * @param data Pointer to the data to be written.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    template <typename T>
    void write(
            const T& data);

    /**
     * @brief Updates the dynamic types payload.
     *
     * The dynamic types payload is written down as an attachment when the MCAP file is being closed.
     *
     * @param dynamic_types_payload The dynamic types payload to be written.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void update_dynamic_types(
            const fastrtps::rtps::SerializedPayload_t& dynamic_types_payload);

protected:

    // Open a new file
    void open_new_file_nts_(
            const std::uint64_t min_file_size);

    // Close the current file
    void close_current_file_nts_();

    // Writes data to the MCAP file.
    template <typename T>
    void write_nts_(
            const T& data);

    // Write the attachment
    void write_attachment_nts_();

    // Write the previous channels
    void write_channels_nts_();

    // Write the metadata
    void write_metadata_nts_();

    // Write the previous schemas
    void write_schemas_nts_();

    // Callback when the MCAP library is full
    void on_mcap_full_nts_(
            const McapFullException& e);

    // Callback when the MCAP library is full and a retry is required
    void on_mcap_full_nts_(
            const McapFullException& e,
            std::function<void()> retry);

    // The configuration for the class
    const OutputSettings configuration_;

    // The configuration for the MCAP library
    const mcap::McapWriterOptions mcap_configuration_;

    // Track the files written by the MCAP library
    std::shared_ptr<FileTracker> file_tracker_;

    // Whether to record the types
    const bool record_types_{false};

    // The mutex to protect the calls to write
    std::mutex mutex_;

    // Whether the writer can write to the MCAP library
    bool enabled_{false};

    // Track the size of the current MCAP file
    McapSizeTracker size_tracker_;

    // The writer from the MCAP library
    mcap::McapWriter writer_;

    // The dynamic types payload to be written as an attachment
    std::unique_ptr<fastrtps::rtps::SerializedPayload_t> dynamic_types_payload_;

    // The channels that have been written
    std::map<mcap::ChannelId, mcap::Channel> channels_;

    // The schemas that have been written
    std::map<mcap::SchemaId, mcap::Schema> schemas_;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */

#include <ddsrecorder_participants/recorder/mcap/impl/McapWriter.ipp>
