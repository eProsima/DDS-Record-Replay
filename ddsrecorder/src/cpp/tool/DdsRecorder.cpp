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

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/utils.hpp>

#include "DdsRecorder.hpp"

namespace eprosima {
namespace ddsrecorder {
namespace recorder {

using namespace eprosima::ddspipe::core;
using namespace eprosima::ddspipe::core::types;
using namespace eprosima::ddspipe::participants;
using namespace eprosima::ddspipe::participants::rtps;
using namespace eprosima::ddsrecorder::participants;
using namespace eprosima::utils;

DdsRecorder::DdsRecorder(
        const yaml::RecorderConfiguration& configuration,
        const participants::McapHandlerStateCode& init_state,
        const std::string& file_name)
{
    // Create allowed topics list
    auto allowed_topics = std::make_shared<AllowedTopicList>(
        configuration.allowlist,
        configuration.blocklist);

    // Create Discovery Database
    discovery_database_ =
            std::make_shared<DiscoveryDatabase>();

    // Create Payload Pool
    payload_pool_ =
            std::make_shared<FastPayloadPool>();

    // Create Thread Pool
    thread_pool_ =
            std::make_shared<SlotThreadPool>(configuration.n_threads);

    // Append current timestamp to configuration's file_name, if none provided to constructor
    std::string mcap_filename;
    if (file_name == "")
    {
        mcap_filename = configuration.recorder_output_file + "_" + timestamp_to_string(now()) + ".mcap";
    }
    else
    {
        mcap_filename = file_name;
    }

    // Create MCAP Handler configuration
    participants::McapHandlerConfiguration handler_config(
        mcap_filename,
        configuration.max_pending_samples,
        configuration.buffer_size,
        configuration.event_window,
        configuration.cleanup_period,
        configuration.log_publish_time,
        configuration.only_with_type,
        configuration.mcap_writer_options);

    // Create MCAP Handler
    mcap_handler_ = std::make_shared<participants::McapHandler>(
        handler_config,
        payload_pool_,
        init_state);

    // Create DynTypes Participant
    dyn_participant_ = std::make_shared<DynTypesParticipant>(
        configuration.simple_configuration,
        payload_pool_,
        discovery_database_);
    dyn_participant_->init();

    // Create Recorder Participant
    recorder_participant_ = std::make_shared<SchemaParticipant>(
        configuration.recorder_configuration,
        payload_pool_,
        discovery_database_,
        mcap_handler_);

    // Create and populate Participant Database
    participants_database_ =
            std::make_shared<ParticipantsDatabase>();

    // Populate Participant Database
    participants_database_->add_participant(
        dyn_participant_->id(),
        dyn_participant_
        );
    participants_database_->add_participant(
        recorder_participant_->id(),
        recorder_participant_
        );

    // Create DDS Pipe
    pipe_ = std::make_unique<DdsPipe>(
        allowed_topics,
        discovery_database_,
        payload_pool_,
        participants_database_,
        thread_pool_,
        configuration.builtin_topics,
        true);
}

utils::ReturnCode DdsRecorder::reload_allowed_topics(
        const std::shared_ptr<AllowedTopicList>& allowed_topics)
{
    return pipe_->reload_allowed_topics(allowed_topics);
}

void DdsRecorder::start()
{
    mcap_handler_->start();
}

void DdsRecorder::pause()
{
    mcap_handler_->pause();
}

void DdsRecorder::stop()
{
    mcap_handler_->stop();
}

void DdsRecorder::trigger_event()
{
    mcap_handler_->trigger_event();
}

} /* namespace recorder */
} /* namespace ddsrecorder */
} /* namespace eprosima */
