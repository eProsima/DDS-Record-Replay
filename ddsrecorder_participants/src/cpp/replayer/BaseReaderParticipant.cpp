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
 * @file BaseReaderParticipant.cpp
 */


#include <cpp_utils/Log.hpp>
#include <cpp_utils/types/cast.hpp>

#include <ddspipe_participants/reader/auxiliar/BlankReader.hpp>
#include <ddspipe_participants/reader/auxiliar/InternalReader.hpp>
#include <ddspipe_participants/writer/auxiliar/BlankWriter.hpp>

#include <ddsrecorder_participants/replayer/BaseReaderParticipant.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

using namespace eprosima::ddspipe::core;
using namespace eprosima::ddspipe::core::types;
using namespace eprosima::ddspipe::participants;
using namespace eprosima::utils;

BaseReaderParticipant::BaseReaderParticipant(
        std::shared_ptr<McapReaderParticipantConfiguration> configuration,
        std::shared_ptr<PayloadPool> payload_pool,
        std::string& file_path)
    : configuration_(configuration)
    , payload_pool_(payload_pool)
    , file_path_(file_path)
    , stop_(false)
{
    // Do nothing
}

ParticipantId BaseReaderParticipant::id() const noexcept
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

TopicQoS BaseReaderParticipant::topic_qos() const noexcept
{
    return configuration_->topic_qos;
}

std::shared_ptr<IWriter> BaseReaderParticipant::create_writer(
        const ITopic& /* topic */)
{
    return std::make_shared<BlankWriter>();
}

std::shared_ptr<IReader> BaseReaderParticipant::create_reader(
        const ITopic& topic)
{
    if (!utils::can_cast<DdsTopic>(topic))
    {
        logWarning(DDSREPLAYER_MCAP_READER_PARTICIPANT, "Not creating Writer for topic " << topic.topic_name());
        return std::make_shared<BlankReader>();
    }

    auto reader = std::make_shared<InternalReader>(id());

    auto dds_topic = dynamic_cast<const DdsTopic&>(topic);

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

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
