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

#include <ddsrecorder_participants/recorder/handler/mcap/McapHandler.hpp>
#include <ddsrecorder_participants/recorder/handler/mcap/McapHandlerConfiguration.hpp>
#include <ddsrecorder_participants/recorder/handler/BaseHandler.hpp>
#include <ddsrecorder_participants/recorder/output/OutputSettings.hpp>
#include <ddsrecorder_participants/recorder/handler/sql/SqlHandler.hpp>
#include <ddsrecorder_participants/recorder/handler/sql/SqlHandlerConfiguration.hpp>

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
        : configuration_(configuration),
        event_handler_(event_handler)
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

    // Configure the resource-limits depending on sql or mcap
    participants::OutputSettings mcap_output_settings = output_settings;
    participants::OutputSettings sql_output_settings = output_settings;
    mcap_output_settings.extension = ".mcap";
    sql_output_settings.extension = ".db";
    utils::Formatter error_msg;
    if(!load_resource_limits(mcap_output_settings, sql_output_settings, error_msg))
    {
        EPROSIMA_LOG_ERROR(DDSRECORDER, "Error loading resource limits: " << error_msg);
        throw InitializationException("Error loading resource limits, not enough available space");
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
    const auto mcap_on_disk_full_lambda = std::bind(&DdsRecorder::mcap_on_disk_full, this);
    const auto sql_on_disk_full_lambda = std::bind(&DdsRecorder::sql_on_disk_full, this);

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
            mcap_on_disk_full_lambda);

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
            sql_on_disk_full_lambda);

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

void DdsRecorder::mcap_on_disk_full()
{
    if (nullptr != event_handler_)
    {
        configuration_.mcap_enabled = false;
        if(!configuration_.sql_enabled)
        {
            // Notify main application to proceed and close
            event_handler_->simulate_event_occurred();
        }
    }
}

void DdsRecorder::sql_on_disk_full()
{
    if (nullptr != event_handler_)
    {
        configuration_.sql_enabled = false;
        if(!configuration_.mcap_enabled)
        {
            // Notify main application to proceed and close
            event_handler_->simulate_event_occurred();
        }
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

bool DdsRecorder::load_resource_limits(
        participants::OutputSettings& mcap_output_settings,
        participants::OutputSettings& sql_output_settings,
        utils::Formatter& error_msg)
{
    /**
     * RESOURCE LIMITS CONFIGURATION
     * A: If only one recorder is enabled
     *  1. If no resource limits are set, the space available will be occupied by the enabled recorder
     *  2. If resource limits are set, just check if the space available and resource limits conflict
     * B: If both recorders are enabled
     *  1. If no resource limits are set, the space available will be divided by half for each recorder
     *  2. If only one resource limits is set, then the other recorder will be set by default to the remaining space
     *  3. If both resource limits are set, just check if the space available and both resource limits altogether conflict
     */

    bool mcap_size_limited = (mcap_output_settings.resource_limits.max_size_ > 0);
    bool sql_size_limited = (sql_output_settings.resource_limits.max_size_ > 0);

    std::uint64_t space_available = std::filesystem::space(mcap_output_settings.filepath).available - configuration_.output_safety_margin;
    if(space_available < 0)
    {
        error_msg << "The available space is lower than the safety margin.";
        return 0;
    }

    // Case A
    if(configuration_.mcap_enabled ^ configuration_.sql_enabled)
    {
        if(configuration_.mcap_enabled)
        {
            // Subcase 2
            if(mcap_size_limited)
            {
                if(!mcap_output_settings.set_resource_limits(configuration_.mcap_resource_limits.resource_limits_struct, space_available))
                {
                    error_msg << "The available space given the MCAP conditions is lower than the safety margin.";
                    return 0;
                }
            }
            // Subcase 1
            else{
                mcap_output_settings.set_resource_limits(configuration_.mcap_resource_limits.resource_limits_struct, space_available);
            }
        }
        else
        {
            // Subcase 2
            if(sql_size_limited)
            {
                if(!sql_output_settings.set_resource_limits(configuration_.sql_resource_limits.resource_limits_struct, space_available))
                {
                    error_msg << "The available space given the SQL conditions is lower than the safety margin.";
                    return 0;
                }
            }
            // Subcase 1
            else{
                sql_output_settings.set_resource_limits(configuration_.sql_resource_limits.resource_limits_struct, space_available);
            }
        }
    }
    // Case B
    else{
        // Subcase 1
        if(!mcap_size_limited && !sql_size_limited)
        {
            EPROSIMA_LOG_WARNING(DDSRECORDER, "Both MCAP and SQL are enabled but no resource limits are set. Defaulting to half of the available space for each.");
            mcap_output_settings.set_resource_limits(configuration_.mcap_resource_limits.resource_limits_struct, space_available/2);
            sql_output_settings.set_resource_limits(configuration_.sql_resource_limits.resource_limits_struct, space_available/2);
        }
        // Subcase 2
        else if(mcap_size_limited ^ sql_size_limited)
        {
            if(mcap_size_limited)
            {
                if(mcap_output_settings.resource_limits.max_size_ == 0)
                {
                    EPROSIMA_LOG_WARNING(DDSRECORDER, "MCAP resource limits are set but no max size is set. Defaulting to half of the available space for MCAP and SQL.");
                    mcap_output_settings.resource_limits.max_size_ = space_available/2;
                }
                if(!mcap_output_settings.set_resource_limits(configuration_.mcap_resource_limits.resource_limits_struct, space_available))
                {
                    error_msg << "The available space given the MCAP conditions is lower than the safety margin.";
                    return 0;
                }
                sql_output_settings.set_resource_limits(configuration_.sql_resource_limits.resource_limits_struct, space_available - mcap_output_settings.resource_limits.max_size_);
            }
            else
            {
                if(sql_output_settings.resource_limits.max_size_ == 0)
                {
                    EPROSIMA_LOG_WARNING(DDSRECORDER, "SQL resource limits are set but no max size is set. Defaulting to half of the available space for MCAP and SQL.");
                    sql_output_settings.resource_limits.max_size_ = space_available/2;
                }
                if(!sql_output_settings.set_resource_limits(configuration_.sql_resource_limits.resource_limits_struct, space_available))
                {
                    error_msg << "The available space given the SQL conditions is lower than the safety margin.";
                    return 0;
                }
                mcap_output_settings.set_resource_limits(configuration_.mcap_resource_limits.resource_limits_struct, space_available - sql_output_settings.resource_limits.max_size_);
            }
        }
        // Subcase 3
        else
        {
            // If any has max_size_ defined, it will be initialized first and the remaining space available will be used for the other
            participants::OutputSettings* first_output_settings = &mcap_output_settings;
            participants::OutputSettings* second_output_settings = &sql_output_settings;
            participants::ResourceLimitsStruct* first_resource_limits = &configuration_.mcap_resource_limits.resource_limits_struct;
            participants::ResourceLimitsStruct* second_resource_limits = &configuration_.sql_resource_limits.resource_limits_struct;
            if(mcap_output_settings.resource_limits.max_size_ == 0)
            {
                first_output_settings = &sql_output_settings;
                second_output_settings = &mcap_output_settings;
                first_resource_limits = &configuration_.sql_resource_limits.resource_limits_struct;
                second_resource_limits = &configuration_.mcap_resource_limits.resource_limits_struct;
            }

            if(!first_output_settings->set_resource_limits(*first_resource_limits, space_available))
            {
                error_msg << "The available space is lower than the safety margin.";
                return 0;
            }
            space_available -= first_resource_limits->max_size_;
            if(!second_output_settings->set_resource_limits(*second_resource_limits, space_available))
            {
                error_msg << "The available space is lower than the safety margin.";
                return 0;
            }
        }
    }

    return 1;
}

} /* namespace recorder */
} /* namespace ddsrecorder */
} /* namespace eprosima */
