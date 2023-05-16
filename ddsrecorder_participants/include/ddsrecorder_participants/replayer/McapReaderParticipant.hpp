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

#include <condition_variable>
#include <map>

#include <mcap/mcap.hpp>

#include <cpp_utils/time/time_utils.hpp>

#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>
#include <ddspipe_core/interface/IParticipant.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddspipe_participants/reader/auxiliar/InternalReader.hpp>

#include <ddsrecorder_participants/library/library_dll.h>
#include <ddsrecorder_participants/replayer/McapReaderParticipantConfiguration.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Participant that reads MCAP files and passes its messages to other DDS Pipe participants.
 *
 * @implements IParticipant
 */
class McapReaderParticipant : public ddspipe::core::IParticipant
{
public:

    /**
     * McapReaderParticipant constructor by required values.
     *
     * Creates McapReaderParticipant instance with given configuration, payload pool and input file path.
     *
     * @param config:       Structure encapsulating all configuration options.
     * @param payload_pool: Owner of every payload contained in sent messages.
     * @param file_path:    Path to the MCAP file with the messages to be read and sent.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    McapReaderParticipant(
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
     * @throw utils::InconsistencyException if failed to read mcap file.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void process_mcap();

    //! Stop participant (abort processing mcap)
    DDSRECORDER_PARTICIPANTS_DllAPI
    void stop() noexcept;

    /**
     * @brief This method converts a mcap timestamp to standard format.
     *
     * @param [in] time Timestamp to be converted
     * @return Timestamp in standard format
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    static utils::Timestamp mcap_timestamp_to_std_timepoint(
            const mcap::Timestamp& time);

    /**
     * @brief This method converts a timestamp in standard format to its mcap equivalent.
     *
     * @param [in] time Timestamp to be converted
     * @return Timestamp in mcap format
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    static mcap::Timestamp std_timepoint_to_mcap_timestamp(
            const utils::Timestamp& time);

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
