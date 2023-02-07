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

/**
 * @file RecorderWriter.hpp
 */

#pragma once

#include <condition_variable>
#include <mutex>

#include <cpp_utils/time/time_utils.hpp>

#include <ddsrouter_core/participants/writer/auxiliar/BaseWriter.hpp>

#include <ddsrecorder_participants/auxiliar/mcap/McapHandler.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Writer implementation that allows to Read custom data produced internally.
 */
class RecorderWriter : public ddsrouter::participants::BaseWriter
{
public:

    RecorderWriter(
        const ddsrouter::core::types::ParticipantId& participant_id,
        const ddsrouter::core::types::DdsTopic& topic,
        std::shared_ptr<ddsrouter::core::PayloadPool> payload_pool,
        std::shared_ptr<McapHandler> mcap_handler);

protected:

    /**
     * @brief Write specific method
     *
     * @param data : data to simulate publication
     * @return RETCODE_OK always
     */
    utils::ReturnCode write_(
            std::unique_ptr<ddsrouter::core::types::DataReceived>& data) noexcept override;

    std::shared_ptr<McapHandler> mcap_handler_;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
