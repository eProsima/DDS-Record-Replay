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


template <typename T>
fastrtps::rtps::SerializedPayload_t Serializer::data_to_payload_(
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

    return payload;
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
