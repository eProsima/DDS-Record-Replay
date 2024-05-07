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
 * @file Serializer.cpp
 */

#include <yaml-cpp/yaml.h>

#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/CDRMessage_t.h>
#include <fastdds/rtps/common/SerializedPayload.h>

#include <ddsrecorder_participants/constants.hpp>
#include <ddsrecorder_participants/recorder/output/Serializer.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {


std::string Serializer::serialize(
        const ddspipe::core::types::TopicQoS& qos)
{
    // TODO: Reuse code from ddspipe_yaml

    YAML::Node qos_yaml;

    qos_yaml[QOS_SERIALIZATION_RELIABILITY] = qos.is_reliable();
    qos_yaml[QOS_SERIALIZATION_DURABILITY] = qos.is_transient_local();
    qos_yaml[QOS_SERIALIZATION_OWNERSHIP] = qos.has_ownership();
    qos_yaml[QOS_SERIALIZATION_KEYED] = qos.keyed.get_value();

    return YAML::Dump(qos_yaml);
}

std::string Serializer::serialize(
        const fastrtps::types::TypeIdentifier& type_identifier)
{
    const auto size =
            fastrtps::rtps::SerializedPayload_t::representation_header_size +
            fastrtps::types::TypeIdentifier::getCdrSerializedSize(type_identifier);

    const auto payload = data_to_payload_(type_identifier, size);
    const auto serialized_payload = payload_to_str_(payload);

    return serialized_payload;
}

std::string Serializer::serialize(
        const fastrtps::types::TypeObject& type_object)
{
    const auto size =
            fastrtps::rtps::SerializedPayload_t::representation_header_size +
            fastrtps::types::TypeObject::getCdrSerializedSize(type_object);

    const auto payload = data_to_payload_(type_object, size);
    const auto serialized_payload = payload_to_str_(payload);

    return serialized_payload;
}

fastrtps::rtps::SerializedPayload_t* Serializer::serialize(
        DynamicTypesCollection* dynamic_types)
{
    // Serialize dynamic types collection using CDR
    fastdds::dds::TypeSupport type_support(new DynamicTypesCollectionPubSubType());
    auto serialized_payload = new fastrtps::rtps::SerializedPayload_t(type_support.get_serialized_size_provider(dynamic_types)());
    type_support.serialize(dynamic_types, serialized_payload);

    return serialized_payload;
}

std::string Serializer::payload_to_str_(
        const fastrtps::rtps::SerializedPayload_t& payload)
{
    // Create CDR message
    // NOTE: Use 0 length to avoid allocation (memory already reserved in payload creation)
    std::unique_ptr<fastrtps::rtps::CDRMessage_t> cdr_message = std::make_unique<fastrtps::rtps::CDRMessage_t>(0);

    cdr_message->buffer = payload.data;
    cdr_message->max_size = payload.max_size;
    cdr_message->length = payload.length;

#if __BIG_ENDIAN__
    cdr_message->msg_endian = fastrtps::rtps::BIGEND;
#else
    cdr_message->msg_endian = fastrtps::rtps::LITTLEEND;
#endif // if __BIG_ENDIAN__

    // Add data
    bool valid = fastrtps::rtps::CDRMessage::addData(cdr_message.get(), payload.data, payload.length);

    const auto payload_size = (payload.length + 3) & ~3;

    for (uint32_t count = payload.length; count < payload_size; ++count)
    {
        valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message.get(), 0);
    }
    // Copy buffer to string
    std::string serialized_payload(reinterpret_cast<char const*>(cdr_message->buffer), payload_size);

    // Delete CDR message
    // NOTE: set wraps attribute to avoid double free (buffer released by payload on destruction)
    cdr_message->wraps = true;

    return serialized_payload;
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
