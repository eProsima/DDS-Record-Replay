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
#include <string>

#include <mcap/errors.hpp>
#include <mcap/reader.hpp>
#include <mcap/types.hpp>

#include <fastdds/rtps/common/Time_t.h>

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/Log.hpp>
#include <cpp_utils/memory/Heritable.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/types/Fuzzy.hpp>

#include <ddspipe_core/types/dds/TopicQoS.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddsrecorder_participants/common/serialize/Serializer.hpp>
#include <ddsrecorder_participants/common/time_utils.hpp>
#include <ddsrecorder_participants/constants.hpp>
#include <ddsrecorder_participants/replayer/McapReaderParticipant.hpp>

#if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v1/DynamicTypesCollection.hpp>
#else
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v2/DynamicTypesCollection.hpp>
#endif // if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13

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
        const auto type_name = schemas.at(channel->schemaId)->name;

        const bool is_topic_ros2_type = channel->metadata[ROS2_TYPES] == "true";
        const auto topic = utils::Heritable<ddspipe::core::types::DdsTopic>::make_heritable(
                create_topic_(topic_name, type_name, is_topic_ros2_type));

        // Apply the QoS stored in the MCAP file as if they were the discovered QoS.
        const auto topic_qos_str = channel->metadata[QOS_SERIALIZATION_QOS];
        const auto topic_qos = Serializer::deserialize<ddspipe::core::types::TopicQoS>(topic_qos_str);

        topic->topic_qos.set_qos(topic_qos, utils::FuzzyLevelValues::fuzzy_level_fuzzy);

        topics.insert(topic);
    }

    // Get the dynamic types from the attachment
    const auto attachments = mcap_reader_.attachments();

    if (attachments.find(DYNAMIC_TYPES_ATTACHMENT_NAME) != attachments.end())
    {
        const auto dynamic_types_attachment = attachments.at(DYNAMIC_TYPES_ATTACHMENT_NAME);

        const std::string dynamic_types_str(
                reinterpret_cast<const char*>(dynamic_types_attachment.data), dynamic_types_attachment.dataSize);

        types = Serializer::deserialize<DynamicTypesCollection>(dynamic_types_str);
    }

    close_file_();
}

void McapReaderParticipant::process_messages()
{
    open_file_();

    auto messages = read_mcap_messages_();

    if (messages.begin() == messages.end())
    {
        logWarning(DDSREPLAYER_MCAP_READER_PARTICIPANT, "Provided input file contains no messages in the given range.");
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
        const bool is_topic_ros2_type = it.channel->metadata[ROS2_TYPES] == "true";
        const auto topic = create_topic_(it.channel->topic, it.schema->name, is_topic_ros2_type);

        const auto readers_it = readers_.find(topic);

        if (readers_it == readers_.end())
        {
            logError(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                    "Failed to replay message in topic " << topic << ": topic not found, skipping...");
            continue;
        }

        logInfo(DDSREPLAYER_MCAP_READER_PARTICIPANT,
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
        data->source_timestamp = fastrtps::rtps::Time_t(to_ticks(scheduled_write_ts) / 1e9);

        // Wait until it's time to write the message
        wait_until_timestamp_(scheduled_write_ts);

        logInfo(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                "Replaying message in topic " << topic << ".");

        // Insert new data in internal reader queue
        readers_it->second->simulate_data_reception(std::move(data));
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
                logWarning(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                        "An error occurred while reading MCAP summary: " << status.message << ".");
            });

    if (status.code != mcap::StatusCode::Success)
    {
        throw utils::InitializationException(STR_ENTRY << "Failed to read summary.");
    }

    // Check the recording version is correct
    const auto metadata = mcap_reader_.metadata();
    std::string recording_version;

    if (metadata.find(VERSION_METADATA_NAME) != metadata.end())
    {
        const auto version_metadata = metadata.at(VERSION_METADATA_NAME).metadata;
        recording_version = version_metadata.at(VERSION_METADATA_RELEASE);
    }

    if (recording_version != DDSRECORDER_PARTICIPANTS_VERSION_STRING)
    {
        logWarning(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                "MCAP file generated with a different DDS Record & Replay version (" << recording_version <<
                ", current is " << DDSRECORDER_PARTICIPANTS_VERSION_STRING << "), incompatibilities might arise...");
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
                logWarning(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                        "An error occurred while reading MCAP messages: " << status.message << ".");
            }, read_options);

    return messages;
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
