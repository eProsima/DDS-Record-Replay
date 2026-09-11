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

#include <map>

#include <mcap/mcap.hpp>

#include <fastdds/rtps/common/SerializedPayload.hpp>

#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddsrecorder_participants/library/library_dll.h>
#include <ddsrecorder_participants/recorder/handler/mcap/McapSizeTracker.hpp>
#include <ddsrecorder_participants/recorder/message/McapMessage.hpp>
#include <ddsrecorder_participants/recorder/handler/BaseWriter.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Class that writes data to an MCAP file.
 *
 * It uses the MCAP library to write data to a file.
 * It tracks the size of the file and handles disk full exceptions.
 */
class DDSRECORDER_PARTICIPANTS_DllAPI McapWriter : public BaseWriter
{
public:

    /**
     * @brief Constructor
     *
     * @param configuration The output settings for the writer.
     * @param mcap_configuration The MCAP writer options.
     * @param file_tracker The file tracker to track the files written by the output library.
     * @param record_types Whether to record the types.
     */
    McapWriter(
            const OutputSettings& configuration,
            const mcap::McapWriterOptions& mcap_configuration,
            std::shared_ptr<FileTracker>& file_tracker,
            const bool record_types = true);

    /**
     * @brief Disable the writer.
     */
    void disable() override;

    /**
     * @brief Writes data to the output file.
     *
     * @param data Pointer to the data to be written.
     *
     * After a \c FullFileException :
     * - @throws \c InconsistencyException if the allocated space is not enough to close the current file or to open a
     * new one.
     * - @throws \c InitializationException if the MCAP library fails to open a new file.
     */
    template<typename T>
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
            const std::string& dynamic_types_payload);

    /**
     * @brief Share the handler's topic-to-channel map with the writer.
     *
     * The writer needs the map for two things: to know which channels to re-create when it opens a
     * new file, and to resolve the channel a sample belongs to when the sample is written. It is
     * the handler's map the writer keeps no channel collection of its own, so a channel version
     * that the handler has superseded cannot survive in the writer and be re-written forever.
     *
     * Must be called before the writer is enabled.
     *
     * @param channels The handler's channels, keyed by topic.
     */
    void set_channels(
            std::map<ddspipe::core::types::DdsTopic, mcap::Channel>& channels);

    /**
     * @brief Adds the pair sequence_number, source guid in the dictionary.
     *
     * @param sequence_number The sequence number associated with a message.
     * @param source_guid The guid associated with a message.
     *
     * After a \c FullFileException :
     * - @throws \c InconsistencyException if the allocated space is not enough to close the current file or to open a
     * new one.
     * - @throws \c InitializationException if the MCAP library fails to open a new file.
     */
    void add_message_sourceguid(
            uint32_t sequence_number,
            const std::string source_guid);

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
            const std::uint64_t min_file_size) override;

    /**
     * @brief Closes the current file.
     *
     * @throws \c InconsistencyException if there is not enough space to write the attachment.
     */
    void close_current_file_nts_() override;

    /**
     * @brief Writes data to the MCAP file.
     *
     * @param data The data to be written.
     * @throws \c FullFileException if the MCAP file is full.
     */
    template<typename T>
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
     * @brief Mark every channel as not yet written to the file that has just been opened.
     *
     * Clears each channel's PARTITIONS metadata, which does two things. It scopes the partition
     * metadata to one file the entries are re-added, one per writer, as the file's own samples
     * are written and, because a cleared channel can never already contain a sample's entry, it
     * guarantees that the first sample of each topic re-writes that topic's channel into the new
     * file. Channels are therefore written lazily, on first use, so a file carries exactly the
     * channels its own messages need.
     */
    void reset_channel_partitions_nts_();

    /**
     * @brief Make sure the channel of \c msg 's topic records that its writer published in its
     * partitions, writing a new version of the channel if it does not say so yet.
     *
     * Called for every sample as it is written, which is after any file rotation the sample's own
     * write triggered so the entry lands in the file the sample is actually written to.
     *
     * @throws \c FullFileException if the MCAP file is full.
     */
    void ensure_channel_partitions_nts_(
            const McapMessage& msg);

    /**
     * @brief Writes the version metadata to the MCAP file.
     *
     * @throws \c FullFileException if the MCAP file is full.
     */
    void write_metadata_version_nts_();

    /**
     * @brief Writes the messages metadata to the MCAP file.
     *
     * @param metadata_name The name of the map in the metadata.
     * @param map The map that is going to be writed in the metadata.
     *
     * @throws \c FullFileException if the MCAP file is full.
     */
    void write_metadata_messages_nts_(
            const std::string metadata_name,
            const mcap::KeyValueMap map);

    /**
     * @brief Writes the schemas to the MCAP file.
     *
     * @throws \c FullFileException if the MCAP file is full.
     */
    void write_schemas_nts_();

    // The configuration for the MCAP library
    const mcap::McapWriterOptions mcap_configuration_;

    // Track the size of the current MCAP file
    McapSizeTracker size_tracker_;

    // The writer from the MCAP library
    mcap::McapWriter writer_;

    // The dynamic types payload to be written as an attachment
    std::string dynamic_types_;

    // The dictionary of sequence-guid
    mcap::KeyValueMap source_guid_by_sequence_;
    // The indexation dictionary for the source_guids
    mcap::KeyValueMap source_guid_by_sequence_index_;

    // The (Auxiliar) dictionary of guid-sequence
    mcap::KeyValueMap sequence_by_source_guid_index_;

    // The handler's channels, keyed by topic and holding one (the newest)
    std::map<ddspipe::core::types::DdsTopic, mcap::Channel>* channels_{nullptr};

    // The schemas that have been written
    std::map<mcap::SchemaId, mcap::Schema> schemas_;

    // The size of an empty MCAP file
    static constexpr std::uint64_t MIN_MCAP_SIZE{2056};
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */

#include <ddsrecorder_participants/recorder/handler/mcap/impl/McapWriter.ipp>
