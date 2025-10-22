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

namespace eprosima {
namespace ddsrecorder {
namespace participants {

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
    channels_.clear();
}

void McapWriter::add_message_sourceguid(
        uint32_t sequence_number,
        const std::string source_guid)
{
    std::lock_guard<std::mutex> lock(mutex_);

    const auto& add_sourceguid_pair = [&]()
            {
                uint32_t pair_length = sequence_number + source_guid.length();

                EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_WRITER,
                        "MCAP_WRITE | Adding a pair (sequence number, guid) payload " <<
                        utils::from_bytes(pair_length) << ".");

                size_tracker_.attachment_to_write(pair_length);
            };

    try
    {
        add_sourceguid_pair();
    }
    catch (const FullFileException& e)
    {
        try
        {
            on_file_full_nts_(e, size_tracker_.get_min_mcap_size());
            add_sourceguid_pair();
        }
        catch (const FullDiskException& e)
        {
            EPROSIMA_LOG_ERROR(DDSRECORDER_MCAP_HANDLER,
                    "FAIL_MCAP_WRITE | Disk is full. Error message:\n " << e.what());
            on_disk_full_();
        }
    }

    sourceguid_by_sequence_[std::to_string(sequence_number)] = source_guid;
    file_tracker_->set_current_file_size(size_tracker_.get_potential_mcap_size());
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
                            "MCAP_WRITE | Setting the dynamic types payload to " <<
                            utils::from_bytes(dynamic_types.length()) << ".");

                    size_tracker_.attachment_to_write(dynamic_types.length());
                }
                else
                {
                    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_WRITER,
                            "MCAP_WRITE | Updating the dynamic types payload from " <<
                            utils::from_bytes(dynamic_types_.length()) << " to " <<
                            utils::from_bytes(dynamic_types.length()) << ".");

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
    write_channels_nts_();

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
        write_metadata_messages_nts_();
    }

    file_tracker_->set_current_file_size(size_tracker_.get_written_mcap_size());
    size_tracker_.reset();

    writer_.close();
    file_tracker_->close_file();
}

template <>
void McapWriter::write_nts_(
        const mcap::Attachment& attachment)
{
    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_WRITER,
            "MCAP_WRITE | Writing attachment: " << attachment.name << " (" << utils::from_bytes(attachment.dataSize) <<
            ").");

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

template <>
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

    // Store the channel to write it on new MCAP files
    channels_[channel.id] = channel;
}

template <>
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

    size_tracker_.message_to_write(msg.dataSize);
    const auto status = writer_.write(msg);

    if (!status.ok())
    {
        EPROSIMA_LOG_ERROR(DDSRECORDER_MCAP_WRITER,
                "MCAP_WRITE | Error writing in MCAP. Error message: " << status.message);
        return;
    }

    size_tracker_.message_written(msg.dataSize);
    file_tracker_->set_current_file_size(size_tracker_.get_potential_mcap_size());
}

template <>
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

template <>
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

void McapWriter::write_channels_nts_()
{
    if (channels_.empty())
    {
        return;
    }

    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_WRITER,
            "MCAP_WRITE | Writing received channels.");

    // Write channels to MCAP file
    for (const auto& [_, channel] : channels_)
    {
        write_nts_(channel);
    }
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

void McapWriter::write_metadata_messages_nts_()
{
    mcap::Metadata metadata;

    // Write down the metadata with the version
    metadata.name = VERSION_METADATA_MESSAGE_NAME;
    for (const auto& pair_message: sourceguid_by_sequence_)
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
