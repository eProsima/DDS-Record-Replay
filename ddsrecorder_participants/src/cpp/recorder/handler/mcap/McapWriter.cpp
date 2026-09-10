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
 * @file McapWriter.cpp
 */

#include <mcap/internal.hpp>

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/Log.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/utils.hpp>

#include <ddsrecorder_participants/common/time_utils.hpp>
#include <ddsrecorder_participants/recorder/exceptions/FullDiskException.hpp>
#include <ddsrecorder_participants/recorder/exceptions/FullFileException.hpp>
#include <ddsrecorder_participants/recorder/message/McapMessage.hpp>
#include <ddsrecorder_participants/recorder/handler/mcap/McapWriter.hpp>
#include <ddsrecorder_participants/constants.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * @brief Whether \c entries a ";"-terminated concatenation of "<guid>:<partitions>;" already
 * contains \c entry.
 */
static bool contains_partition_entry(
        const std::string& entries,
        const std::string& entry)
{
    for (auto pos = entries.find(entry); pos != std::string::npos; pos = entries.find(entry, pos + 1))
    {
        if (pos == 0 || entries[pos - 1] == ';')
        {
            return true;
        }
    }

    return false;
}

McapWriter::McapWriter(
        const OutputSettings& configuration,
        const mcap::McapWriterOptions& mcap_configuration,
        std::shared_ptr<FileTracker>& file_tracker,
        const bool record_types)
    : BaseWriter(configuration, file_tracker, record_types, MIN_MCAP_SIZE)
    , mcap_configuration_(mcap_configuration)
{
}

void McapWriter::disable()
{
    BaseWriter::disable();

    // Clear the channels when disabling the writer so the old channels are not rewritten in every new file
    if (channels_ != nullptr)
    {
        channels_->clear();
    }
}

void McapWriter::set_channels(
        std::map<ddspipe::core::types::DdsTopic, mcap::Channel>& channels)
{
    std::lock_guard<std::mutex> lock(mutex_);

    channels_ = &channels;
}

void McapWriter::add_message_sourceguid(
        uint32_t sequence_number,
        const std::string source_guid)
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::string indx;

    auto source_guid_it = sequence_by_source_guid_index_.find(source_guid);
    if (source_guid_it == sequence_by_source_guid_index_.end())
    {
        indx = std::to_string(sequence_by_source_guid_index_.size());

        source_guid_by_sequence_index_[indx] = source_guid;
        sequence_by_source_guid_index_[source_guid] = indx;
    }
    else
    {
        indx = source_guid_it->second;
    }

    source_guid_by_sequence_[std::to_string(sequence_number)] = indx;
}

void McapWriter::update_dynamic_types(
        const std::string& dynamic_types)
{
    std::lock_guard<std::mutex> lock(mutex_);

    const auto& update_dynamic_types = [&]()
            {
                if (dynamic_types_.empty())
                {
                    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_WRITER,
                            "MCAP_WRITE | Setting the dynamic types payload to "
                            << utils::from_bytes(dynamic_types.length()) << ".");

                    size_tracker_.attachment_to_write(dynamic_types.length());
                }
                else
                {
                    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_WRITER,
                            "MCAP_WRITE | Updating the dynamic types payload from "
                            << utils::from_bytes(dynamic_types_.length()) << " to "
                            << utils::from_bytes(dynamic_types.length()) << ".");

                    size_tracker_.attachment_to_write(dynamic_types.length(), dynamic_types_.length());
                }
            };

    try
    {
        update_dynamic_types();
    }
    catch (const FullFileException& e)
    {
        try
        {
            on_file_full_nts_(e, size_tracker_.get_min_mcap_size());
            update_dynamic_types();
        }
        catch (const FullDiskException& e)
        {
            EPROSIMA_LOG_ERROR(DDSRECORDER_MCAP_HANDLER,
                    "FAIL_MCAP_WRITE | Disk is full. Error message:\n " << e.what());
            on_disk_full_();
        }
    }

    dynamic_types_ = dynamic_types;
    file_tracker_->set_current_file_size(size_tracker_.get_potential_mcap_size());
}

void McapWriter::open_new_file_nts_(
        const std::uint64_t min_file_size)
{
    try
    {
        file_tracker_->new_file(min_file_size);
    }
    catch (const std::invalid_argument& e)
    {
        throw FullDiskException(
                  "The minimum MCAP size (" + utils::from_bytes(min_file_size) + ") is greater than the maximum MCAP "
                  "size (" + utils::from_bytes(configuration_.resource_limits.max_file_size_) + ").");
    }

    const auto filename = file_tracker_->get_current_filename();
    const auto status = writer_.open(filename, mcap_configuration_);

    if (!status.ok())
    {
        const auto error_msg = "Failed to open MCAP file " + filename + " for writing: " + status.message;

        EPROSIMA_LOG_ERROR(DDSRECORDER_MCAP_WRITER,
                "FAIL_MCAP_OPEN | " << error_msg);
        throw utils::InitializationException(error_msg);
    }

    // Set the file's maximum size
    const auto max_file_size = std::min(
        configuration_.resource_limits.max_file_size_,
        configuration_.resource_limits.max_size_ - file_tracker_->get_total_size());

    size_tracker_.init(max_file_size, configuration_.resource_limits.size_tolerance_,
            file_tracker_->get_current_filename());

    // NOTE: These writes should never fail since the minimum size accounts for them.
    write_metadata_version_nts_();
    write_schemas_nts_();
    reset_channel_partitions_nts_();

    if (record_types_ && dynamic_types_.length() > 0)
    {
        size_tracker_.attachment_to_write(dynamic_types_.length());
    }

    file_tracker_->set_current_file_size(size_tracker_.get_potential_mcap_size());
}

void McapWriter::close_current_file_nts_()
{
    if (record_types_ && dynamic_types_.length() > 0)
    {
        // NOTE: This write should never fail since the minimum size accounts for it.
        write_attachment_nts_();
        // Add the metadata dictionaries of source_guid messages
        write_metadata_messages_nts_(VERSION_METADATA_MESSAGE_NAME, source_guid_by_sequence_);
        write_metadata_messages_nts_(VERSION_METADATA_MESSAGE_INDEX_NAME, source_guid_by_sequence_index_);
        // Clear the dictionaries (resource-limits)
        source_guid_by_sequence_.clear();
        sequence_by_source_guid_index_.clear();
    }

    file_tracker_->set_current_file_size(size_tracker_.get_written_mcap_size());
    size_tracker_.reset();

    writer_.close();
    file_tracker_->close_file();
}

template<>
void McapWriter::write_nts_(
        const mcap::Attachment& attachment)
{
    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_WRITER,
            "MCAP_WRITE | Writing attachment: " << attachment.name << " (" << utils::from_bytes(attachment.dataSize)
                                                << ").");

    // NOTE: There is no need to check if the MCAP is full, since it is checked when adding a new dynamic_type.
    const auto status = writer_.write(const_cast<mcap::Attachment&>(attachment));

    if (!status.ok())
    {
        EPROSIMA_LOG_ERROR(DDSRECORDER_MCAP_WRITER,
                "MCAP_WRITE | Error writing in MCAP. Error message: " << status.message);
        return;
    }

    size_tracker_.attachment_written(attachment.dataSize);
    file_tracker_->set_current_file_size(size_tracker_.get_potential_mcap_size());
}

template<>
void McapWriter::write_nts_(
        const mcap::Channel& channel)
{
    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_WRITER,
            "MCAP_WRITE | Writing channel " << channel.topic << ".");

    size_tracker_.channel_to_write(channel);
    writer_.addChannel(const_cast<mcap::Channel&>(channel));
    size_tracker_.channel_written(channel);

    file_tracker_->set_current_file_size(size_tracker_.get_potential_mcap_size());

    // Ideally, the channels and schemas should be shared between the McapHandler and McapWriter.
    // Right now, the data is duplicated in both classes, which uses more memory and can lead to inconsistencies.
    // TODO: Share the channels and schemas between the McapHandler and McapWriter.
}

template<>
void McapWriter::write_nts_(
        const McapMessage& msg)
{
    if (!enabled_)
    {
        EPROSIMA_LOG_WARNING(DDSRECORDER_MCAP_WRITER,
                "MCAP_WRITE | Attempting to write a message in a disabled writer.");
        return;
    }

    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_WRITER, "Writing message: " << utils::from_bytes(msg.dataSize) << ".");

    // Record this sample's writer and partitions first, so the channel it is written on already
    // describes where the sample came from. This also writes the topic's channel into the file the
    // first time the file sees a sample for it.
    ensure_channel_partitions_nts_(msg);

    // Resolve the channel from the sample's own topic rather than trusting the id the sample was
    // stamped with. Channel ids are assigned per file, and a sample can be written into a later
    // file than the one that was open when it was created a buffered sample when the file
    // rotates, or a pending sample waiting for its schema. Its stamped id would then name a
    // different channel, or none at all.
    mcap::Message to_write = msg;

    if (channels_ != nullptr)
    {
        const auto it = channels_->find(msg.topic);

        if (it != channels_->end())
        {
            to_write.channelId = it->second.id;
        }
    }

    size_tracker_.message_to_write(msg.dataSize);
    const auto status = writer_.write(to_write);

    if (!status.ok())
    {
        EPROSIMA_LOG_ERROR(DDSRECORDER_MCAP_WRITER,
                "MCAP_WRITE | Error writing in MCAP. Error message: " << status.message);
        return;
    }

    size_tracker_.message_written(msg.dataSize);
    file_tracker_->set_current_file_size(size_tracker_.get_potential_mcap_size());
}

template<>
void McapWriter::write_nts_(
        const mcap::Metadata& metadata)
{
    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_WRITER,
            "MCAP_WRITE | Writing metadata: " << metadata.name << ".");

    size_tracker_.metadata_to_write(metadata);
    const auto status = writer_.write(metadata);

    if (!status.ok())
    {
        EPROSIMA_LOG_ERROR(DDSRECORDER_MCAP_WRITER,
                "MCAP_WRITE | Error writing in MCAP. Error message: " << status.message);
        return;
    }

    size_tracker_.metadata_written(metadata);
    file_tracker_->set_current_file_size(size_tracker_.get_potential_mcap_size());
}

template<>
void McapWriter::write_nts_(
        const mcap::Schema& schema)
{
    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_WRITER,
            "MCAP_WRITE | Writing schema: " << schema.name << ".");

    size_tracker_.schema_to_write(schema);
    writer_.addSchema(const_cast<mcap::Schema&>(schema));
    size_tracker_.schema_written(schema);

    file_tracker_->set_current_file_size(size_tracker_.get_potential_mcap_size());

    // Store the schema to write it on new MCAP files
    schemas_[schema.id] = schema;
}

void McapWriter::write_attachment_nts_()
{
    mcap::Attachment attachment;

    // Write down the attachment with the dynamic types and guids dictionary
    attachment.name = DYNAMIC_TYPES_ATTACHMENT_NAME;
    attachment.data = reinterpret_cast<std::byte*>(const_cast<char*>(dynamic_types_.c_str()));
    attachment.dataSize = dynamic_types_.length();
    attachment.createTime = to_mcap_timestamp(utils::now());

    write_nts_(attachment);
}

void McapWriter::reset_channel_partitions_nts_()
{
    if (channels_ == nullptr)
    {
        return;
    }

    for (auto& [_, channel] : *channels_)
    {
        channel.metadata[PARTITIONS] = "";
    }
}

void McapWriter::ensure_channel_partitions_nts_(
        const McapMessage& msg)
{
    if (channels_ == nullptr)
    {
        return;
    }

    const auto it = channels_->find(msg.topic);

    if (it == channels_->end())
    {
        return;
    }

    auto& channel = it->second;
    const std::string entry = msg.writer_guid_string + ":" + msg.partitions + ";";
    const auto metadata_it = channel.metadata.find(PARTITIONS);
    const std::string recorded =
            metadata_it != channel.metadata.end() ? metadata_it->second : std::string();

    if (contains_partition_entry(recorded, entry))
    {
        return;
    }

    // The metadata carries the writer GUID as well as its partitions, so a new writer needs a new
    // version of the channel even when it publishes in the same partitions as an existing one
    // otherwise its GUID is lost from the file. Channel metadata is immutable once written.
    //
    // Entries accumulate within a file so that a sample always lands on a version that describes
    // its own writer, and reset_channel_partitions_nts_ empties them again on the next file.
    auto metadata = channel.metadata;
    metadata[PARTITIONS] = recorded + entry;

    mcap::Channel new_channel(channel.topic, channel.messageEncoding, channel.schemaId, metadata);

    write_nts_(new_channel);

    channel = new_channel;
}

void McapWriter::write_metadata_version_nts_()
{
    mcap::Metadata metadata;

    // Write down the metadata with the version
    metadata.name = VERSION_METADATA_NAME;
    metadata.metadata[VERSION_METADATA_RELEASE] = DDSRECORDER_PARTICIPANTS_VERSION_STRING;
    metadata.metadata[VERSION_METADATA_COMMIT] = DDSRECORDER_PARTICIPANTS_COMMIT_HASH;

    write_nts_(metadata);
}

void McapWriter::write_metadata_messages_nts_(
        const std::string metadata_name,
        const mcap::KeyValueMap map)
{
    mcap::Metadata metadata;

    // Write down the metadata with the version
    metadata.name = metadata_name;
    for (const auto& pair_message: map)
    {
        metadata.metadata[pair_message.first] = pair_message.second;
    }

    write_nts_(metadata);
}

void McapWriter::write_schemas_nts_()
{
    if (schemas_.empty())
    {
        return;
    }

    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_WRITER,
            "MCAP_WRITE | Writing received schemas.");

    // Write schemas to MCAP file
    for (const auto& [_, schema] : schemas_)
    {
        write_nts_(schema);
    }
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
