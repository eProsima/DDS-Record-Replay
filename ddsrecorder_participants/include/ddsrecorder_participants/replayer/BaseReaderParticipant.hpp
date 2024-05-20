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
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>
#include <ddspipe_core/interface/IParticipant.hpp>
#include <ddspipe_core/interface/ITopic.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddspipe_participants/reader/auxiliar/InternalReader.hpp>

#include <ddsrecorder_participants/library/library_dll.h>
#include <ddsrecorder_participants/replayer/McapReaderParticipantConfiguration.hpp>

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
            std::shared_ptr<McapReaderParticipantConfiguration> configuration,
            std::shared_ptr<ddspipe::core::PayloadPool> payload_pool,
            std::string& file_path);

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
     * @brief Read and send messages sequentially (according to timestamp).
     *
     * @throw utils::InconsistencyException if failed to read file.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    virtual void process_file() = 0;

    //! Stop participant (abort processing file)
    DDSRECORDER_PARTICIPANTS_DllAPI
    void stop() noexcept;

protected:

    //! Participant Configuration
    std::shared_ptr<McapReaderParticipantConfiguration> configuration_;

    //! DDS Pipe shared Payload Pool
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    //! Input file path
    std::string file_path_;

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
