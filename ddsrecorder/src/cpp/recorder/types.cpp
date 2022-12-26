// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file types.cpp
 */

#include <fastrtps/types/DynamicTypePtr.h>
#include <fastrtps/types/DynamicType.h>

#include <ddsrecorder/types/dds/Data.hpp>

#include <recorder/types.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {
namespace recorder {

types::DdsTopic type_object_topic()
{
    // TODO(recorder) explicitly define type object topic qos
    types::TopicQoS qos;
    qos.durability_qos = eprosima::ddsrecorder::core::types::DurabilityKind::TRANSIENT_LOCAL;
    qos.reliability_qos = eprosima::ddsrecorder::core::types::ReliabilityKind::RELIABLE;

    return types::DdsTopic(
        TYPE_OBJECT_TOPIC_NAME,
        TYPE_OBJECT_DATA_TYPE_NAME,
        false,
        qos
    );
}

bool is_type_object_topic(const types::DdsTopic& topic)
{
    return (strcmp(topic.topic_name.c_str(), TYPE_OBJECT_TOPIC_NAME) == 0)
        && (strcmp(topic.type_name.c_str(), TYPE_OBJECT_DATA_TYPE_NAME) == 0);
}

types::Guid new_unique_guid()
{
    static int current_unique_value = 0;
    types::Guid new_guid;
    new_guid.entityId.value[3] = ++current_unique_value;
    // TODO(recorder) WARNING this only admits 256 simulated guids. Extend it for the rest of the entity Id
    return new_guid;
}

std::unique_ptr<types::DataReceived> type_object_data_serialization(
    std::shared_ptr<PayloadPool> payload_pool,
    eprosima::fastrtps::types::DynamicType_ptr dyn_type)
{
    // TODO(recorder)
    // This moves the actual ptr for the type, serializing the ptr itself as it is not going destroyed (stored in Participant).
    // This is very bad. Dont do this.

    // Create and data and serialize inside the string with the type name
    auto ptr_ = dyn_type.get();
    std::unique_ptr<types::DataReceived> data = std::make_unique<types::DataReceived>();
    auto size_of_data = sizeof(sizeof(dyn_type.get()));
    payload_pool->get_payload(
        size_of_data,
        data->payload
    );

    std::memcpy(data->payload.data, &ptr_, size_of_data);
    data->payload.length = size_of_data;

    return data;
}

eprosima::fastrtps::types::DynamicType* type_object_data_deserialization(
    const std::unique_ptr<types::DataReceived>& data)
{
    // TODO(recorder)
    // Get the Dyn ptr from "serialized" data
    eprosima::fastrtps::types::DynamicType* result;
    std::memcpy(&result, data->payload.data, data->payload.length);
    return result;
}

} /* namespace recorder */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
