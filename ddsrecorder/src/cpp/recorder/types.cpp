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

#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

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

std::unique_ptr<types::DataReceived> actual_type_object_data_serialization(
    std::shared_ptr<PayloadPool> payload_pool,
    const eprosima::fastrtps::types::TypeObject* type_obj)
{
    std::unique_ptr<types::DataReceived> data = std::make_unique<types::DataReceived>();

    // auto size_of_data = type_obj->getCdrSerializedSize(*type_obj);
    auto size_of_data = fastrtps::types::TypeObject::getCdrSerializedSize(*type_obj) + eprosima::fastrtps::rtps::SerializedPayload_t::representation_header_size;

    payload_pool->get_payload(size_of_data, data->payload);

    data->payload.length = size_of_data;

    // Object that manages the raw buffer.
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(data->payload.data), data->payload.max_size);
    // Object that serializes the data.
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
    data->payload.encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
    // Serialize encapsulation
    ser.serialize_encapsulation();

    try
    {
        // Serialize the object.
        type_obj->serialize(ser);
    }
    catch (eprosima::fastcdr::exception::NotEnoughMemoryException& ex)
    {
        std::cout << "ERRORACO ON TYPE OBJECT SERIALIZATION: " << ex.what() << std::endl;
    }

    // // Get the serialized length
    // // data->payload.length = static_cast<uint32_t>(ser.getSerializedDataLength());

    //////////////////////////////////////////////////////////////////////////////////

    // size_t size = fastrtps::types::TypeObject::getCdrSerializedSize(type_obj)
    //         + eprosima::fastrtps::rtps::SerializedPayload_t::representation_header_size;
    // fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(size));
    // eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

    // eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
    //         eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
    // payload.encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    // ser.serialize_encapsulation();

    // type_obj.serialize(ser);
    // payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
    // size = (ser.getSerializedDataLength() + 3) & ~3;

    // bool valid = fastrtps::rtps::CDRMessage::addUInt16(cdr_message, qos_policy.Pid);
    // valid &= fastrtps::rtps::CDRMessage::addUInt16(cdr_message, static_cast<uint16_t>(size));
    // valid &= fastrtps::rtps::CDRMessage::addData(cdr_message, payload.data, payload.length);

    // for (uint32_t count = payload.length; count < size; ++count)
    // {
    //     valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    // }

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


eprosima::fastrtps::types::TypeObject* actual_type_object_data_deserialization(
    const std::unique_ptr<types::DataReceived>& data)
{
    eprosima::fastrtps::types::TypeObject* result = nullptr;
    // eprosima::fastrtps::types::TypeObject* result = new eprosima::fastrtps::types::TypeObject();
    try
    {
        //Convert DATA to pointer of your type
        result = static_cast<eprosima::fastrtps::types::TypeObject*>((void*)data->payload.data);

        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(data->payload.data), data->payload.length);

        // Object that deserializes the data.
        eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

        // Deserialize encapsulation.
        deser.read_encapsulation();
        data->payload.encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

        // Deserialize the object.
        result->deserialize(deser);
    }
    catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        std::cout << "ERRORACO ON TYPE OBJECT deSERIALIZATION" << std::endl;
    }

    return result;
}

} /* namespace recorder */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
