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

#include <mcap/reader.hpp>

#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastdds/rtps/history/IPayloadPool.h>

#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/Log.hpp>
#include <cpp_utils/ros2_mangling.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/utils.hpp>

#include <ddspipe_core/types/data/RtpsPayloadData.hpp>
#include <ddspipe_core/types/dds/Payload.hpp>

#include <ddsrecorder_participants/constants.hpp>
#include <ddsrecorder_participants/replayer/McapReaderParticipant.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

using namespace eprosima::ddspipe::core;
using namespace eprosima::ddspipe::core::types;
using namespace eprosima::ddspipe::participants;
using namespace eprosima::utils;

McapReaderParticipant::McapReaderParticipant(
        std::shared_ptr<McapReaderParticipantConfiguration> configuration,
        std::shared_ptr<PayloadPool> payload_pool,
        std::string& file_path)
    : BaseReaderParticipant(configuration, payload_pool, file_path)
{
    // Do nothing
}

void McapReaderParticipant::process_file()
{
    // Read MCAP file
    mcap::McapReader mcap_reader;
    auto status = mcap_reader.open(file_path_);
    if (status.code != mcap::StatusCode::Success)
    {
        throw utils::InconsistencyException(
                  STR_ENTRY << "Failed MCAP read."
                  );
    }

    // NOTE: begin_time < end_time assertion already done in YAML module
    mcap::Timestamp begin_time = 0;
    mcap::Timestamp end_time = mcap::MaxTime;
    if (configuration_->begin_time.is_set())
    {
        begin_time = std_timepoint_to_mcap_timestamp(configuration_->begin_time.get_reference());
    }
    if (configuration_->end_time.is_set())
    {
        end_time = std_timepoint_to_mcap_timestamp(configuration_->end_time.get_reference());
    }
    mcap::ReadMessageOptions read_options(begin_time, end_time);

    // Iterate over messages ordered by incremental log_time
    // NOTE: this corresponds to recording time (not publication) unless recorder configured with `log-publish-time: true`
    read_options.readOrder = mcap::ReadMessageOptions::ReadOrder::LogTimeOrder;

    // Read messages
    const auto onProblem = [](const mcap::Status& status)
            {
                logWarning(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                        "An error occurred while reading messages: " << status.message << ".");
            };
    auto messages = mcap_reader.readMessages(onProblem, read_options);

    // Obtain timestamp of first recorded message
    utils::Timestamp initial_ts_origin;
    auto messages_it = messages.begin();
    auto messages_end = messages.end();
    if (messages_it != messages_end)
    {
        initial_ts_origin = mcap_timestamp_to_std_timepoint(messages_it->message.logTime);
    }
    else
    {
        logWarning(DDSREPLAYER_MCAP_READER_PARTICIPANT, "Provided input file contains no messages in the given range.");
        return;
    }

    // Define the time to start replaying messages
    utils::Timestamp initial_ts;
    utils::Timestamp now = utils::now();
    if (configuration_->start_replay_time.is_set())
    {
        initial_ts = configuration_->start_replay_time.get_reference();

        if (initial_ts < now)
        {
            logWarning(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                    "Provided start-replay-time already expired, starting immediately...");
            initial_ts = now;
        }
    }
    else
    {
        initial_ts = now;
    }

    // Schedule messages to be replayed
    utils::Timestamp scheduled_write_ts;
    for (auto it = messages.begin(); it != messages_end; it++)
    {
        // Create RTPS data
        auto data = std::make_unique<RtpsPayloadData>();

        // Create data payload
        Payload mcap_payload;
        mcap_payload.length = it->message.dataSize;
        mcap_payload.max_size = it->message.dataSize;
        mcap_payload.data = (unsigned char*)reinterpret_cast<const unsigned char*>(it->message.data);

        // Copy payload from MCAP file to RTPS data through payload pool
        eprosima::fastrtps::rtps::IPayloadPool* null_payload_pool = nullptr;
        payload_pool_->get_payload(mcap_payload, null_payload_pool, data->payload); // this reserves and copies payload
        data->payload_owner = payload_pool_.get();
        mcap_payload.data = nullptr; // Set to nullptr after copy to avoid free on destruction

        // Set publication delay from original log time and configured playback rate
        auto delay = mcap_timestamp_to_std_timepoint(it->message.logTime) - initial_ts_origin;
        scheduled_write_ts = std::chrono::time_point_cast<utils::Timestamp::duration>(initial_ts + std::chrono::duration_cast<std::chrono::nanoseconds>(
                            delay / configuration_->rate));

        // Set source timestamp
        // NOTE: this is important for QoS such as LifespanQosPolicy
        data->source_timestamp =
                fastrtps::rtps::Time_t(std::chrono::duration_cast<std::chrono::nanoseconds>(scheduled_write_ts
                                .time_since_epoch()).count() / 1e9);

        // Create topic on which this message should be published
        DdsTopic channel_topic;
        channel_topic.m_topic_name = it->channel->metadata[ROS2_TYPES] == "true" ? utils::mangle_if_ros_topic(
            it->channel->topic) : it->channel->topic;
        channel_topic.type_name = it->channel->metadata[ROS2_TYPES] == "true" ? utils::mangle_if_ros_type(
            it->schema->name) : it->schema->name;

        auto readers_it = readers_.find(channel_topic);
        if (readers_it == readers_.end())
        {
            logError(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                    "Failed to replay message in topic " << channel_topic << ": topic not found, skipping...");
            continue;
        }

        logInfo(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                "Scheduling message to be replayed in topic " << readers_it->first << ".");

        {
            std::unique_lock<std::mutex> lock(scheduling_cv_mtx_);
            scheduling_cv_.wait_until(
                lock,
                scheduled_write_ts,
                [&]
                {
                    return stop_ || (utils::now() >= scheduled_write_ts);
                });

            if (stop_)
            {
                logInfo(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                        "Participant stopped while processing MCAP file.");
                break;
            }
        }

        logInfo(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                "Replaying message in topic " << readers_it->first << ".");

        // Insert new data in internal reader queue
        readers_it->second->simulate_data_reception(std::move(data));
    }

    mcap_reader.close();
}

utils::Timestamp McapReaderParticipant::mcap_timestamp_to_std_timepoint(
        const mcap::Timestamp& time)
{
    return std::chrono::time_point_cast<utils::Timestamp::duration>(utils::Timestamp() +
                   std::chrono::nanoseconds(time));
}

mcap::Timestamp McapReaderParticipant::std_timepoint_to_mcap_timestamp(
        const utils::Timestamp& time)
{
    return mcap::Timestamp(std::chrono::duration_cast<std::chrono::nanoseconds>(time.time_since_epoch()).count());
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
