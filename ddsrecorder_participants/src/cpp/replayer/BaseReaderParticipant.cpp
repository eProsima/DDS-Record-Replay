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
 * @file BaseReaderParticipant.cpp
 */

#include <chrono>

#include <fastdds/rtps/history/IPayloadPool.h>

#include <cpp_utils/Log.hpp>
#include <cpp_utils/ros2_mangling.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/types/cast.hpp>

#include <ddspipe_core/types/dds/Payload.hpp>
#include <ddspipe_core/types/data/RtpsPayloadData.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>
#include <ddspipe_participants/reader/auxiliar/BlankReader.hpp>
#include <ddspipe_participants/reader/auxiliar/InternalReader.hpp>
#include <ddspipe_participants/writer/auxiliar/BlankWriter.hpp>

#include <ddsrecorder_participants/replayer/BaseReaderParticipant.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

BaseReaderParticipant::BaseReaderParticipant(
        const std::shared_ptr<BaseReaderParticipantConfiguration>& configuration,
        const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
        const std::string& file_path)
    : configuration_(configuration)
    , payload_pool_(payload_pool)
    , file_path_(file_path)
    , stop_(false)
{
    // Do nothing
}

ddspipe::core::types::ParticipantId BaseReaderParticipant::id() const noexcept
{
    return configuration_->id;
}

bool BaseReaderParticipant::is_repeater() const noexcept
{
    return false;
}

bool BaseReaderParticipant::is_rtps_kind() const noexcept
{
    return false;
}

ddspipe::core::types::TopicQoS BaseReaderParticipant::topic_qos() const noexcept
{
    return configuration_->topic_qos;
}

std::shared_ptr<ddspipe::core::IWriter> BaseReaderParticipant::create_writer(
        const ddspipe::core::ITopic& /* topic */)
{
    return std::make_shared<ddspipe::participants::BlankWriter>();
}

std::shared_ptr<ddspipe::core::IReader> BaseReaderParticipant::create_reader(
        const ddspipe::core::ITopic& topic)
{
    if (!utils::can_cast<ddspipe::core::types::DdsTopic>(topic))
    {
        logWarning(DDSREPLAYER_MCAP_READER_PARTICIPANT, "Not creating Writer for topic " << topic.topic_name());
        return std::make_shared<ddspipe::participants::BlankReader>();
    }

    auto reader = std::make_shared<ddspipe::participants::InternalReader>(id());

    auto dds_topic = dynamic_cast<const ddspipe::core::types::DdsTopic&>(topic);

    readers_[dds_topic] = reader;

    return reader;
}

void BaseReaderParticipant::stop() noexcept
{
    {
        std::lock_guard<std::mutex> lock(scheduling_cv_mtx_);
        stop_ = true;
    }
    scheduling_cv_.notify_one();
}

std::unique_ptr<ddspipe::core::types::RtpsPayloadData> BaseReaderParticipant::create_payload_(
        const void* raw_data,
        const std::uint32_t raw_data_size)
{
    auto data = std::make_unique<ddspipe::core::types::RtpsPayloadData>();

    // Store the raw data in a payload
    ddspipe::core::types::Payload payload(raw_data_size);
    payload.max_size = raw_data_size;
    payload.length = raw_data_size;
    payload.data = (unsigned char*) reinterpret_cast<const unsigned char*>(raw_data);

    // Reserve and copy the payload into the payload pool
    fastrtps::rtps::IPayloadPool* null_payload_pool = nullptr;

    payload_pool_->get_payload(payload, null_payload_pool, data->payload);
    data->payload_owner = payload_pool_.get();

    // Remove the raw data pointer to avoid freeing it on destruction
    payload.data = nullptr;

    return data;
}

ddspipe::core::types::DdsTopic BaseReaderParticipant::create_topic_(
        const std::string& topic_name,
        const std::string& type_name,
        const bool is_ros2_topic)
{
    ddspipe::core::types::DdsTopic topic;

    if (is_ros2_topic)
    {
        topic.m_topic_name = utils::mangle_if_ros_topic(topic_name);
        topic.type_name = utils::mangle_if_ros_type(type_name);
    }
    else
    {
        topic.m_topic_name = topic_name;
        topic.type_name = type_name;
    }

    return topic;
}

utils::Timestamp BaseReaderParticipant::when_to_start_replay_(
        const utils::Fuzzy<utils::Timestamp>& start_replay_time)
{
    const auto now = utils::now();

    if (!start_replay_time.is_set())
    {
        return now;
    }

    const auto time = start_replay_time.get_reference();

    if (time < now)
    {
        logWarning(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                "Provided start-replay-time already expired, starting immediately...");
        return now;
    }

    return time;
}

void BaseReaderParticipant::wait_until_timestamp_(
        const utils::Timestamp& timestamp)
{
    const auto timepoint = std::chrono::time_point_cast<utils::Timestamp::duration>(timestamp);

    std::unique_lock<std::mutex> lock(scheduling_cv_mtx_);
    scheduling_cv_.wait_until(
        lock,
        timepoint,
        [&]
        {
            return stop_ || (utils::now() >= timepoint);
        });
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
