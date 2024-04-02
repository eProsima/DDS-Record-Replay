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

#include <ddsrecorder_participants/recorder/mcap/McapHandler.hpp>
#include <ddsrecorder_participants/recorder/mcap/McapHandlerConfiguration.hpp>
#include <ddsrecorder_participants/recorder/monitoring/DdsRecorderMonitor.hpp>

#include <ddsrecorder_yaml/recorder/YamlReaderConfiguration.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace recorder {

//! State of the DdsRecorder instance
ENUMERATION_BUILDER(
    DdsRecorderStateCode,
    STOPPED,                  //! Internal entities are not created and thus no messages are received.
    SUSPENDED,                //! Messages are received (internal entities created) but discarded.
    RUNNING,                  //! Messages are stored in MCAP file.
    PAUSED                    //! Messages are stored in buffer and stored in MCAP file if event triggered.
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
     * @param file_name:     Name of the mcap file where data is recorded. If not provided, the one from configuration is used instead.
     */
    DdsRecorder(
            const yaml::RecorderConfiguration& configuration,
            const DdsRecorderStateCode& init_state,
            const std::string& file_name = "");

    /**
     * DdsRecorder constructor by required values and event handler reference.
     *
     * Creates DdsRecorder instance with given configuration, initial state and mcap file name.
     *
     * @param configuration: Structure encapsulating all recorder configuration options.
     * @param init_state:    Initial instance state (RUNNING/PAUSED/SUSPENDED/STOPPED).
     * @param event_handler: Reference to event handler used for thread synchronization in main application.
     * @param file_name:     Name of the mcap file where data is recorded. If not provided, the one from configuration is used instead.
     */
    DdsRecorder(
            const yaml::RecorderConfiguration& configuration,
            const DdsRecorderStateCode& init_state,
            std::shared_ptr<eprosima::utils::event::MultipleEventHandler> event_handler,
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

    //! Callback to execute when disk is full
    void on_disk_full();

protected:

    /**
     * Load the Recorder's internal topics into a configuration object.
     *
     * @param configuration: The configuration to load the internal topics into.
     */
    void load_internal_topics_(
            yaml::RecorderConfiguration& configuration);

    static participants::McapHandlerStateCode recorder_to_handler_state_(
            const DdsRecorderStateCode& recorder_state);

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
    std::shared_ptr<eprosima::ddsrecorder::participants::McapHandler> mcap_handler_;

    //! Dynamic Types Participant
    std::shared_ptr<eprosima::ddspipe::participants::DynTypesParticipant> dyn_participant_;

    //! Schema Participant
    std::shared_ptr<eprosima::ddspipe::participants::SchemaParticipant> recorder_participant_;

    //! DDS Pipe
    std::unique_ptr<ddspipe::core::DdsPipe> pipe_;

    //! Monitor
    std::unique_ptr<ddspipe::core::Monitor> monitor_;

    //! Reference to event handler used for thread synchronization in main application
    std::shared_ptr<eprosima::utils::event::MultipleEventHandler> event_handler_;
};

} /* namespace recorder */
} /* namespace ddsrecorder */
} /* namespace eprosima */
