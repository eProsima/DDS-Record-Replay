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

#include <cpp_utils/event/MultipleEventHandler.hpp>
#include <cpp_utils/ReturnCode.hpp>
#include <cpp_utils/thread_pool/pool/SlotThreadPool.hpp>

#include <ddspipe_core/core/DdsPipe.hpp>
#include <ddspipe_core/dynamic/AllowedTopicList.hpp>
#include <ddspipe_core/dynamic/DiscoveryDatabase.hpp>
#include <ddspipe_core/dynamic/ParticipantsDatabase.hpp>
#include <ddspipe_core/efficiency/payload/FastPayloadPool.hpp>
#include <ddspipe_core/types/topic/dds/DistributedTopic.hpp>

#include <ddspipe_participants/participant/dynamic_types/DynTypesParticipant.hpp>
#include <ddspipe_participants/participant/dynamic_types/SchemaParticipant.hpp>

#include <ddsrecorder_participants/recorder/monitoring/DdsRecorderMonitor.hpp>
#include <ddsrecorder_participants/recorder/output/BaseHandler.hpp>
#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>

#include <ddsrecorder_yaml/recorder/YamlReaderConfiguration.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace recorder {

//! State of the DdsRecorder instance
ENUMERATION_BUILDER(
    DdsRecorderStateCode,
    STOPPED,                  //! Internal entities are not created and thus no messages are received.
    SUSPENDED,                //! Messages are received (internal entities created) but discarded.
    RUNNING,                  //! Messages are stored in MCAP/SQL file.
    PAUSED                    //! Messages are stored in buffer and stored in MCAP/SQL file if event triggered.
    );

/**
 * Wrapper class that encapsulates all dependencies required to launch a DDS Recorder application.
 */
class DdsRecorder
{
public:

    /**
     * DdsRecorder constructor by required values.
     *
     * Creates DdsRecorder instance with given configuration, initial state and mcap file name.
     *
     * @param configuration: Structure encapsulating all recorder configuration options.
     * @param init_state:    Initial instance state (RUNNING/PAUSED/SUSPENDED/STOPPED).
     * @param mcap_file_tracker:  Reference to file tracker used to manage mcap files.
     * @param sql_file_tracker:  Reference to file tracker used to manage sql files.
     * @param file_name:     Name of the mcap file where data is recorded. If not provided, the one from configuration is used instead.
     */
    DdsRecorder(
            const yaml::RecorderConfiguration& configuration,
            const DdsRecorderStateCode& init_state,
            std::shared_ptr<participants::FileTracker>& mcap_file_tracker,
            std::shared_ptr<participants::FileTracker>& sql_file_tracker,
            const std::string& file_name = "");

    /**
     * DdsRecorder constructor by required values and event handler reference.
     *
     * Creates DdsRecorder instance with given configuration, initial state and mcap file name.
     *
     * @param configuration: Structure encapsulating all recorder configuration options.
     * @param init_state:    Initial instance state (RUNNING/PAUSED/SUSPENDED/STOPPED).
     * @param mcap_file_tracker:  Reference to file tracker used to manage mcap files.
     * @param sql_file_tracker:  Reference to file tracker used to manage sql files.
     * @param file_name:     Name of the mcap file where data is recorded. If not provided, the one from configuration is used instead.
     */
    DdsRecorder(
            const yaml::RecorderConfiguration& configuration,
            const DdsRecorderStateCode& init_state,
            std::shared_ptr<utils::event::MultipleEventHandler> event_handler,
            std::shared_ptr<participants::FileTracker>& mcap_file_tracker,
            std::shared_ptr<participants::FileTracker>& sql_file_tracker,
            const std::string& file_name = "");

    /**
     * Reconfigure the Recorder with the new configuration.
     *
     * @param new_configuration: The configuration to replace the previous configuration with.
     *
     * @return \c RETCODE_OK if allowed topics list has been updated correctly
     * @return \c RETCODE_NO_DATA if new allowed topics list is the same as the previous one
     */
    utils::ReturnCode reload_configuration(
            yaml::RecorderConfiguration& new_configuration);

    //! Start recorder (\c mcap_handler_)
    void start();

    //! Pause recorder (\c mcap_handler_)
    void pause();

    //! Suspend recorder (stop \c mcap_handler_)
    void suspend();

    //! Stop recorder (\c mcap_handler_)
    void stop();

    //! Trigger event (in \c mcap_handler_)
    void trigger_event();

    //! Callback to execute when disk is full on mcap criteria
    void mcap_on_disk_full();

    //! Callback to execute when disk is full on sql criteria
    void sql_on_disk_full();

protected:

    /**
     * Load the Recorder's internal topics into a configuration object.
     *
     * @param configuration: The configuration to load the internal topics into.
     */
    void load_internal_topics_(
            yaml::RecorderConfiguration& configuration);

    static participants::BaseHandlerStateCode recorder_to_handler_state_(
            const DdsRecorderStateCode& recorder_state);

    /**
     * Encapsulate the logic to load the resource limits configuration into the output settings.
     * 
     * RESOURCE LIMITS LOGIC
     * A: If only one recorder is enabled
     *  1. If no resource limits are set, the space available will be occupied by the enabled recorder
     *  2. If resource limits are set, just check if the space available and resource limits conflict
     * B: If both recorders are enabled
     *  1. If no resource limits are set, the space available will be divided by half for each recorder
     *  2. If only one resource limits is set, then the other recorder will be set by default to the remaining space
     *  3. If both resource limits are set, just check if the space available and both resource limits altogether conflict
     * 
     * @param mcap_output_settings: Reference to the output settings for the MCAP recorder.
     * @param sql_output_settings: Reference to the output settings for the SQL recorder.
     * @param error_msg: Reference to the error message to be filled in case of error.
     * 
     * @return Error flag if the resource limits are not valid.
     */
    bool load_resource_limits(
            participants::OutputSettings& mcap_output_settings,
            participants::OutputSettings& sql_output_settings,
            utils::Formatter& error_msg);

    //! Configuration of the DDS Recorder
    yaml::RecorderConfiguration configuration_;

    //! Payload Pool
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    //! Thread Pool
    std::shared_ptr<utils::SlotThreadPool> thread_pool_;

    //! Discovery Database
    std::shared_ptr<ddspipe::core::DiscoveryDatabase> discovery_database_;

    //! Participants Database
    std::shared_ptr<ddspipe::core::ParticipantsDatabase> participants_database_;

    //! MCAP Handler
    std::shared_ptr<ddsrecorder::participants::BaseHandler> mcap_handler_;

    //! SQL Handler
    std::shared_ptr<ddsrecorder::participants::BaseHandler> sql_handler_;

    //! Dynamic Types Participant
    std::shared_ptr<ddspipe::participants::DynTypesParticipant> dyn_participant_;

    //! MCAP Schema Participant
    std::shared_ptr<ddspipe::participants::SchemaParticipant> mcap_recorder_participant_;

    //! SQL Schema Participant
    std::shared_ptr<ddspipe::participants::SchemaParticipant> sql_recorder_participant_;
    //! DDS Pipe
    std::unique_ptr<ddspipe::core::DdsPipe> pipe_;

    //! Monitor
    std::unique_ptr<ddspipe::core::Monitor> monitor_;

    //! Reference to event handler used for thread synchronization in main application
    std::shared_ptr<utils::event::MultipleEventHandler> event_handler_;
};

} /* namespace recorder */
} /* namespace ddsrecorder */
} /* namespace eprosima */
