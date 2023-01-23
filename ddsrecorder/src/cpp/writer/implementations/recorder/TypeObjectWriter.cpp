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
#include <recorder/dynamic_types/types.hpp>
#include <recorder/dynamic_types/schema.hpp>
#include <recorder/dynamic_types/utils.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {

using namespace eprosima::ddsrecorder::core::types;

TypeObjectWriter::TypeObjectWriter(
        const ParticipantId& participant_id,
        const DdsTopic& topic,
        std::shared_ptr<PayloadPool> payload_pool,
        std::shared_ptr<recorder::McapHandler> mcap_handler)
    : BaseWriter(participant_id, topic, payload_pool)
    , mcap_handler_(mcap_handler)
{
    // Do nothing
}

utils::ReturnCode TypeObjectWriter::write_(
        std::unique_ptr<DataReceived>& data) noexcept
{
    auto type_name = recorder::string_deserialization(data);
    auto dyn_type = recorder::dynamic_type_from_name(type_name);
    if (nullptr == dyn_type){
        logError(DDSRECORDER_DYNTYPES, "Type " << type_name << " is not present in TypeObjectFactory");
        return utils::ReturnCode::RETCODE_PRECONDITION_NOT_MET;
    }

    logInfo(DDSRECORDER_RECORDER_WRITER,
        "Type Object received: "
        << type_name
    );

    // Add schema
    try
    {
        // TODO: this will call multiple times to generate_type_object_schema unnecesary
        // Add this type object as a new schema
        mcap_handler_->add_schema(type_name, recorder::generate_dyn_type_schema(dyn_type));

        logUser(DDSRECORDER_RECORDER_WRITER,
            "Type Object " << type_name << " stored.");
    }
    catch(const utils::Exception& e)
    {
        logError(
            DDSRECORDER_RECORDER_WRITER,
            "Error generating schema: <" << e.what() << ">.\nContinue recording...");
    }

    return utils::ReturnCode::RETCODE_OK;
}

} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
