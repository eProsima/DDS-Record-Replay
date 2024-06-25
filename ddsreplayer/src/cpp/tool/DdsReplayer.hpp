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

#include <cpp_utils/memory/Heritable.hpp>
#include <cpp_utils/ReturnCode.hpp>
#include <cpp_utils/thread_pool/pool/SlotThreadPool.hpp>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>

#include <ddspipe_core/core/DdsPipe.hpp>
#include <ddspipe_core/dynamic/AllowedTopicList.hpp>
#include <ddspipe_core/dynamic/DiscoveryDatabase.hpp>
#include <ddspipe_core/dynamic/ParticipantsDatabase.hpp>
#include <ddspipe_core/efficiency/payload/FastPayloadPool.hpp>
#include <ddspipe_core/types/dds/TopicQoS.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <yaml-cpp/yaml.h>

#if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v1/DynamicTypesCollection.hpp>
#else
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v2/DynamicTypesCollection.hpp>
#endif // if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13


#include <ddsrecorder_participants/replayer/McapReaderParticipant.hpp>
#include <ddsrecorder_participants/replayer/ReplayerParticipant.hpp>

#include <ddsrecorder_yaml/replayer/YamlReaderConfiguration.hpp>

#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>

#include <yaml-cpp/yaml.h>

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
     *
     * @throw utils::InitializationException if failed to create dynamic participant/publisher.
     */
    DdsReplayer(
            yaml::ReplayerConfiguration& configuration,
            std::string& input_file,
            int domain = 0);

    /**
     * @brief Destructor
     *
     * Removes all DDS resources created via the \c fastdds::DomainParticipantFactory
     */
    ~DdsReplayer();

    /**
     * Reconfigure the Replayer with the new configuration.
     *
     * @param new_configuration: The configuration to replace the previous configuration with.
     *
     * @return \c RETCODE_OK if allowed topics list has been updated correctly
     * @return \c RETCODE_NO_DATA if new allowed topics list is the same as the previous one
     */
    utils::ReturnCode reload_configuration(
            const yaml::ReplayerConfiguration& new_configuration);

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
     * @throw utils::InitializationException if failed to read mcap file.
     */
    std::set<utils::Heritable<ddspipe::core::types::DistributedTopic>> generate_builtin_topics_(
            const yaml::ReplayerConfiguration& configuration,
            std::string& input_file);

    /**
     * @brief Deserialize and register \c dynamic_type into \c TypeObjectFactory .
     *
     * @param dynamic_type: serialized dynamic type to be registered.
     */
    void register_dynamic_type_(
            const ddsrecorder::participants::DynamicType& dynamic_type);

    /**
     * @brief Create DDS DataWriter in given topic to send associated dynamic type information to applications relying
     * on dynamic types (e.g. applications which dynamically create DataReaders every time a participant receives a
     * dynamic type via on_type_discovery/on_type_information_received callbacks).
     *
     * @param topic: topic on which DataWriter is created.
     */
    void create_dynamic_writer_(
            utils::Heritable<ddspipe::core::types::DdsTopic> topic);

    /**
     * @brief Deserialize a serialized \c TopicQoS string.
     *
     * @param [in] qos_str Serialized \c TopicQoS string
     * @return Deserialized TopicQoS
     */
    static ddspipe::core::types::TopicQoS deserialize_qos_(
            const std::string& qos_str);

    /**
     * @brief Deserialize a serialized \c TypeIdentifier string.
     *
     * @param [in] typeid_str Serialized \c TypeIdentifier string
     * @return Deserialized TypeIdentifier
     */
    static fastrtps::types::TypeIdentifier deserialize_type_identifier_(
            const std::string& typeid_str);

    /**
     * @brief Deserialize a serialized \c TypeObject string.
     *
     * @param [in] typeobj_str Serialized \c TypeObject string
     * @return Deserialized TypeObject
     */
    static fastrtps::types::TypeObject deserialize_type_object_(
            const std::string& typeobj_str);

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

    //! Dynamic DDS DomainParticipant
    fastdds::dds::DomainParticipant* dyn_participant_;

    //! Dynamic DDS Publisher
    fastdds::dds::Publisher* dyn_publisher_;

    //! Dynamic DDS Topics map
    std::map<utils::Heritable<ddspipe::core::types::DdsTopic>, fastdds::dds::Topic*> dyn_topics_;

    //! Dynamic DDS DataWriters map
    std::map<utils::Heritable<ddspipe::core::types::DdsTopic>, fastdds::dds::DataWriter*> dyn_writers_;
};

} /* namespace replayer */
} /* namespace ddsrecorder */
} /* namespace eprosima */
