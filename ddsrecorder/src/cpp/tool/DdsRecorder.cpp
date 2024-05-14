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

#include <filesystem>
#include <math.h>

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/utils.hpp>

#include <ddspipe_core/monitoring/producers/StatusMonitorProducer.hpp>
#include <ddspipe_core/monitoring/producers/TopicsMonitorProducer.hpp>
#include <ddspipe_core/types/dynamic_types/types.hpp>

#include <ddsrecorder_participants/recorder/mcap/McapHandler.hpp>
#include <ddsrecorder_participants/recorder/mcap/McapHandlerConfiguration.hpp>
#include <ddsrecorder_participants/recorder/output/BaseHandler.hpp>
#include <ddsrecorder_participants/recorder/output/OutputSettings.hpp>
#include <ddsrecorder_participants/recorder/sql/SqlHandler.hpp>
#include <ddsrecorder_participants/recorder/sql/SqlHandlerConfiguration.hpp>

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
        const DdsRecorderStateCode& init_state,
        std::shared_ptr<participants::FileTracker>& file_tracker,
        const std::string& file_name /* = "" */)
    : DdsRecorder(configuration, init_state, nullptr, file_tracker, file_name)
{
}

DdsRecorder::DdsRecorder(
        const yaml::RecorderConfiguration& configuration,
        const DdsRecorderStateCode& init_state,
        std::shared_ptr<eprosima::utils::event::MultipleEventHandler> event_handler,
        std::shared_ptr<participants::FileTracker>& file_tracker,
        const std::string& file_name /* = "" */)
    : configuration_(configuration)
    , event_handler_(event_handler)
{
    load_internal_topics_(configuration_);

    // Create Discovery Database
    discovery_database_ = std::make_shared<DiscoveryDatabase>();

    // Create Payload Pool
    payload_pool_ = std::make_shared<FastPayloadPool>();

    // Create Thread Pool
    thread_pool_ = std::make_shared<SlotThreadPool>(configuration_.n_threads);

    // Fill MCAP output file settings
    participants::OutputSettings output_settings;

    if (file_name == "")
    {
        output_settings.filename = configuration_.output_filename;
        output_settings.filepath = configuration_.output_filepath;
        output_settings.prepend_timestamp = true;
        output_settings.timestamp_format = configuration_.output_timestamp_format;
        output_settings.local_timestamp = configuration_.output_local_timestamp;
    }
    else
    {
        output_settings.filename = file_name;
        output_settings.filepath = ".";
        output_settings.prepend_timestamp = false;
    }

    switch (configuration_.output_library)
    {
        case participants::OutputLibrary::mcap:
            output_settings.extension = ".mcap";
            break;
        case participants::OutputLibrary::sql:
            output_settings.extension = ".db";
            break;
        default:
            utils::tsnh(utils::Formatter() << "The library " << configuration_.output_library << " is not valid.");
            break;
    }

    output_settings.safety_margin = configuration_.safety_margin;
    output_settings.file_rotation = configuration_.output_resource_limits_file_rotation;
    output_settings.max_file_size = configuration_.output_resource_limits_max_file_size;

    if (output_settings.max_file_size == 0)
    {
        output_settings.max_file_size = std::filesystem::space(output_settings.filepath).available;
    }

    output_settings.max_size = configuration_.output_resource_limits_max_size;

    if (output_settings.max_size == 0)
    {
        output_settings.max_size = output_settings.max_file_size;
    }

    if (file_tracker == nullptr)
    {
        // Create the File Tracker
        file_tracker.reset(new participants::FileTracker(output_settings));
    }

    const auto handler_state = recorder_to_handler_state_(init_state);
    const auto on_disk_full_lambda = std::bind(&DdsRecorder::on_disk_full, this);

    switch (configuration_.output_library)
    {
        case participants::OutputLibrary::mcap:
        {
            // Create MCAP Handler configuration
            participants::McapHandlerConfiguration handler_config(
                output_settings,
                configuration_.max_pending_samples,
                configuration_.buffer_size,
                configuration_.event_window,
                configuration_.cleanup_period,
                configuration_.log_publish_time,
                configuration_.only_with_type,
                configuration_.mcap_writer_options,
                configuration_.record_types,
                configuration_.ros2_types);

            // Create MCAP Handler
            handler_ = std::make_shared<participants::McapHandler>(
                handler_config,
                payload_pool_,
                file_tracker,
                handler_state,
                on_disk_full_lambda);

            break;
        }
        case participants::OutputLibrary::sql:
        {
            // Create SQL Handler configuration
            participants::SqlHandlerConfiguration handler_config(
                output_settings,
                configuration_.max_pending_samples,
                configuration_.buffer_size,
                configuration_.event_window,
                configuration_.cleanup_period,
                configuration_.log_publish_time,
                configuration_.only_with_type,
                configuration_.record_types,
                configuration_.ros2_types);

            // Create MCAP Handler
            handler_ = std::make_shared<participants::SqlHandler>(
                handler_config,
                payload_pool_,
                file_tracker,
                handler_state,
                on_disk_full_lambda);

            break;
        }
        default:
        {
            utils::tsnh(utils::Formatter() << "The library " << configuration_.output_library << " is not valid.");
            break;
        }
    }


    // Create DynTypes Participant
    dyn_participant_ = std::make_shared<DynTypesParticipant>(
        configuration_.simple_configuration,
        payload_pool_,
        discovery_database_);
    dyn_participant_->init();

    // Create Recorder Participant
    recorder_participant_ = std::make_shared<SchemaParticipant>(
        configuration_.recorder_configuration,
        payload_pool_,
        discovery_database_,
        handler_);

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
        configuration_.ddspipe_configuration,
        discovery_database_,
        payload_pool_,
        participants_database_,
        thread_pool_);

    // Create a Monitor
    auto monitor_configuration = configuration.monitor_configuration;
    monitor_ = std::make_unique<Monitor>(monitor_configuration);

    if (monitor_configuration.producers[eprosima::ddspipe::core::STATUS_MONITOR_PRODUCER_ID].enabled)
    {
        monitor_->monitor_status();
    }

    if (monitor_configuration.producers[eprosima::ddspipe::core::TOPICS_MONITOR_PRODUCER_ID].enabled)
    {
        monitor_->monitor_topics();
    }
}

utils::ReturnCode DdsRecorder::reload_configuration(
        yaml::RecorderConfiguration& new_configuration)
{
    load_internal_topics_(new_configuration);

    // Update the Recorder's configuration
    configuration_ = new_configuration;

    return pipe_->reload_configuration(new_configuration.ddspipe_configuration);
}

void DdsRecorder::start()
{
    handler_->start();
}

void DdsRecorder::pause()
{
    handler_->pause();
}

void DdsRecorder::suspend()
{
    handler_->stop();
}

void DdsRecorder::stop()
{
    handler_->stop();
}

void DdsRecorder::trigger_event()
{
    handler_->trigger_event();
}

void DdsRecorder::on_disk_full()
{
    if (nullptr != event_handler_)
    {
        // Notify main application to proceed and close
        event_handler_->simulate_event_occurred();
    }
}

void DdsRecorder::load_internal_topics_(
        yaml::RecorderConfiguration& configuration)
{
    // Create an internal topic to transmit the dynamic types
    configuration.ddspipe_configuration.builtin_topics.insert(
        utils::Heritable<DdsTopic>::make_heritable(type_object_topic()));

    if (!configuration.ddspipe_configuration.allowlist.empty())
    {
        // The allowlist is not empty. Add the internal topic.
        WildcardDdsFilterTopic internal_topic;
        internal_topic.topic_name.set_value(TYPE_OBJECT_TOPIC_NAME);

        configuration.ddspipe_configuration.allowlist.insert(
            utils::Heritable<WildcardDdsFilterTopic>::make_heritable(internal_topic));
    }
}

participants::BaseHandlerStateCode DdsRecorder::recorder_to_handler_state_(
        const DdsRecorderStateCode& recorder_state)
{
    switch (recorder_state)
    {
        case DdsRecorderStateCode::RUNNING:
            return participants::BaseHandlerStateCode::RUNNING;

        case DdsRecorderStateCode::PAUSED:
            return participants::BaseHandlerStateCode::PAUSED;

        case DdsRecorderStateCode::STOPPED:
        case DdsRecorderStateCode::SUSPENDED:
            return participants::BaseHandlerStateCode::STOPPED;

        default:
            // Unreachable
            utils::tsnh(
                utils::Formatter() << "Trying to convert to McapHandler state an invalid DdsRecorder state.");
            return participants::BaseHandlerStateCode::STOPPED;
    }
}

} /* namespace recorder */
} /* namespace ddsrecorder */
} /* namespace eprosima */
