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
 * @file FoxgloveWsWriter.cpp
 */

#include <cpp_utils/Log.hpp>
#include <cpp_utils/exception/InconsistencyException.hpp>

#include <writer/implementations/foxglove_ws/FoxgloveWsWriter.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {

using namespace eprosima::ddsrecorder::core::types;

FoxgloveWsWriter::FoxgloveWsWriter(
        const ParticipantId& participant_id,
        const DdsTopic& topic,
        std::shared_ptr<PayloadPool> payload_pool,
        std::shared_ptr<recorder::FoxgloveWsHandler> foxglove_ws_handler)
    : BaseWriter(participant_id, topic, payload_pool)
    , foxglove_ws_handler_(foxglove_ws_handler)
{
    // Do nothing
}

utils::ReturnCode FoxgloveWsWriter::write_(
        std::unique_ptr<DataReceived>& data) noexcept
{
    logInfo(DDSRECORDER_RECORDER_WRITER,
        "Data in topic: "
        << topic_ << " received: "
        << data->payload
    );

    // Add this data to the mcap handler
    try
    {
        foxglove_ws_handler_->add_data(topic_, data);
    }
    catch(const utils::Exception& e)
    {
        logError(
            DDSRECORDER_RECORDER_WRITER,
            "Error storing data: <" << e.what() << ">.\nContinue recording...");
    }

    return utils::ReturnCode::RETCODE_OK;
}

} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
