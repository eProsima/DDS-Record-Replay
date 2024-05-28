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
 * @file Serializer.hpp
 */

#pragma once

#include <string>

#include <ddsrecorder_participants/library/library_dll.h>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Class to serialize and deserialize different types of data.
 */
class DDSRECORDER_PARTICIPANTS_DllAPI Serializer
{
public:

    /**
     * @brief Serialize \c object into a string.
     *
     * @param [in] object Object to serialize
     * @return Serialized object string
     */
    template <typename T>
    static std::string serialize(
            const T& object);

    /**
     * @brief Deserialize \c serialized_str into \c T.
     *
     * @param [in] serialized_str String to deserialize
     * @return Deserialized \c T
     */
    template <typename T>
    static T deserialize(
            const std::string& serialized_str);

protected:

    /**
     * @brief Serialize a \c TypeObject or a \c TypeIdentifier into a \c SerializedPayload_t.
     *
     * @param [in] object \c TypeObject or \c TypeIdentifier to serialize
     * @return \c SerializedPayload_t with the serialization
     */
    template <typename T>
    static std::string type_data_to_type_str_(
            const T& data,
            const size_t size);

    /**
     * @brief Deserialize \c type_str into a \c TypeObject or a \c TypeIdentifier.
     *
     * @param [in] type_str String to deserialize
     * @return Deserialized \c TypeObject or \c TypeIdentifier
     */
    template <typename T>
    static T type_str_to_type_data_(
            const std::string& type_str);
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */

#include <ddsrecorder_participants/common/serialize/impl/Serializer.ipp>
