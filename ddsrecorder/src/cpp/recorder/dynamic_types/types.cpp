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

#include <fastrtps/types/TypeObjectFactory.h>

#include <ddsrecorder/types/dds/Data.hpp>

#include <recorder/dynamic_types/types.hpp>

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

std::unique_ptr<types::DataReceived> string_serialization(
        std::shared_ptr<PayloadPool> payload_pool,
        const std::string& str)
{
    // Create and data and serialize inside the string with the type name
    std::unique_ptr<types::DataReceived> data = std::make_unique<types::DataReceived>();

    auto size_of_data = str.size();
    payload_pool->get_payload(
        size_of_data,
        data->payload
    );

    data->payload.length = size_of_data;
    std::memcpy(data->payload.data, str.c_str(), size_of_data);

    return data;
}

std::string string_deserialization(
        const std::unique_ptr<types::DataReceived>& data)
{
    return std::string(
        const_cast<const char*>(
            reinterpret_cast<char*>(
                data->payload.data)),
        data->payload.length);
}

const fastrtps::types::TypeObject* type_object_from_name(
        const std::string& type_name)
{
    auto type_obj_factory = eprosima::fastrtps::types::TypeObjectFactory::get_instance();
    auto type_id = type_obj_factory->get_type_identifier(type_name, true);

    if (type_id == nullptr)
    {
        return nullptr;
    }

    return type_obj_factory->get_type_object(type_id);
}

const fastrtps::types::DynamicType_ptr dynamic_type_from_name(
        const std::string& type_name)
{
    auto type_obj_factory = eprosima::fastrtps::types::TypeObjectFactory::get_instance();

    auto type_id = type_obj_factory->get_type_identifier(type_name, true);
    if (type_id == nullptr)
    {
        return fastrtps::types::DynamicType_ptr();
    }

    auto type_obj = type_obj_factory->get_type_object(type_id);
    if (type_obj == nullptr)
    {
        return fastrtps::types::DynamicType_ptr();
    }

    return type_obj_factory->build_dynamic_type(type_name, type_id, type_obj);
}

} /* namespace recorder */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
