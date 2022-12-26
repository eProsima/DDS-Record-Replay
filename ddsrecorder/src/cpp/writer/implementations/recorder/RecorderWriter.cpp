// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file RecorderWriter.cpp
 */

#include <cpp_utils/Log.hpp>

#include <writer/implementations/recorder/RecorderWriter.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {

using namespace eprosima::ddsrecorder::core::types;

utils::ReturnCode RecorderWriter::write_(
        std::unique_ptr<DataReceived>& data) noexcept
{
    // TODO Do something with the serialized data
    logInfo(DDSRECORDER_RECORDER_WRITER,
        "Data in topic: "
        << topic_ << " received: "
        << data->payload
    );
    logUser(DDSRECORDER_RECORDER_WRITER,
        "Data in topic: "
        << topic_ << " received: "
        << data->payload
    ); // TODO(recorder) remove

    return utils::ReturnCode::RETCODE_OK;
}

} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
