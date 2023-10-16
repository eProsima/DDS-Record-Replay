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
#include <cpp_utils/utils.hpp>

#include <ddspipe_core/types/dynamic_types/types.hpp>

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
        yaml::RecorderConfiguration& configuration,
        const DdsRecorderStateCode& init_state,
        const std::string& file_name)
{
    // Create Discovery Database
    discovery_database_ = std::make_shared<DiscoveryDatabase>();

    // Create Payload Pool
    payload_pool_ = std::make_shared<FastPayloadPool>();

    // Create Thread Pool
    thread_pool_ = std::make_shared<SlotThreadPool>(configuration.n_threads);

    // Fill MCAP output file settings
    participants::McapOutputSettings mcap_output_settings;
    if (file_name == "")
    {
        mcap_output_settings.output_filename = configuration.output_filename;
        mcap_output_settings.output_filepath = configuration.output_filepath;
        mcap_output_settings.prepend_timestamp = true;
        mcap_output_settings.output_timestamp_format = configuration.output_timestamp_format;
        mcap_output_settings.output_local_timestamp = configuration.output_local_timestamp;
    }
    else
    {
        mcap_output_settings.output_filename = file_name;
        mcap_output_settings.output_filepath = ".";
        mcap_output_settings.prepend_timestamp = false;
    }

    // Create MCAP Handler configuration
    participants::McapHandlerConfiguration handler_config(
        mcap_output_settings,
        configuration.max_pending_samples,
        configuration.buffer_size,
        configuration.event_window,
        configuration.cleanup_period,
        configuration.log_publish_time,
        configuration.only_with_type,
        configuration.mcap_writer_options,
        configuration.record_types);

    // Create MCAP Handler
    mcap_handler_ = std::make_shared<participants::McapHandler>(
        handler_config,
        payload_pool_,
        recorder_to_handler_state_(init_state));

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

    // Create the internal communication (built-in) topics
    const auto& internal_topic = utils::Heritable<DistributedTopic>::make_heritable(type_object_topic());

    configuration.ddspipe_configuration.builtin_topics.insert(internal_topic);

    // Create Participant Database
    participants_database_ = std::make_shared<ParticipantsDatabase>();

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
        configuration.ddspipe_configuration,
        discovery_database_,
        payload_pool_,
        participants_database_,
        thread_pool_);
}

utils::ReturnCode DdsRecorder::reload_configuration(
        const yaml::RecorderConfiguration& new_configuration)
{
    return pipe_->reload_configuration(new_configuration.ddspipe_configuration);
}

void DdsRecorder::start()
{
    mcap_handler_->start();
}

void DdsRecorder::pause()
{
    mcap_handler_->pause();
}

void DdsRecorder::suspend()
{
    mcap_handler_->stop();
}

void DdsRecorder::stop()
{
    mcap_handler_->stop();
}

void DdsRecorder::trigger_event()
{
    mcap_handler_->trigger_event();
}

participants::McapHandlerStateCode DdsRecorder::recorder_to_handler_state_(
        const DdsRecorderStateCode& recorder_state)
{
    switch (recorder_state)
    {
        case DdsRecorderStateCode::RUNNING:
            return participants::McapHandlerStateCode::RUNNING;

        case DdsRecorderStateCode::PAUSED:
            return participants::McapHandlerStateCode::PAUSED;

        case DdsRecorderStateCode::STOPPED:
        case DdsRecorderStateCode::SUSPENDED:
            return participants::McapHandlerStateCode::STOPPED;

        default:
            // Unreachable
            utils::tsnh(
                utils::Formatter() << "Trying to convert to McapHandler state an invalid DdsRecorder state.");
            return participants::McapHandlerStateCode::STOPPED;
    }
}

} /* namespace recorder */
} /* namespace ddsrecorder */
} /* namespace eprosima */
