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
        std::shared_ptr<participants::FileTracker>& mcap_file_tracker,
        std::shared_ptr<participants::FileTracker>& sql_file_tracker,
        const std::string& file_name /* = "" */)
    : DdsRecorder(configuration, init_state, nullptr, mcap_file_tracker, sql_file_tracker, file_name)
{
}

DdsRecorder::DdsRecorder(
        const yaml::RecorderConfiguration& configuration,
        const DdsRecorderStateCode& init_state,
        std::shared_ptr<eprosima::utils::event::MultipleEventHandler> event_handler,
        std::shared_ptr<participants::FileTracker>& mcap_file_tracker,
        std::shared_ptr<participants::FileTracker>& sql_file_tracker,
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

    // Fill output file settings
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

    const auto space_available = std::filesystem::space(output_settings.filepath).available;
    output_settings.max_file_size = space_available;
    output_settings.max_size = space_available;

    // Configure the resource-limits depending on sql or mcap
    participants::OutputSettings mcap_output_settings = output_settings;
    participants::OutputSettings sql_output_settings = output_settings;
    if(configuration_.mcap_enabled){
        mcap_output_settings.safety_margin = configuration_.mcap_resource_limits_safety_margin;
        mcap_output_settings.file_rotation = configuration_.mcap_resource_limits_file_rotation;

        if (configuration_.mcap_resource_limits_max_file_size > 0)
        {
            mcap_output_settings.max_file_size = configuration_.mcap_resource_limits_max_file_size;
            mcap_output_settings.max_size = configuration_.mcap_resource_limits_max_file_size;
        }

        if (configuration_.mcap_resource_limits_max_size > 0)
        {
            mcap_output_settings.max_size = configuration_.mcap_resource_limits_max_size;
        }

        mcap_output_settings.extension = ".mcap";
    }

    if(configuration_.sql_enabled){
        sql_output_settings.safety_margin = configuration_.sql_resource_limits_safety_margin;
        sql_output_settings.file_rotation = configuration_.sql_resource_limits_file_rotation;

        if (configuration_.sql_resource_limits_max_file_size > 0)
        {
            sql_output_settings.max_file_size = configuration_.sql_resource_limits_max_file_size;
            sql_output_settings.max_size = configuration_.sql_resource_limits_max_file_size;
        }

        if (configuration_.sql_resource_limits_max_size > 0)
        {
            sql_output_settings.max_size = configuration_.sql_resource_limits_max_size;
        }

        sql_output_settings.extension = ".db";
    }

    if (mcap_file_tracker == nullptr)
    {
        // Create the MCAP File Tracker
        mcap_file_tracker.reset(new participants::FileTracker(mcap_output_settings));
    }
    if(sql_file_tracker == nullptr)
    {
        // Create the SQL File Tracker
        sql_file_tracker.reset(new participants::FileTracker(sql_output_settings));
    }

    const auto handler_state = recorder_to_handler_state_(init_state);
    const auto on_disk_full_lambda = std::bind(&DdsRecorder::on_disk_full, this);

    // Create DynTypes Participant
    dyn_participant_ = std::make_shared<DynTypesParticipant>(
        configuration_.simple_configuration,
        payload_pool_,
        discovery_database_);
    dyn_participant_->init();

    // Create Participant Database
    participants_database_ = std::make_shared<ParticipantsDatabase>();

    // Populate Participant Database
    participants_database_->add_participant(
        dyn_participant_->id(),
        dyn_participant_
        );

    if (configuration_.mcap_enabled)
    {
        // Create MCAP Handler configuration
        participants::McapHandlerConfiguration handler_config(
            mcap_output_settings,
            configuration_.max_pending_samples,
            configuration_.buffer_size,
            configuration_.event_window,
            configuration_.cleanup_period,
            configuration_.mcap_log_publish_time,
            configuration_.only_with_type,
            configuration_.mcap_writer_options,
            configuration_.record_types,
            configuration_.ros2_types);

        // Create MCAP Handler
        mcap_handler_ = std::make_shared<participants::McapHandler>(
            handler_config,
            payload_pool_,
            mcap_file_tracker,
            handler_state,
            on_disk_full_lambda);

        // Create Recorder Participant
        mcap_recorder_participant_ = std::make_shared<SchemaParticipant>(
            configuration_.mcap_recorder_configuration,
            payload_pool_,
            discovery_database_,
            mcap_handler_);

        // Populate Participant Database with the mcap recorder participant
        participants_database_->add_participant(
        mcap_recorder_participant_->id(),
        mcap_recorder_participant_
        );
    }
    if (configuration_.sql_enabled)
    {
        // Create SQL Handler configuration
        participants::SqlHandlerConfiguration handler_config(
            sql_output_settings,
            configuration_.max_pending_samples,
            configuration_.buffer_size,
            configuration_.event_window,
            configuration_.cleanup_period,
            configuration_.only_with_type,
            configuration_.record_types,
            configuration_.ros2_types,
            configuration_.sql_data_format);

        // Create SQL Handler
        sql_handler_ = std::make_shared<participants::SqlHandler>(
            handler_config,
            payload_pool_,
            sql_file_tracker,
            handler_state,
            on_disk_full_lambda);

        // Create Recorder Participant
        sql_recorder_participant_ = std::make_shared<SchemaParticipant>(
            configuration_.sql_recorder_configuration,
            payload_pool_,
            discovery_database_,
            sql_handler_);

        // Populate Participant Database with the sql recorder participant
        participants_database_->add_participant(
        sql_recorder_participant_->id(),
        sql_recorder_participant_
        );
    }

    
    

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
    if(configuration_.sql_enabled)
    {
        sql_handler_->start();
    }
    if(configuration_.mcap_enabled)
    {
        mcap_handler_->start();
    }
}

void DdsRecorder::pause()
{
    if(configuration_.sql_enabled)
    {
        sql_handler_->pause();
    }
    if(configuration_.mcap_enabled)
    {
        mcap_handler_->pause();
    }
}

void DdsRecorder::suspend()
{
    if(configuration_.sql_enabled)
    {
        sql_handler_->stop();
    }
    if(configuration_.mcap_enabled)
    {
        mcap_handler_->stop();
    }
}

void DdsRecorder::stop()
{
    if(configuration_.sql_enabled)
    {
        sql_handler_->stop();
    }
    if(configuration_.mcap_enabled)
    {
        mcap_handler_->stop();
    }
}

void DdsRecorder::trigger_event()
{
    if(configuration_.sql_enabled)
    {
        sql_handler_->trigger_event();
    }
    if(configuration_.mcap_enabled)
    {
        mcap_handler_->trigger_event();
    }
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
