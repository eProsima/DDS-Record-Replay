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
#include <ddspipe_core/types/dds/TopicQoS.hpp>
#include <ddspipe_core/types/topic/dds/DistributedTopic.hpp>

#include <ddsrecorder_participants/replayer/McapReaderParticipant.hpp>
#include <ddsrecorder_participants/replayer/ReplayerParticipant.hpp>

#include <ddsrecorder_yaml/replayer/YamlReaderConfiguration.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace replayer {

/**
 * Wrapper class that encapsulates all dependencies required to launch a DDS Replayer application.
 */
class DdsReplayer
{
public:

    /**
     * DdsRecorder constructor by required values.
     *
     * Creates DdsRecorder instance with given configuration, initial state and mcap file name.
     *
     * @param configuration: Structure encapsulating all replayer configuration options.
     * @param input_file:    MCAP file containing the messages to be played back.
     */
    DdsReplayer(
            const yaml::ReplayerConfiguration& configuration,
            std::string& input_file);

    /**
     * Reload allowed topics list.
     *
     * @param allowed_topics: Allowed topics list to be loaded.
     *
     * @return \c RETCODE_OK if allowed topics list has been updated correctly
     * @return \c RETCODE_NO_DATA if new allowed topics list is the same as the previous one
     */
    utils::ReturnCode reload_allowed_topics(
            const std::shared_ptr<ddspipe::core::AllowedTopicList>& allowed_topics);

    //! Process input MCAP file
    void process_mcap();

    //! Stop replayer instance
    void stop();

protected:

    /**
     * @brief Generate a builtin-topics list by combining the channels information within the MCAP file and the
     * optional builtin-topics list provided via YAML configuration file.
     *
     * @param configuration: replayer config containing, among other specs, the YAML-provided builtin-topics list.
     * @param input_file: path to the input MCAP file.
     *
     * @return generated builtin-topics list (set).
     */
    std::set<utils::Heritable<ddspipe::core::types::DistributedTopic>> generate_builtin_topics_(
            const yaml::ReplayerConfiguration& configuration,
            std::string& input_file);

    /**
     * @brief Deserialize a serialized \c TopicQoS string.
     *
     * @param [in] qos_str Serialized \c TopicQoS string
     * @return Deserialized TopicQoS
     */
    ddspipe::core::types::TopicQoS deserialize_qos_(
            const std::string& qos_str);

    //! Payload Pool
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    //! Thread Pool
    std::shared_ptr<utils::SlotThreadPool> thread_pool_;

    //! Discovery Database
    std::shared_ptr<ddspipe::core::DiscoveryDatabase> discovery_database_;

    //! Participants Database
    std::shared_ptr<ddspipe::core::ParticipantsDatabase> participants_database_;

    //! Replayer Participant
    std::shared_ptr<participants::ReplayerParticipant> replayer_participant_;

    //! MCAP Reader Participant
    std::shared_ptr<participants::McapReaderParticipant> mcap_reader_participant_;

    //! DDS Pipe
    std::unique_ptr<ddspipe::core::DdsPipe> pipe_;
};

} /* namespace replayer */
} /* namespace ddsrecorder */
} /* namespace eprosima */