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
 * @file Serializer.hpp
 */

#pragma once

#include <fastdds/dds/xtypes/type_representation/detail/dds_xtypes_typeobject.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

#include <ddspipe_core/types/dds/TopicQoS.hpp>

#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollection.hpp>
#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollectionPubSubTypes.h>
#include <ddsrecorder_participants/library/library_dll.h>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Class to serialize different types of data.
 */
class Serializer
{
public:

    /**
     * @brief Serialize a \c TopicQoS struct into a string.
     *
     * @param [in] qos TopicQoS to be serialized
     * @return Serialized TopicQoS string
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    static std::string serialize(
            const ddspipe::core::types::TopicQoS& qos);

    /**
     * @brief Serialize a \c TypeIdentifier into a string.
     *
     * Calculates the size of the serialized TypeIdentifier.
     * Calls \c data_to_str_ to serialize \c type_identifier
     *
     * @param [in] type_identifier TypeIdentifier to be serialized
     * @return Serialized TypeIdentifier string
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    static std::string serialize(
            const fastdds::dds::xtypes::TypeIdentifier& type_identifier);

    /**
     * @brief Serialize a \c TypeObject into a string.
     *
     * @param [in] type_object TypeObject to be serialized
     * @return Serialized TypeObject string
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    static std::string serialize(
            const fastdds::dds::xtypes::TypeObject& type_object);

    /**
     * @brief Serialize given \c DynamicTypesCollection into a \c SerializedPayload .
     *
     * @param [in] dynamic_types Dynamic types collection to be serialized.
     * @return Serialized payload for the given dynamic types collection.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    static fastdds::rtps::SerializedPayload_t* serialize(
            DynamicTypesCollection* dynamic_types);

protected:

    /**
     * @brief Serialize a \c TypeObject or a \c TypeIdentifier into a string.
     *
     * @param [in] type_data \c TypeObject or \c TypeIdentifier to serialize
     * @return string with the serialization
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    template <typename T>
    static std::string type_data_to_str_(
            const T& type_data);
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */

#include <ddsrecorder_participants/recorder/output/impl/Serializer.ipp>
