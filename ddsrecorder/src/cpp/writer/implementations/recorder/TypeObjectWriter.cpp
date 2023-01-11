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
 * @file TypeObjectWriter.cpp
 */

#include <cpp_utils/Log.hpp>

#include <writer/implementations/recorder/TypeObjectWriter.hpp>
#include <recorder/types.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {

using namespace eprosima::ddsrecorder::core::types;

utils::ReturnCode TypeObjectWriter::write_(
        std::unique_ptr<DataReceived>& data) noexcept
{
    // TODO(recorder) Do something with the Dynamic type
    auto type_name = recorder::string_deserialization(data);
    auto type_object = recorder::type_object_from_name(type_name);
    if (nullptr == type_object){
        logError(DDSRECORDER_DYNTYPES, "Type " << type_name << " is not present in TypeObjectFactory");
        return utils::ReturnCode::RETCODE_PRECONDITION_NOT_MET;
    }

    logInfo(DDSRECORDER_RECORDER_WRITER,
        "Type Object received: "
        << type_name
    );
    logError(DEBUG,
        "Type Object received: "
        << type_name
    );

    return utils::ReturnCode::RETCODE_OK;
}

} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
