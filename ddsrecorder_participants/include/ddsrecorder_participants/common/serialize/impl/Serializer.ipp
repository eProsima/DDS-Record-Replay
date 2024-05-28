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
        const T& type_data)
{
    // Reserve payload and create buffer
    fastcdr::CdrSizeCalculator calculator(fastcdr::CdrVersion::XCDRv2);
    size_t current_alignment {0};
    size_t type_data_size = calculator.calculate_serialized_size(type_data, current_alignment) +
                            fastdds::rtps::SerializedPayload_t::representation_header_size;

    fastdds::rtps::SerializedPayload_t payload(static_cast<std::uint32_t>(type_data_size));
    fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

    // Create CDR serializer
    fastcdr::Cdr ser(fastbuffer, fastcdr::Cdr::DEFAULT_ENDIAN,
            fastcdr::CdrVersion::XCDRv2);

    payload.encapsulation = ser.endianness() == fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    // Serialize
    fastcdr::serialize(ser, type_data);
    payload.length = (std::uint32_t)ser.get_serialized_data_length();
    type_data_size = (ser.get_serialized_data_length() + 3) & ~3;

    // Create CDR message with payload
    auto cdr_message = std::make_unique<fastdds::rtps::CDRMessage_t>(payload);

    // Add data
    if (!(cdr_message && (cdr_message->pos + payload.length <= cdr_message->max_size))|| (payload.length > 0 && !payload.data))
    {
        // TODO Warning
    }
    else
    {
        memcpy(&cdr_message->buffer[cdr_message->pos], payload.data, payload.length);
        cdr_message->pos += payload.length;
        cdr_message->length += payload.length;
    }

    fastdds::rtps::octet value = 0;

    for (std::uint32_t count = payload.length; count < type_data_size; ++count)
    {
        const std::uint32_t size_octet = sizeof(value);
        if (!(cdr_message && (cdr_message->pos + size_octet <= cdr_message->max_size)))
        {
            // TODO Warning
            continue;
        }

        for (std::uint32_t i = 0; i < size_octet; i++)
        {
            cdr_message->buffer[cdr_message->pos + i] = *((fastdds::rtps::octet*)&value + size_octet - 1 - i);
        }

        cdr_message->pos += size_octet;
        cdr_message->length += size_octet;
    }

    // Copy buffer to string
    std::string type_data_str(reinterpret_cast<char const*>(cdr_message->buffer), type_data_size);

    // Delete CDR message
    // NOTE: set wraps attribute to avoid double free (buffer released by payload on destruction)
    cdr_message->wraps = true;

    return type_data_str;
}

template <typename T>
T Serializer::type_str_to_type_data_(
        const std::string& type_str)
{
    // Create CDR message from string
    // NOTE: Use 0 length to avoid allocation
    auto cdr_message = std::make_unique<fastdds::rtps::CDRMessage_t>(0);

    cdr_message->buffer = (unsigned char*)reinterpret_cast<const unsigned char*>(type_str.c_str());
    cdr_message->length = type_str.length();
#if __BIG_ENDIAN__
    cdr_message->msg_endian = fastdds::rtps::BIGEND;
#else
    cdr_message->msg_endian = fastdds::rtps::LITTLEEND;
#endif // if __BIG_ENDIAN__

    // Reserve payload and create buffer
    const auto parameter_length = cdr_message->length;
    fastdds::rtps::SerializedPayload_t payload(parameter_length);
    fastcdr::FastBuffer fastbuffer((char*)payload.data, parameter_length);

    // Read data
    if (cdr_message != nullptr && cdr_message->length >= cdr_message->pos + parameter_length && parameter_length > 0 && payload.data != nullptr)
    {
        memcpy(payload.data, &cdr_message->buffer[cdr_message->pos], parameter_length);
        cdr_message->pos += parameter_length;
    }

    // Create CDR deserializer
    fastcdr::Cdr deser(fastbuffer, fastcdr::Cdr::DEFAULT_ENDIAN, fastcdr::CdrVersion::XCDRv2);
    payload.encapsulation = deser.endianness() == fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    // Deserialize
    T type_data;
    fastcdr::deserialize(deser, type_data);

    // Delete CDR message
    // NOTE: set wraps attribute to avoid double free (buffer released by string on destruction)
    cdr_message->wraps = true;

    return type_data;
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
