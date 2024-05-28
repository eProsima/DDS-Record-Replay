// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <condition_variable>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>

#include <ddspipe_core/interface/IReader.hpp>
#include <ddspipe_core/interface/IWriter.hpp>

#include <cpp_utils/memory/Heritable.hpp>
#include <cpp_utils/time/time_utils.hpp>

#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>
#include <ddspipe_core/interface/IParticipant.hpp>
#include <ddspipe_core/interface/ITopic.hpp>
#include <ddspipe_core/types/data/RtpsPayloadData.hpp>
#include <ddspipe_core/types/dds/TopicQoS.hpp>
#include <ddspipe_core/types/participant/ParticipantId.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddspipe_participants/reader/auxiliar/InternalReader.hpp>

#include <ddsrecorder_participants/library/library_dll.h>
#include <ddsrecorder_participants/replayer/BaseReaderParticipantConfiguration.hpp>

#if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v1/DynamicTypesCollection.hpp>
#else
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v2/DynamicTypesCollection.hpp>
#endif // if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Participant that reads files and passes its messages to other DDS Pipe participants.
 *
 * @implements IParticipant
 */
class BaseReaderParticipant : public ddspipe::core::IParticipant
{
public:

    /**
     * BaseReaderParticipant constructor by required values.
     *
     * Creates BaseReaderParticipant instance with given configuration, payload pool and input file path.
     *
     * @param config:       Structure encapsulating all configuration options.
     * @param payload_pool: Owner of every payload contained in sent messages.
     * @param file_path:    Path to the file with the messages to be read and sent.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    BaseReaderParticipant(
            const std::shared_ptr<BaseReaderParticipantConfiguration>& configuration,
            const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
            const std::string& file_path);

    //! Override id() IParticipant method
    DDSRECORDER_PARTICIPANTS_DllAPI
    ddspipe::core::types::ParticipantId id() const noexcept override;

    //! Override is_repeater() IParticipant method
    DDSRECORDER_PARTICIPANTS_DllAPI
    bool is_repeater() const noexcept override;

    //! Override is_rtps_kind() IParticipant method
    DDSRECORDER_PARTICIPANTS_DllAPI
    bool is_rtps_kind() const noexcept override;

    //! Override topic_qos() IParticipant method
    DDSRECORDER_PARTICIPANTS_DllAPI
    ddspipe::core::types::TopicQoS topic_qos() const noexcept override;

    //! Override create_writer_() IParticipant method
    DDSRECORDER_PARTICIPANTS_DllAPI
    std::shared_ptr<ddspipe::core::IWriter> create_writer(
            const ddspipe::core::ITopic& topic) override;

    //! Override create_reader_() IParticipant method
    DDSRECORDER_PARTICIPANTS_DllAPI
    std::shared_ptr<ddspipe::core::IReader> create_reader(
            const ddspipe::core::ITopic& topic) override;

    /**
     * @brief Process the input file's summary.
     *
     * @param topics: Set of topics to be filled with the information from the input file.
     * @param types:  DynamicTypesCollection instance to be filled with the types information from the input file.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    virtual void process_summary(
        std::set<utils::Heritable<ddspipe::core::types::DdsTopic>>& topics,
        DynamicTypesCollection& types) = 0;

    /**
     * @brief Process the input file's messages
     *
     * Reads and sends messages sequentially (according to timestamp).
     *
     * @throw \c InconsistencyException if failed to read the input file.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    virtual void process_messages() = 0;

    //! Stop participant (abort processing file)
    DDSRECORDER_PARTICIPANTS_DllAPI
    void stop() noexcept;

protected:

    /**
     * @brief Create a payload from raw data.
     *
     * @param raw_data: Raw data to be encapsulated in the payload.
     * @param raw_data_size: Size of the raw data.
     */
    std::unique_ptr<ddspipe::core::types::RtpsPayloadData> create_payload_(
            const void* raw_data,
            const std::uint32_t raw_data_size);

    /**
     * @brief Create a new \c DdsTopic instance.
     *
     * @param topic_name: Name of the topic.
     * @param type_name:  Name of the type.
     * @param is_ros2_type: Whether the type is a ROS2 type.
     * @return A new \c DdsTopic instance.
     */
    ddspipe::core::types::DdsTopic create_topic_(
        const std::string& topic_name,
        const std::string& type_name,
        const bool is_ros2_type);

    /**
     * @brief Given a fuzzy timestamp, return the timestamp to start replaying.
     *
     * It returns the current timestamp if \c start_replay_time is not set or if it is in the past.
     * Otherwise, it returns \c start_replay_time.
     *]
     * @param start_replay_time: Fuzzy timestamp to start replaying.
     * @return The timestamp to start replaying.
     */
    static utils::Timestamp when_to_start_replay_(
            const utils::Fuzzy<utils::Timestamp>& start_replay_time);

    /**
     * @brief Wait until timestamp is reached.
     *
     * @param timestamp: Timestamp to wait until.
     */
    void wait_until_timestamp_(
            const utils::Timestamp& timestamp);

    //! Participant Configuration
    const std::shared_ptr<BaseReaderParticipantConfiguration> configuration_;

    //! DDS Pipe shared Payload Pool
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    //! Input file path
    const std::string file_path_;

    //! Internal readers map
    std::map<ddspipe::core::types::DdsTopic, std::shared_ptr<ddspipe::participants::InternalReader>> readers_;

    //! Stop flag
    bool stop_;

    //! Scheduling condition variable
    std::condition_variable scheduling_cv_;

    //! Scheduling condition variable mutex
    std::mutex scheduling_cv_mtx_;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
