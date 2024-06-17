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
#include <functional>
#include <mutex>

#include <mcap/mcap.hpp>

#include <fastdds/rtps/common/SerializedPayload.h>

#include <ddsrecorder_participants/constants.hpp>
#include <ddsrecorder_participants/library/library_dll.h>
#include <ddsrecorder_participants/recorder/mcap/McapHandlerConfiguration.hpp>
#include <ddsrecorder_participants/recorder/mcap/McapSizeTracker.hpp>
#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>
#include <ddsrecorder_participants/recorder/output/FullFileException.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

class DDSRECORDER_PARTICIPANTS_DllAPI McapWriter
{
public:

    McapWriter(
            const OutputSettings& configuration,
            const mcap::McapWriterOptions& mcap_configuration,
            std::shared_ptr<FileTracker>& file_tracker,
            const bool record_types = true);

    ~McapWriter();

    /**
     * @brief Enable the writer.
     *
     * After a \c FullFileException :
     * - @throws \c InconsistencyException if the allocated space is not enough to close the current file or to open a
     * new one.
     * - @throws \c InitializationException if the MCAP library fails to open a new file.
     */
    void enable();

    /**
     * @brief Disable the writer.
     *
     * After a \c FullFileException :
     * - @throws \c InconsistencyException if the allocated space is not enough to close the current file or to open a
     * new one.
     */
    void disable();

    /**
     * @brief Writes data to the MCAP file.
     *
     * @param data Pointer to the data to be written.
     *
     * After a \c FullFileException :
     * - @throws \c InconsistencyException if the allocated space is not enough to close the current file or to open a
     * new one.
     * - @throws \c InitializationException if the MCAP library fails to open a new file.
     */
    template <typename T>
    void write(
            const T& data);

    /**
     * @brief Updates the dynamic types payload.
     *
     * The dynamic types payload is written down as an attachment when the MCAP file is being closed.
     *
     * @param dynamic_types_payload The dynamic types payload to be written.
     *
     * After a \c FullFileException :
     * - @throws \c InconsistencyException if the allocated space is not enough to close the current file or to open a
     * new one.
     * - @throws \c InitializationException if the MCAP library fails to open a new file.
     */
    void update_dynamic_types(
            const fastrtps::rtps::SerializedPayload_t& dynamic_types_payload);

    /**
     * @brief Sets the callback to be called when the disk is full.
     *
     * Sets \c on_disk_full_lambda_ to \c on_disk_full_lambda.
     */
    void set_on_disk_full_callback(
            std::function<void()> on_disk_full_lambda) noexcept;

protected:

    /**
     * @brief Opens a new file.
     *
     * @param min_file_size The minimum size of the file.
     * @throws \c FullDiskException if the disk is full.
     * @throws \c InconsistencyException if \c min_file_size is not enough to write the: metadata, schemas, channels,
     * and attachment.
     * @throws \c InitializationException if the MCAP library fails to open the new file.
     */
    void open_new_file_nts_(
            const std::uint64_t min_file_size);

    /**
     * @brief Closes the current file.
     *
     * @throws \c InconsistencyException if there is not enough space to write the attachment.
     */
    void close_current_file_nts_();

    /**
     * @brief Writes data to the MCAP file.
     *
     * @param data The data to be written.
     * @throws \c FullFileException if the MCAP file is full.
     */
    template <typename T>
    void write_nts_(
            const T& data);

    /**
     * @brief Writes the attachment to the MCAP file.
     *
     * The attachment is written down as a message with the attachment data.
     * The size of the attachment is allocated by calling \c update_dynamic_types.
     *
     * @throws \c FullFileException if the MCAP file is full.
     */
    void write_attachment_nts_();

    /**
     * @brief Writes the channels to the MCAP file.
     *
     * @throws \c FullFileException if the MCAP file is full.
     */
    void write_channels_nts_();

    /**
     * @brief Writes the metadata to the MCAP file.
     *
     * @throws \c FullFileException if the MCAP file is full.
     */
    void write_metadata_nts_();

    /**
     * @brief Writes the schemas to the MCAP file.
     *
     * @throws \c FullFileException if the MCAP file is full.
     */
    void write_schemas_nts_();

    /**
     * @brief Function called when the MCAP file is full.
     *
     * The function closes the current file and opens a new one.
     *
     * @throws \c FullDiskException if the MCAP file is full.
     * @throws \c InconsistencyException if \c min_file_size is not enough to write the: metadata, schemas, channels,
     * and attachment.
     * @throws \c InitializationException if the MCAP library fails to open the new file.
     */
    void on_mcap_full_nts_(
            const FullFileException& e);

    /**
     * @brief Function called when the disk is full.
     */
    void on_disk_full_() const noexcept;

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

    //! Lambda to call when the disk is full
    std::function<void()> on_disk_full_lambda_;

    // The size of an MCAP file only with metadata and an empty attachment
    static constexpr std::uint64_t MIN_MCAP_SIZE = 2056;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */

#include <ddsrecorder_participants/recorder/mcap/impl/McapWriter.ipp>
