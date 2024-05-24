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
 * @file Deserializer.cpp
 */

#include <memory>

#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/CDRMessage_t.h>
#include <fastdds/rtps/common/SerializedPayload.h>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

template <typename T>
T Deserializer::type_str_to_type_data_(
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