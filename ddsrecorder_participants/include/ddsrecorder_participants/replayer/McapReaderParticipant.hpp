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
 * TODO
 */
class McapReaderParticipant : public ddspipe::core::IParticipant
{
public:

    // TODO
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

    DDSRECORDER_PARTICIPANTS_DllAPI
    void process_mcap();

    DDSRECORDER_PARTICIPANTS_DllAPI
    void stop() noexcept;

    DDSRECORDER_PARTICIPANTS_DllAPI
    static utils::Timestamp mcap_timestamp_to_std_timepoint(
            const mcap::Timestamp& time);

    DDSRECORDER_PARTICIPANTS_DllAPI
    static mcap::Timestamp std_timepoint_to_mcap_timestamp(
            const utils::Timestamp& time);

protected:

    //! Participant Configuration
    std::shared_ptr<McapReaderParticipantConfiguration> configuration_;

    //! DDS Pipe shared Payload Pool
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    std::string file_path_;

    std::map<ddspipe::core::types::DdsTopic, std::shared_ptr<ddspipe::participants::InternalReader>> readers_;

    bool stop_;
    std::condition_variable scheduling_cv_;
    std::mutex scheduling_cv_mtx_;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
