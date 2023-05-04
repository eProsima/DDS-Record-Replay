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

#pragma once

#include <memory>
#include <set>

#include <cpp_utils/thread_pool/pool/SlotThreadPool.hpp>
#include <cpp_utils/ReturnCode.hpp>

#include <ddspipe_core/core/DdsPipe.hpp>
#include <ddspipe_core/dynamic/AllowedTopicList.hpp>
#include <ddspipe_core/dynamic/DiscoveryDatabase.hpp>
#include <ddspipe_core/dynamic/ParticipantsDatabase.hpp>
#include <ddspipe_core/efficiency/payload/FastPayloadPool.hpp>
#include <ddspipe_core/types/topic/dds/DistributedTopic.hpp>

#include <ddspipe_participants/participant/dynamic_types/DynTypesParticipant.hpp>
#include <ddspipe_participants/participant/dynamic_types/SchemaParticipant.hpp>

#include <ddsrecorder_participants/recorder/mcap/McapHandler.hpp>
#include <ddsrecorder_participants/recorder/mcap/McapHandlerConfiguration.hpp>

#include <ddsrecorder_yaml/recorder/YamlReaderConfiguration.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace recorder {

class DdsRecorder
{
public:

    DdsRecorder(
            const yaml::RecorderConfiguration& configuration,
            const participants::McapHandlerStateCode& init_state,
            const std::string& file_name = "");

    utils::ReturnCode reload_allowed_topics(
            const std::shared_ptr<ddspipe::core::AllowedTopicList>& allowed_topics);

    void start();

    void pause();

    void stop();

    void trigger_event();

protected:

    //! Payload Pool
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    //! Thread Pool
    std::shared_ptr<utils::SlotThreadPool> thread_pool_;

    //! Discovery Database
    std::shared_ptr<ddspipe::core::DiscoveryDatabase> discovery_database_;

    //! Participants Database
    std::shared_ptr<ddspipe::core::ParticipantsDatabase> participants_database_;

    //! MCAP Handler
    std::shared_ptr<eprosima::ddsrecorder::participants::McapHandler> mcap_handler_;

    //! Dynamic Types Participant
    std::shared_ptr<eprosima::ddspipe::participants::DynTypesParticipant> dyn_participant_;

    //! Schema Participant
    std::shared_ptr<eprosima::ddspipe::participants::SchemaParticipant> recorder_participant_;

    //! DDS Pipe
    std::unique_ptr<ddspipe::core::DdsPipe> pipe_;
};

} /* namespace recorder */
} /* namespace ddsrecorder */
} /* namespace eprosima */
