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
 * @file McapReaderParticipant.cpp
 */

#include <chrono>
#include <exception>
#include <stdexcept>
#include <string>

#include <mcap/errors.hpp>
#include <mcap/reader.hpp>
#include <mcap/types.hpp>

#include <fastdds/dds/core/Time_t.hpp>

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/Log.hpp>
#include <cpp_utils/memory/Heritable.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/types/Fuzzy.hpp>
#include <cpp_utils/utils.hpp>

#include <ddspipe_core/types/dds/TopicQoS.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddsrecorder_participants/common/serialize/Serializer.hpp>
#include <ddsrecorder_participants/common/time_utils.hpp>
#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollection.hpp>
#include <ddsrecorder_participants/constants.hpp>
#include <ddsrecorder_participants/replayer/McapReaderParticipant.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

McapReaderParticipant::McapReaderParticipant(
        const std::shared_ptr<BaseReaderParticipantConfiguration>& configuration,
        const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
        const std::string& file_path)
    : BaseReaderParticipant(configuration, payload_pool, file_path)
{
}

void McapReaderParticipant::add_partition_list(
        std::set<std::string> allowed_partition_list)
{
    // adds the allowed partitions list to the class
    allowed_partition_list_ = allowed_partition_list;
}

void McapReaderParticipant::update_partition_list(
        std::set<std::string> allowed_partition_list)
{
    allowed_partition_list_ = allowed_partition_list;
    filtered_writersguid_list_.clear();

    for (const auto& [topic_id, topic] : topics_)
    {
        for (const auto& [writer, writer_partition] : topic.partition_name)
        {
            bool pass_partition_filter = allowed_partition_list_.empty();

            if (writer_partition == "*" || pass_partition_filter)
            {
                pass_partition_filter = true;
            }
            else
            {
                std::string curr_partition;
                std::vector<std::string> partition_vector;
                int j = 0, writer_partition_n = writer_partition.size();
                while (j < writer_partition_n)
                {
                    if (writer_partition[j] == '|')
                    {
                        partition_vector.push_back(curr_partition);
                        curr_partition.clear();
                    }
                    else
                    {
                        curr_partition += writer_partition[j];
                    }
                    j++;
                }

                if (!curr_partition.empty())
                {
                    partition_vector.push_back(curr_partition);
                }
                else if (writer_partition_n == 0 ||
                        writer_partition[writer_partition_n - 1] == '|')
                {
                    partition_vector.push_back("");
                }

                for (const std::string& partition : partition_vector)
                {
                    for (const std::string& allowed_partition : allowed_partition_list_)
                    {
                        if (utils::match_pattern(allowed_partition, partition))
                        {
                            pass_partition_filter = true;
                            break;
                        }
                    }
                    if (pass_partition_filter)
                    {
                        break;
                    }
                }
            }

            if (!pass_partition_filter)
            {
                filtered_writersguid_list_.insert(writer);
            }
        }
    }
}

void McapReaderParticipant::process_summary(
        std::set<utils::Heritable<ddspipe::core::types::DdsTopic>>& topics,
        DynamicTypesCollection& types)
{
    open_file_();

    read_mcap_summary_();

    // Get the topics from the channels and schemas
    const auto channels = mcap_reader_.channels();
    const auto schemas = mcap_reader_.schemas();

    for (const auto& [_, channel]: channels)
    {
        const auto topic_name = channel->topic;
        const auto schema_it = schemas.find(channel->schemaId);
        if (schema_it == schemas.end() || !schema_it->second)
        {
            EPROSIMA_LOG_WARNING(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                    "Skipping topic " << topic_name
                                      << ": schema with id " << channel->schemaId << " not found.");
            continue;
        }
        const auto type_name = schema_it->second->name;

        const auto ros2_types_it = channel->metadata.find(ROS2_TYPES);
        const bool is_topic_ros2_type =
                ros2_types_it != channel->metadata.end() && ros2_types_it->second == "true";
        const auto topic = utils::Heritable<ddspipe::core::types::DdsTopic>::make_heritable(
            create_topic_(topic_name, type_name, is_topic_ros2_type));

        const auto topic_id = std::make_pair(topic_name, type_name);

        // Apply the QoS stored in the MCAP file as if they were the discovered QoS.
        const auto topic_qos_it = channel->metadata.find(QOS_SERIALIZATION_QOS);
        if (topic_qos_it != channel->metadata.end())
        {
            ddspipe::core::types::TopicQoS topic_qos;
            Serializer::deserialize<ddspipe::core::types::TopicQoS>(topic_qos_it->second, topic_qos);
            topic->topic_qos.set_qos(topic_qos, utils::FuzzyLevelValues::fuzzy_level_fuzzy);
        }
        else
        {
            EPROSIMA_LOG_WARNING(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                    "Topic " << topic_name
                             << " has no serialized QoS metadata. Using default QoS.");
        }

        std::string writer = "";
        std::string writer_partition = "";
        bool pass_partition_filter;

        std::string channel_partitions;
        const auto partitions_it = channel->metadata.find(PARTITIONS);
        if (partitions_it != channel->metadata.end())
        {
            channel_partitions = partitions_it->second;
        }

        // adds the partitions of the topic using the stored channel
        // metadata[partitions] = <writer_1>:<partition_1>;...;<writer_n>:<partition_n>
        // using: n >= 1
        int i = 0, partitions_n = channel_partitions.size();
        while (i < partitions_n)
        {
            // resets the filter condition
            pass_partition_filter = allowed_partition_list_.empty();

            // -- Writer (get one of the possible writers) --------------------
            while (i < partitions_n && channel_partitions[i] != ':')
            {
                writer += channel_partitions[i++];
            }
            i++;

            // -- Partition (get the partitions set) --------------------------
            while (i < partitions_n && channel_partitions[i] != ';')
            {
                writer_partition += channel_partitions[i++];
            }

            // add to the topic, the pair (writer_guid, partitions)
            topic->partition_name[writer] = writer_partition;

            // -- Partitions filter -------------------------------------------

            // checks if the writer partition is the wildcard or the
            // allowed partition list is empty
            if (writer_partition == "*" || pass_partition_filter)
            {
                pass_partition_filter = true;
            }
            else
            {
                // get all the partitions
                std::string curr_partition = "";
                std::vector<std::string> partition_vector;
                int j = 0, writer_partition_n = writer_partition.size();
                while (j < writer_partition_n)
                {
                    if (writer_partition[j] == '|')
                    {
                        // adds the partitions and continue the search
                        partition_vector.push_back(curr_partition);
                        curr_partition = "";
                    }
                    else
                    {
                        curr_partition += writer_partition[j];
                    }

                    j++;
                }

                // adds the last partition
                if (curr_partition != "")
                {
                    partition_vector.push_back(curr_partition);
                }
                // check if have the empty partition.
                else if (writer_partition_n == 0 ||
                        writer_partition[writer_partition_n - 1] == '|')
                {
                    // e.g.:    Partitions: "" only have the empty partition
                    //          Partitions: "A|" have two partitions "A" and "".
                    partition_vector.push_back("");
                }

                // check if the partitions of the writer match with an allowed partition
                for (std::string partition: partition_vector)
                {
                    // check if the current partition is in the filter of partitions
                    for (std::string allowed_partition: allowed_partition_list_)
                    {
                        if (utils::match_pattern(allowed_partition, partition))
                        {
                            pass_partition_filter = true;
                            break;
                        }
                    }
                }
            }

            if (!pass_partition_filter)
            {
                // the writer did not pass the partition filter
                filtered_writersguid_list_.insert(writer);
            }

            i++;

            writer = "";
            writer_partition = "";
        }

        topics_[topic_id] = *topic;

        topics.insert(topic);
    }

    // Get the dynamic types from the attachment
    const auto attachments = mcap_reader_.attachments();

    const auto dynamic_types_attachment_it = attachments.find(DYNAMIC_TYPES_ATTACHMENT_NAME);
    if (dynamic_types_attachment_it != attachments.end())
    {
        const auto& dynamic_types_attachment = dynamic_types_attachment_it->second;

        const std::string dynamic_types_str(
            reinterpret_cast<const char*>(dynamic_types_attachment.data), dynamic_types_attachment.dataSize);

        Serializer::deserialize<DynamicTypesCollection>(dynamic_types_str, types);
    }

    close_file_();
}

void McapReaderParticipant::process_messages()
{
    open_file_();

    auto messages = read_mcap_messages_();

    if (messages.begin() == messages.end())
    {
        EPROSIMA_LOG_WARNING(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                "Provided input file contains no messages in the given range.");
        close_file_();
        return;
    }

    // Obtain timestamp of first recorded message
    const auto first_message_timestamp = to_std_timestamp(messages.begin()->message.logTime);

    // Define the time to start replaying messages
    const auto initial_timestamp = when_to_start_replay_(configuration_->start_replay_time);

    // Replay messages
    for (const auto& it : messages)
    {
        // Create topic on which this message should be published

        const auto topic_id = std::make_pair(it.channel->topic, it.schema->name);
        const auto topic_it = topics_.find(topic_id);
        if (topic_it == topics_.end())
        {
            EPROSIMA_LOG_WARNING(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                    "Skipping message for unknown topic "
                    << it.channel->topic << " with type " << it.schema->name << ".");
            continue;
        }
        const auto& topic = topic_it->second;
        const std::string seq_num_str = std::to_string(it.message.sequence);
        std::string writer_guid = "";
        const auto source_guid_it = source_guid_by_sequence_.find(seq_num_str);
        if (source_guid_it != source_guid_by_sequence_.end())
        {
            const auto writer_guid_it = sequence_by_source_guid_index_.find(source_guid_it->second);
            if (writer_guid_it != sequence_by_source_guid_index_.end())
            {
                writer_guid = writer_guid_it->second;
            }
        }

        if (filtered_writersguid_list_.find(writer_guid) != filtered_writersguid_list_.end())
        {
            // current message do not pass the filter
            continue;
        }

        const auto readers_it = readers_.find(topic);

        if (readers_it == readers_.end())
        {
            EPROSIMA_LOG_ERROR(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                    "Failed to replay message in topic " << topic << ": topic not found, skipping...");
            continue;
        }

        EPROSIMA_LOG_INFO(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                "Scheduling message to be replayed in topic " << topic << ".");

        // Set publication delay from original log time and configured playback rate
        auto delay = to_std_timestamp(it.message.logTime) - first_message_timestamp;
        auto scheduled_write_ts =
                std::chrono::time_point_cast<utils::Timestamp::duration>(initial_timestamp +
                        std::chrono::duration_cast<std::chrono::nanoseconds>(delay / configuration_->rate));

        // Create RTPS data
        auto data = create_payload_(it.message.data, it.message.dataSize);

        // Set source timestamp
        // NOTE: this is important for QoS such as LifespanQosPolicy
        data->source_timestamp = fastdds::dds::Time_t(to_ticks(scheduled_write_ts) / 1e9);

        // add the topic partitions, in the writer_qos
        std::string partition_name = "";
        auto it_partition = topic.partition_name.find(writer_guid);

        // check if the message (using the writer_guid) has partitions
        if (it_partition != topic.partition_name.end())
        {

            // check if the message is already added in the dictionary of PartitionsQos
            // (optimize the search of partitions in the message by storing the PartitionQos of the writer_guid)
            if (partitions_qos_dict_.find(writer_guid) != partitions_qos_dict_.end())
            {
                data->writer_qos.partitions = partitions_qos_dict_[writer_guid];
            }
            else
            {
                partition_name = it_partition->second;
                if (!partition_name.empty())
                {
                    int i = 0, partition_name_n = partition_name.size();
                    std::string tmp = "";
                    while (i < partition_name_n)
                    {
                        if (partition_name[i] == '|')
                        {
                            data->writer_qos.partitions.push_back(tmp.c_str());
                            tmp = "";
                        }
                        else
                        {
                            tmp += partition_name[i];
                        }

                        i++;
                    }
                    // add the last partition in the set of partitions.
                    // e.g.: "A|B" adds the "B" partition
                    if (!tmp.empty() || partition_name[partition_name_n - 1] == '|')
                    {
                        data->writer_qos.partitions.push_back(tmp.c_str());
                    }

                }
                // Empty partition set ("") must still be represented with one empty partition.
                else
                {
                    data->writer_qos.partitions.push_back("");
                }

                partitions_qos_dict_[writer_guid] = data->writer_qos.partitions;
            }
        }

        // Wait until it's time to write the message
        wait_until_timestamp_(scheduled_write_ts);

        EPROSIMA_LOG_INFO(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                "Replaying message in topic " << topic << ".");

        // Insert new data in internal reader queue, with a try catch
        try
        {
            readers_it->second->simulate_data_reception(std::move(data));
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                    "Failed to replay message in topic " << topic
                                                         << ": " << e.what() << ". Skipping...");
        }
    }

    close_file_();
}

void McapReaderParticipant::open_file_()
{
    const auto status = mcap_reader_.open(file_path_);

    if (status.code != mcap::StatusCode::Success)
    {
        throw utils::InitializationException(STR_ENTRY << "Failed to open MCAP.");
    }
}

void McapReaderParticipant::close_file_()
{
    mcap_reader_.close();
}

void McapReaderParticipant::read_mcap_summary_()
{
    // Read mcap summary: ForceScan method required for parsing metadata and attachments
    const auto status = mcap_reader_.readSummary(mcap::ReadSummaryMethod::ForceScan, [](const mcap::Status& status)
                    {
                        EPROSIMA_LOG_WARNING(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                        "An error occurred while reading MCAP summary: " << status.message << ".");
                    });

    if (status.code != mcap::StatusCode::Success)
    {
        throw utils::InitializationException(STR_ENTRY << "Failed to read summary.");
    }

    // Check the recording version is correct
    const auto metadata = mcap_reader_.metadata();
    std::string recording_version;

    // Version metadata, not guaranteed in all files, if absent replay the file
    // without failures, just log a warning message
    const auto version_metadata_it = metadata.find(VERSION_METADATA_NAME);
    if (version_metadata_it != metadata.end())
    {
        const auto& version_metadata = version_metadata_it->second.metadata;
        const auto release_it = version_metadata.find(VERSION_METADATA_RELEASE);
        if (release_it != version_metadata.end())
        {
            recording_version = release_it->second;
        }
        else
        {
            EPROSIMA_LOG_WARNING(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                    "MCAP metadata does not include recording release information.");
        }
    }

    if (recording_version != DDSRECORDER_PARTICIPANTS_VERSION_STRING)
    {
        EPROSIMA_LOG_WARNING(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                "MCAP file generated with a different DDS Record & Replay version ("
                << recording_version << ", current is "
                << DDSRECORDER_PARTICIPANTS_VERSION_STRING
                << "), incompatibilities might arise...");
    }

    const auto sequence_metadata_it = metadata.find(VERSION_METADATA_MESSAGE_NAME);
    if (sequence_metadata_it != metadata.end())
    {
        source_guid_by_sequence_ = sequence_metadata_it->second.metadata;

        const auto source_guid_index_metadata_it = metadata.find(VERSION_METADATA_MESSAGE_INDEX_NAME);
        if (source_guid_index_metadata_it != metadata.end())
        {
            sequence_by_source_guid_index_ = source_guid_index_metadata_it->second.metadata;
        }
        else
        {
            EPROSIMA_LOG_WARNING(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                    "MCAP metadata does not include source-guid index map.");
            sequence_by_source_guid_index_.clear();
        }
    }
    else
    {
        source_guid_by_sequence_.clear();
        sequence_by_source_guid_index_.clear();
    }
}

mcap::LinearMessageView McapReaderParticipant::read_mcap_messages_()
{
    // NOTE: begin_time < end_time assertion already done in YAML module
    const mcap::Timestamp begin_time =
            configuration_->begin_time.is_set() ?
            to_mcap_timestamp(configuration_->begin_time.get_reference()) :
            0;

    const mcap::Timestamp end_time =
            configuration_->end_time.is_set() ?
            to_mcap_timestamp(configuration_->end_time.get_reference()) :
            mcap::MaxTime;

    mcap::ReadMessageOptions read_options(begin_time, end_time);

    // Iterate over messages ordered by incremental log_time
    // NOTE: this corresponds to recording time (not publication) unless recorder configured with `log-publish-time: true`
    read_options.readOrder = mcap::ReadMessageOptions::ReadOrder::LogTimeOrder;

    // Read messages
    auto messages = mcap_reader_.readMessages([](const mcap::Status& status)
                    {
                        EPROSIMA_LOG_WARNING(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                        "An error occurred while reading MCAP messages: " << status.message << ".");
                    }, read_options);

    return messages;
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
