// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file Serializer.ipp
 */

#include <memory>
#include <string>

#include <yaml-cpp/yaml.h>

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>

#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/CDRMessage_t.h>
#include <fastdds/rtps/common/SerializedPayload.h>

namespace eprosima {
namespace ddsrecorder {
namespace participants {


template <typename T>
std::string Serializer::type_data_to_type_str_(
        const T& data,
        const size_t data_size)
{
    // Reserve payload and create buffer
    fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(data_size));
    fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

    // Create CDR serializer
#if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    fastcdr::Cdr ser(fastbuffer, fastcdr::Cdr::DEFAULT_ENDIAN, fastcdr::Cdr::DDS_CDR);
#else
    fastcdr::Cdr ser(fastbuffer, fastcdr::Cdr::DEFAULT_ENDIAN, fastcdr::CdrVersion::XCDRv1);
#endif // if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13

    payload.encapsulation = ser.endianness() == fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    // Serialize
    data.serialize(ser);

#if FASTCDR_VERSION_MAJOR == 1
    payload.length = (uint32_t) ser.getSerializedDataLength();
#else
    payload.length = (uint32_t) ser.get_serialized_data_length();
#endif // if FASTCDR_VERSION_MAJOR == 1

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

template <typename T>
T Serializer::type_str_to_type_data_(
        const std::string& type_str)
{
    // Create CDR message from string
    // NOTE: Use 0 length to avoid allocation
    auto cdr_message = std::make_unique<fastrtps::rtps::CDRMessage_t>(0);

    cdr_message->buffer = (unsigned char*)reinterpret_cast<const unsigned char*>(type_str.c_str());
    cdr_message->length = type_str.length();
#if __BIG_ENDIAN__
    cdr_message->msg_endian = fastrtps::rtps::BIGEND;
#else
    cdr_message->msg_endian = fastrtps::rtps::LITTLEEND;
#endif // if __BIG_ENDIAN__

    // Reserve payload and create buffer
    const auto parameter_length = cdr_message->length;
    fastrtps::rtps::SerializedPayload_t payload(parameter_length);
    fastcdr::FastBuffer fastbuffer((char*)payload.data, parameter_length);

    // Read data
    fastrtps::rtps::CDRMessage::readData(cdr_message.get(), payload.data, parameter_length);

    // Create CDR deserializer
#if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    fastcdr::Cdr deser(fastbuffer, fastcdr::Cdr::DEFAULT_ENDIAN, fastcdr::Cdr::DDS_CDR);
#else
    fastcdr::Cdr deser(fastbuffer, fastcdr::Cdr::DEFAULT_ENDIAN, fastcdr::CdrVersion::XCDRv1);
#endif // if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    payload.encapsulation = deser.endianness() == fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    // Deserialize
    T type;
    type.deserialize(deser);

    // Delete CDR message
    // NOTE: set wraps attribute to avoid double free (buffer released by string on destruction)
    cdr_message->wraps = true;

    return type;
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
