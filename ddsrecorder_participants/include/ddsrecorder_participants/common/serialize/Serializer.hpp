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

#include <ddspipe_core/types/dds/TopicQoS.hpp>
#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollection.hpp>


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
     * @param [in]  object Object to serialize
     * @param [out] serialized_str String to store the serialized object
     *
     * @return Serialized object string
     */
    template <typename T>
    static bool serialize(
            const T& object,
            std::string& serialized_str);

    /**
     * @brief Deserialize \c serialized_str into \c T.
     *
     * @param [in]  serialized_str String to deserialize
     * @param [out] deserialized_object Reference to store the deserialized object
     *
     * @return Deserialized \c T
     */
    template <typename T>
    static bool deserialize(
            const std::string& serialized_str,
            T& deserialized_object);

protected:

    /**
     * @brief Serialize a \c TypeObject or a \c TypeIdentifier into a string.
     *
     * @param [in]  data \c TypeObject or \c TypeIdentifier to serialize
     * @param [out] type_str String to store the serialized data
     *
     * @return Whether the serialization was successful
     */
    template <typename T>
    static bool type_data_to_type_str_(
            const T& data,
            std::string& type_str);

    /**
     * @brief Deserialize \c type_str into a \c TypeObject or a \c TypeIdentifier.
     *
     * @param [in]  type_str String to deserialize
     * @param [out] type_data Reference to store the deserialized data
     *
     * @return Deserialized \c TypeObject or \c TypeIdentifier
     */
    template <typename T>
    static bool type_str_to_type_data_(
            const std::string& type_str,
            T& type_data);
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */

#include <ddsrecorder_participants/common/serialize/impl/Serializer.ipp>
