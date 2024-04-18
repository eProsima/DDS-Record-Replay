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

#include <filesystem>
#include <stdexcept>

#include <mcap/internal.hpp>

#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/Formatter.hpp>
#include <cpp_utils/Log.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/utils.hpp>

#include <ddsrecorder_participants/recorder/mcap/McapFullException.hpp>
#include <ddsrecorder_participants/recorder/mcap/McapWriter.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

McapWriter::McapWriter(
        const OutputSettings& configuration,
        const mcap::McapWriterOptions& mcap_configuration,
        std::shared_ptr<FileTracker>& file_tracker,
        const bool record_types)
    : configuration_(configuration)
    , mcap_configuration_(mcap_configuration)
    , file_tracker_(file_tracker)
    , record_types_(record_types)
{
    enable();
}

McapWriter::~McapWriter()
{
    disable();
}

void McapWriter::enable()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (enabled_)
    {
        return;
    }

    logInfo(DDSRECORDER_MCAP_WRITER, "Enabling MCAP writer.")

    open_new_file_nts_(size_tracker_.get_min_mcap_size());

    enabled_ = true;
}

void McapWriter::disable()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!enabled_)
    {
        return;
    }

    logInfo(DDSRECORDER_MCAP_WRITER, "Disabling MCAP writer.")

    close_current_file_nts_();

    enabled_ = false;
}

void McapWriter::update_dynamic_types(
        const fastrtps::rtps::SerializedPayload_t& dynamic_types_payload)
{
    std::lock_guard<std::mutex> lock(mutex_);

    try
    {
        if (dynamic_types_payload_ == nullptr)
        {
            size_tracker_.attachment_to_write(dynamic_types_payload.length);
        }
        else
        {
            size_tracker_.attachment_to_write(dynamic_types_payload.length, dynamic_types_payload_->length);
        }
    }
    catch (const McapFullException& e)
    {
        on_mcap_full_nts_(e);
    }

    dynamic_types_payload_.reset(const_cast<fastrtps::rtps::SerializedPayload_t*>(&dynamic_types_payload));
    file_tracker_->set_current_file_size(size_tracker_.get_potential_mcap_size());
}

void McapWriter::open_new_file_nts_(
        const std::uint64_t min_file_size)
{
    try
    {
        file_tracker_->new_file(min_file_size);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Error creating new MCAP file: " + std::string(e.what()));
    }

    // Calculate the maximum size of the file
    const auto max_file_size = std::min(
        configuration_.max_file_size,
        configuration_.max_size - file_tracker_->get_total_size());

    size_tracker_.init(max_file_size, configuration_.safety_margin);

    const auto filename = file_tracker_->get_current_filename();
    const auto status = writer_.open(filename, mcap_configuration_);

    if (!status.ok())
    {
        throw std::runtime_error(
                  STR_ENTRY << "FAIL_MCAP_OPEN | Error opening MCAP file: " << filename << ", error message: " <<
                      status.message);
    }

    // NOTE: These writes will never fail since the minimum size accounts for them.
    write_metadata_nts_();
    write_schemas_nts_();
    write_channels_nts_();

    if (dynamic_types_payload_ != nullptr && record_types_)
    {
        // Recalculate the size of the attachment to update the min_mcap_size
        size_tracker_.attachment_to_write(dynamic_types_payload_->length);
    }

    file_tracker_->set_current_file_size(size_tracker_.get_potential_mcap_size());
}

void McapWriter::close_current_file_nts_()
{
    if (record_types_)
    {
        write_attachment_nts_();
    }

    file_tracker_->set_current_file_size(size_tracker_.get_written_mcap_size());
    size_tracker_.reset(file_tracker_->get_current_filename());

    file_tracker_->close_file();
    writer_.close();
}

template <>
void McapWriter::write_nts_(
        const mcap::Attachment& attachment)
{
    logInfo(DDSRECORDER_MCAP_WRITER,
            "Writing attachment: " << attachment.name << " (" << utils::from_bytes(attachment.dataSize) << ").");

    // NOTE: There is no need to check if the MCAP is full, since it is checked when adding a new dynamic_type.
    auto status = writer_.write(const_cast<mcap::Attachment&>(attachment));

    if (!status.ok())
    {
        throw utils::InconsistencyException(
                  STR_ENTRY << "FAIL_MCAP_WRITE | Error writting in MCAP, error message: " << status.message
                  );
    }

    size_tracker_.attachment_written(attachment.dataSize);
    file_tracker_->set_current_file_size(size_tracker_.get_potential_mcap_size());
}

template <>
void McapWriter::write_nts_(
        const mcap::Channel& channel)
{
    logInfo(DDSRECORDER_MCAP_WRITER, "Writing channel " << channel.topic << ".");

    try
    {
        size_tracker_.channel_to_write(channel);
    }
    catch (const McapFullException& e)
    {
        on_mcap_full_nts_(e);
    }

    writer_.addChannel(const_cast<mcap::Channel&>(channel));

    size_tracker_.channel_written(channel);
    file_tracker_->set_current_file_size(size_tracker_.get_potential_mcap_size());

    // Store the channel to write it down when the MCAP file is closed
    channels_[channel.id] = channel;
}

template <>
void McapWriter::write_nts_(
        const Message& msg)
{
    logInfo(DDSRECORDER_MCAP_WRITER, "Writing message: " << utils::from_bytes(msg.dataSize) << ".");

    try
    {
        size_tracker_.message_to_write(msg.dataSize);
    }
    catch (const McapFullException& e)
    {
        on_mcap_full_nts_(e, [&]()
                {
                    size_tracker_.message_to_write(msg.dataSize);
                });
    }

    const auto status = writer_.write(msg);

    if (!status.ok())
    {
        throw utils::InconsistencyException(
                  STR_ENTRY << "FAIL_MCAP_WRITE | Error writting in MCAP, error message: " << status.message
                  );
    }

    size_tracker_.message_written(msg.dataSize);
    file_tracker_->set_current_file_size(size_tracker_.get_potential_mcap_size());
}

template <>
void McapWriter::write_nts_(
        const mcap::Metadata& metadata)
{
    logInfo(DDSRECORDER_MCAP_WRITER, "Writing metadata: " << metadata.name << ".");

    try
    {
        size_tracker_.metadata_to_write(metadata);
    }
    catch (const McapFullException& e)
    {
        on_mcap_full_nts_(e);
    }

    const auto status = writer_.write(metadata);

    if (!status.ok())
    {
        throw utils::InconsistencyException(
                  STR_ENTRY << "FAIL_MCAP_WRITE | Error writting in MCAP, error message: " << status.message
                  );
    }

    size_tracker_.metadata_written(metadata);
    file_tracker_->set_current_file_size(size_tracker_.get_potential_mcap_size());
}

template <>
void McapWriter::write_nts_(
        const mcap::Schema& schema)
{
    logInfo(DDSRECORDER_MCAP_WRITER, "Writing schema: " << schema.name << ".");

    try
    {
        size_tracker_.schema_to_write(schema);
    }
    catch (const McapFullException& e)
    {
        on_mcap_full_nts_(e);
    }

    writer_.addSchema(const_cast<mcap::Schema&>(schema));

    size_tracker_.schema_written(schema);
    file_tracker_->set_current_file_size(size_tracker_.get_potential_mcap_size());

    // Store the schema to write it down when the MCAP file is closed
    schemas_[schema.id] = schema;
}

void McapWriter::write_attachment_nts_()
{
    mcap::Attachment attachment;

    // Write down the attachment with the dynamic types
    attachment.name = DYNAMIC_TYPES_ATTACHMENT_NAME;
    attachment.data = reinterpret_cast<std::byte*>(dynamic_types_payload_->data);
    attachment.dataSize = dynamic_types_payload_->length;
    attachment.createTime =
            mcap::Timestamp(std::chrono::duration_cast<std::chrono::nanoseconds>(
                        utils::now().time_since_epoch()).count());

    write_nts_(attachment);
}

void McapWriter::write_channels_nts_()
{
    if (channels_.empty())
    {
        return;
    }

    logInfo(DDSRECORDER_MCAP_WRITER, "Writing received channels.");

    // Write channels to MCAP file
    for (const auto& [_, channel] : channels_)
    {
        write_nts_(channel);
    }
}

void McapWriter::write_metadata_nts_()
{
    mcap::Metadata metadata;

    // Write down the metadata with the version
    metadata.name = VERSION_METADATA_NAME;
    metadata.metadata[VERSION_METADATA_NAME] = DDSRECORDER_PARTICIPANTS_VERSION_STRING;
    metadata.metadata[VERSION_METADATA_COMMIT] = DDSRECORDER_PARTICIPANTS_COMMIT_HASH;

    write_nts_(metadata);
}

void McapWriter::write_schemas_nts_()
{
    if (schemas_.empty())
    {
        return;
    }

    logInfo(DDSRECORDER_MCAP_WRITER, "Writing received schemas.");

    // Write schemas to MCAP file
    for (const auto& [_, schema] : schemas_)
    {
        write_nts_(schema);
    }
}

void McapWriter::on_mcap_full_nts_(
        const McapFullException& e)
{
    close_current_file_nts_();

    // Disable the writer in case opening a new file fails
    enabled_ = false;

    try
    {
        // Open a new file to write the remaining data.
        // Throw an exception if a file with the minimum size cannot be opened.
        const auto min_file_size = size_tracker_.get_min_mcap_size() + e.data_size_to_write();
        open_new_file_nts_(min_file_size);

        // The file has been opened correctly. Enable the writer.
        enabled_ = true;
    }
    catch (const std::exception& new_e)
    {
        logError(DDSRECORDER_MCAP_WRITER,
                "Error opening a new MCAP file: " << new_e.what());
        throw e;
    }
}

void McapWriter::on_mcap_full_nts_(
        const McapFullException& e,
        std::function<void()> func)
{
    on_mcap_full_nts_(e);
    func();
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
