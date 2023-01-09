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

#include <fastrtps/types/DynamicTypePtr.h>
#include <fastrtps/types/DynamicType.h>

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
    // eprosima::fastrtps::types::DynamicType* type = recorder::type_object_data_deserialization(data);
    eprosima::fastrtps::types::TypeObject* type_obj = recorder::actual_type_object_data_deserialization(data);

    // logInfo(DDSRECORDER_RECORDER_WRITER,
    //     "Type Object received: "
    //     << type->get_name()
    // );
    // logError(DDSRECORDER_RECORDER_WRITER,
    //     "Type Object received: "
    //     << type->get_name()
    // ); // TODO(recorder) remove

    logInfo(DDSRECORDER_RECORDER_WRITER,
        "Type Object received");
    logError(DDSRECORDER_RECORDER_WRITER,
        "Type Object received");

    if (type_obj == nullptr)
    {
        std::cout << "LLEGÓ PERO MAL" << std::endl;
    }

    // delete type_obj;

    // TODO(recorder) if DynamicType_ptr is required, fight yourself to achieve it.
    // But probably cant happen. Solutions:
    // 1. Why inherit from shared_ptr, wtf dynamic types?
    // 2. Why DynamicType has no copy constructor?

    return utils::ReturnCode::RETCODE_OK;
}

} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
