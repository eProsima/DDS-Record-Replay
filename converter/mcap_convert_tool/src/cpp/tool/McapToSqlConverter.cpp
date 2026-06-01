// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "McapToSqlConverter.hpp"

#include <algorithm>
#include <cctype>
#include <exception>
#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <mcap/reader.hpp>

#include <fastdds/dds/core/Time_t.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/Formatter.hpp>
#include <cpp_utils/Log.hpp>

#include <ddspipe_core/efficiency/payload/FastPayloadPool.hpp>
#include <ddspipe_core/types/data/RtpsPayloadData.hpp>
#include <ddspipe_core/types/dds/Guid.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollection.hpp>
#include <ddsrecorder_participants/constants.hpp>
#include <ddsrecorder_participants/recorder/handler/sql/SqlWriter.hpp>
#include <ddsrecorder_participants/recorder/message/SqlMessage.hpp>
#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>
#include <ddsrecorder_participants/recorder/output/OutputSettings.hpp>
#include <ddsrecorder_participants/replayer/DynamicTypesSupport.hpp>
#include <ddsrecorder_participants/replayer/McapReaderParticipant.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace converter {

namespace {

class McapReaderParticipantAccessor : public participants::McapReaderParticipant
{
public:

    using participants::BaseReaderParticipant::create_payload_;
    using participants::McapReaderParticipant::close_file_;
    using participants::McapReaderParticipant::open_file_;
    using participants::McapReaderParticipant::read_mcap_messages_;

    McapReaderParticipantAccessor(
            const std::shared_ptr<participants::BaseReaderParticipantConfiguration>& configuration,
            const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
            const std::string& file_path)
        : participants::McapReaderParticipant(configuration, payload_pool, file_path)
    {
    }

    const std::set<std::string>& filtered_writersguid_list() const noexcept
    {
        return filtered_writersguid_list_;
    }

    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool() const noexcept
    {
        return payload_pool_;
    }

    const mcap::KeyValueMap& sequence_by_source_guid_index() const noexcept
    {
        return sequence_by_source_guid_index_;
    }

    const mcap::KeyValueMap& source_guid_by_sequence() const noexcept
    {
        return source_guid_by_sequence_;
    }

    const std::map<std::pair<std::string, std::string>, ddspipe::core::types::DdsTopic>& topics() const noexcept
    {
        return topics_;
    }
};

std::string get_writer_guid_string_(
        const McapReaderParticipantAccessor& reader,
        const mcap::MessageView& message)
{
    const auto sequence_str = std::to_string(message.message.sequence);
    const auto source_guid_it = reader.source_guid_by_sequence().find(sequence_str);

    if (source_guid_it == reader.source_guid_by_sequence().end())
    {
        return {};
    }

    const auto writer_guid_it = reader.sequence_by_source_guid_index().find(source_guid_it->second);

    if (writer_guid_it == reader.sequence_by_source_guid_index().end())
    {
        return {};
    }

    return writer_guid_it->second;
}

ddspipe::core::types::Guid to_guid_(
        const std::string& writer_guid_str)
{
    if (writer_guid_str.empty())
    {
        return {};
    }

    try
    {
        return ddspipe::core::types::Guid(writer_guid_str);
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_WARNING(
            DDSREPLAYER,
            "Failed to parse writer GUID '" << writer_guid_str << "': " << e.what());
        return {};
    }
}

std::string get_writer_partition_(
        const ddspipe::core::types::DdsTopic& topic,
        const std::string& writer_guid_str)
{
    const auto partition_it = topic.partition_name.find(writer_guid_str);
    return partition_it == topic.partition_name.end() ? std::string() : partition_it->second;
}

void write_topic_metadata_(
        participants::SqlWriter& sql_writer,
        const ddspipe::core::types::DdsTopic& topic,
        const std::string& partition,
        std::set<ddspipe::core::types::DdsTopic>& written_topics,
        std::set<std::string>& written_partitions,
        std::set<std::string>& written_topic_partitions)
{
    if (written_topics.insert(topic).second)
    {
        sql_writer.write(topic);
    }

    if (written_partitions.insert(partition).second)
    {
        sql_writer.write(partition);
    }

    const auto topic_partition_key = topic.topic_name() + topic.type_name + partition;
    if (written_topic_partitions.insert(topic_partition_key).second)
    {
        sql_writer.write_partition(topic.topic_name(), topic.type_name, partition);
    }
}

bool is_dependency_type_name_(
        const std::string& name,
        const std::set<std::string>& type_names)
{
    const auto underscore_pos = name.rfind('_');
    if (underscore_pos == std::string::npos || underscore_pos == name.size() - 1)
    {
        return false;
    }

    const auto suffix_begin = name.begin() + underscore_pos + 1;
    if (!std::all_of(suffix_begin, name.end(), [](unsigned char c)
            {
                return std::isdigit(c) != 0;
            }))
    {
        return false;
    }

    const auto parent = name.substr(0, underscore_pos);
    return type_names.find(parent) != type_names.end();
}

participants::OutputSettings create_output_settings_(
        const std::string& output_file)
{
    participants::OutputSettings output_settings;
    const auto output_path = std::filesystem::path(output_file);
    const auto output_directory = output_path.has_parent_path() ? output_path.parent_path() : std::filesystem::path(".");

    output_settings.filepath = output_directory.string();
    output_settings.filename = output_path.stem().string();
    output_settings.extension = output_path.has_extension() ? output_path.extension().string() : ".db";
    output_settings.prepend_timestamp = false;
    output_settings.local_timestamp = false;
    output_settings.timestamp_format.clear();

    participants::ResourceLimitsStruct resource_limits;
    resource_limits.file_rotation_ = false;

    std::uintmax_t available_space = 0;
    try
    {
        available_space = std::filesystem::space(output_directory).available;
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        throw utils::InitializationException(
                  utils::Formatter() << "Failed to access SQL output directory " << output_directory.string()
                                     << ": " << e.what());
    }

    if (!output_settings.set_resource_limits(resource_limits, available_space))
    {
        throw utils::InitializationException(
                  utils::Formatter() << "Failed to configure SQL output path " << output_file << ".");
    }

    return output_settings;
}

bool compute_instance_handle_(
        const fastdds::dds::DynamicType::_ref_type& dynamic_type,
        ddspipe::core::types::Payload& payload,
        ddspipe::core::types::InstanceHandle& instance_handle)
{
    fastdds::dds::DynamicPubSubType pub_sub_type(dynamic_type);
    return pub_sub_type.compute_key(payload, instance_handle);
}

} // namespace

McapToSqlConverter::McapToSqlConverter(
        const yaml::ReplayerConfiguration& configuration,
        const std::string& input_file,
        const std::string& output_file)
    : configuration_(configuration)
    , input_file_(input_file)
    , output_file_(resolve_output_file(input_file, output_file))
{
}

std::string McapToSqlConverter::resolve_output_file(
        const std::string& input_file,
        const std::string& output_file)
{
    if (!output_file.empty())
    {
        auto explicit_output = std::filesystem::path(output_file);

        if (!explicit_output.has_extension())
        {
            explicit_output += ".db";
        }

        return explicit_output.string();
    }

    auto default_output = std::filesystem::path(input_file);
    default_output.replace_extension(".db");
    return default_output.string();
}

void McapToSqlConverter::convert()
{
    auto payload_pool = std::make_shared<ddspipe::core::FastPayloadPool>();
    McapReaderParticipantAccessor reader(
        configuration_.base_reader_configuration,
        payload_pool,
        input_file_);

    if (configuration_.replayer_configuration)
    {
        reader.add_partition_list(configuration_.replayer_configuration->allowed_partition_list);
    }

    std::set<utils::Heritable<ddspipe::core::types::DdsTopic>> topics;
    participants::DynamicTypesCollection dynamic_types_collection;
    reader.process_summary(topics, dynamic_types_collection);

    const auto registered_dynamic_types = participants::detail::register_dynamic_types(dynamic_types_collection);
    const auto dynamic_types_by_name = participants::detail::build_dynamic_types(registered_dynamic_types);

    auto output_settings = create_output_settings_(output_file_);
    auto file_tracker = std::make_shared<participants::FileTracker>(output_settings);
    participants::SqlWriter sql_writer(output_settings, file_tracker, true, false, participants::DataFormat::both);

    std::set<ddspipe::core::types::DdsTopic> written_topics;
    std::set<std::string> written_partitions;
    std::set<std::string> written_topic_partitions;
    std::map<ddspipe::core::types::InstanceHandle, std::string> keys_by_instance_handle;

    sql_writer.enable();

    try
    {
        std::set<std::string> dynamic_type_names;
        for (const auto& dynamic_type : dynamic_types_collection.dynamic_types())
        {
            dynamic_type_names.insert(dynamic_type.type_name());
        }

        for (const auto& dynamic_type : dynamic_types_collection.dynamic_types())
        {
            if (is_dependency_type_name_(dynamic_type.type_name(), dynamic_type_names))
            {
                continue;
            }

            sql_writer.update_dynamic_types(dynamic_type);
        }

        reader.open_file_();
        auto messages = reader.read_mcap_messages_();

        if (messages.begin() == messages.end())
        {
            EPROSIMA_LOG_WARNING(
                DDSREPLAYER,
                "Provided input file contains no messages in the given range.");
        }

        for (const auto& message : messages)
        {
            const auto topic_id = std::make_pair(message.channel->topic, message.schema->name);
            const auto topic_it = reader.topics().find(topic_id);

            if (topic_it == reader.topics().end())
            {
                EPROSIMA_LOG_WARNING(
                    DDSREPLAYER,
                    "Skipping message for unknown topic " << message.channel->topic
                                                          << " with type " << message.schema->name << ".");
                continue;
            }

            const auto writer_guid_str = get_writer_guid_string_(reader, message);

            if (reader.filtered_writersguid_list().find(writer_guid_str) != reader.filtered_writersguid_list().end())
            {
                continue;
            }

            auto data = reader.create_payload_(message.message.data, message.message.dataSize);
            data->source_guid = to_guid_(writer_guid_str);

            const auto dynamic_type_it = dynamic_types_by_name.find(topic_it->second.type_name);
            const auto has_instance_handle = dynamic_type_it != dynamic_types_by_name.end() &&
                    compute_instance_handle_(dynamic_type_it->second, data->payload, data->instanceHandle);

            participants::SqlMessage sql_message(*data, reader.payload_pool(), topic_it->second);
            sql_message.sequence_number = fastdds::rtps::SequenceNumber_t(
                static_cast<uint64_t>(message.message.sequence));
            sql_message.log_time = fastdds::dds::Time_t(
                static_cast<int32_t>(message.message.logTime / 1000000000ULL),
                static_cast<uint32_t>(message.message.logTime % 1000000000ULL));
            sql_message.publish_time = fastdds::dds::Time_t(
                static_cast<int32_t>(message.message.publishTime / 1000000000ULL),
                static_cast<uint32_t>(message.message.publishTime % 1000000000ULL));

            const auto partition = get_writer_partition_(sql_message.topic, writer_guid_str);
            write_topic_metadata_(
                sql_writer,
                sql_message.topic,
                partition,
                written_topics,
                written_partitions,
                written_topic_partitions);

            if (dynamic_type_it == dynamic_types_by_name.end())
            {
                EPROSIMA_LOG_WARNING(
                    DDSREPLAYER,
                    "Type information for topic " << sql_message.topic.topic_name()
                                                  << " with type " << sql_message.topic.type_name
                                                  << " is not available. Storing only CDR payload.");
            }
            else
            {
                sql_message.deserialize(dynamic_type_it->second);

                const auto key_it = has_instance_handle ?
                        keys_by_instance_handle.find(sql_message.instance_handle) :
                        keys_by_instance_handle.end();

                if (has_instance_handle && key_it != keys_by_instance_handle.end())
                {
                    sql_message.key = key_it->second;
                }
                else
                {
                    sql_message.set_key(dynamic_type_it->second);
                    if (has_instance_handle)
                    {
                        keys_by_instance_handle[sql_message.instance_handle] = sql_message.key;
                    }
                }
            }

            sql_writer.write(std::vector<participants::SqlMessage>{sql_message});
        }

        reader.close_file_();
        sql_writer.disable();
    }
    catch (...)
    {
        try
        {
            reader.close_file_();
        }
        catch (const std::exception&)
        {
        }

        try
        {
            sql_writer.disable();
        }
        catch (const std::exception&)
        {
        }

        throw;
    }
}

} /* namespace converter */
} /* namespace ddsrecorder */
} /* namespace eprosima */
