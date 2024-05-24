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

#include <map>
#include <memory>
#include <set>
#include <string>

#include <cpp_utils/memory/Heritable.hpp>
#include <cpp_utils/ReturnCode.hpp>
#include <cpp_utils/thread_pool/pool/SlotThreadPool.hpp>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>

#include <ddspipe_core/core/DdsPipe.hpp>
#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>
#include <ddspipe_core/types/topic/Topic.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddsrecorder_participants/replayer/BaseReaderParticipant.hpp>
#include <ddsrecorder_yaml/replayer/YamlReaderConfiguration.hpp>

#if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v1/DynamicTypesCollection.hpp>
#else
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v2/DynamicTypesCollection.hpp>
#endif // if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13

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
     * @brief DdsRecorder constructor by required values.
     *
     * Creates a DdsRecorder instance with a configuration, an initial state, and input file name.
     *
     * @param configuration: Structure encapsulating all replayer configuration options.
     * @param input_file:    File containing the DDS data to be played back.
     *
     * @throw utils::InitializationException if failed to create dynamic participant/publisher.
     */
    DdsReplayer(
            yaml::ReplayerConfiguration& configuration,
            std::string& input_file);

    /**
     * @brief DdsReplayer destructor.
     */
    ~DdsReplayer();

    /**
     * @brief Process the input file.
     */
    void process_file();

    /**
     * @brief Stop the Replayer.
     */
    void stop();

    /**
     * @brief Reconfigure the Replayer with the new configuration.
     *
     * @param new_configuration: The configuration to replace the previous configuration with.
     *
     * @return \c RETCODE_OK if allowed topics list has been updated correctly
     * @return \c RETCODE_NO_DATA if new allowed topics list is the same as the previous one
     */
    utils::ReturnCode reload_configuration(
            const yaml::ReplayerConfiguration& new_configuration);

protected:

    /**
     * @brief Register the dynamic types in the \c dyn_participant_.
     *
     * @param dynamic_types: The dynamic types to be registered.
     * @return The set of registered types.
     */
    std::set<std::string> register_dynamic_types_(
            const participants::DynamicTypesCollection& dynamic_types);

    /**
     * @brief Create the dynamic types' writers for \c topics.
     *
     * @param topics: The topics to create the writers for.
     * @param registered_types: The types to create the writers for.
     */
    void create_dynamic_types_writers_(
            const std::set<utils::Heritable<ddspipe::core::types::DdsTopic>>& topics,
            const std::set<std::string>& registered_types);

    /**
     * @brief Create the dynamic types' writer for \c topic.
     *
     * @param topic: The topic to create the writer for.
     */
    void create_dynamic_type_writer_(
            utils::Heritable<ddspipe::core::types::DdsTopic> topic);

    //! Replayer Configuration
    const yaml::ReplayerConfiguration configuration_;

    //! Payload Pool
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    //! Thread Pool
    std::shared_ptr<utils::SlotThreadPool> thread_pool_;

    //! Reader Participant
    std::shared_ptr<participants::BaseReaderParticipant> reader_participant_;

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
