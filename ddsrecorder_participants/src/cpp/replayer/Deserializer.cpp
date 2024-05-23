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

#include <cstdint>

#include <yaml-cpp/yaml.h>

#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/CDRMessage_t.h>
#include <fastdds/rtps/common/SerializedPayload.h>

#include <fastdds/dds/xtypes/type_representation/detail/dds_xtypes_typeobject.hpp>

#include <ddspipe_core/types/dds/TopicQoS.hpp>

#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollection.hpp>
#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollectionPubSubTypes.h>
#include <ddsrecorder_participants/constants.hpp>
#include <ddsrecorder_participants/replayer/Deserializer.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

template <>
ddspipe::core::types::TopicQoS Deserializer::deserialize(
        const std::string& topic_qos_str)
{
    // TODO: Reuse code from ddspipe_yaml

    ddspipe::core::types::TopicQoS qos{};

    auto qos_yaml = YAML::Load(topic_qos_str);
    bool reliable = qos_yaml[QOS_SERIALIZATION_RELIABILITY].as<bool>();
    bool transient_local = qos_yaml[QOS_SERIALIZATION_DURABILITY].as<bool>();
    bool exclusive_ownership = qos_yaml[QOS_SERIALIZATION_OWNERSHIP].as<bool>();
    bool keyed = qos_yaml[QOS_SERIALIZATION_KEYED].as<bool>();

    // Parse reliability
    if (reliable)
    {
        qos.reliability_qos = ddspipe::core::types::ReliabilityKind::RELIABLE;
    }
    else
    {
        qos.reliability_qos = ddspipe::core::types::ReliabilityKind::BEST_EFFORT;
    }

    // Parse durability
    if (transient_local)
    {
        qos.durability_qos = ddspipe::core::types::DurabilityKind::TRANSIENT_LOCAL;
    }
    else
    {
        qos.durability_qos = ddspipe::core::types::DurabilityKind::VOLATILE;
    }

    // Parse ownership
    if (exclusive_ownership)
    {
        qos.ownership_qos = ddspipe::core::types::OwnershipQosPolicyKind::EXCLUSIVE_OWNERSHIP_QOS;
    }
    else
    {
        qos.ownership_qos = ddspipe::core::types::OwnershipQosPolicyKind::SHARED_OWNERSHIP_QOS;
    }

    // Parse keyed
    qos.keyed = keyed;

    return qos;
}

template <>
fastdds::dds::xtypes::TypeIdentifier Deserializer::deserialize(
        const std::string& type_identifier_str)
{
    return type_str_to_type_data_<fastdds::dds::xtypes::TypeIdentifier>(type_identifier_str);
}

template <>
fastdds::dds::xtypes::TypeObject Deserializer::deserialize(
        const std::string& type_object_str)
{
    return type_str_to_type_data_<fastdds::dds::xtypes::TypeObject>(type_object_str);
}

template <>
DynamicTypesCollection Deserializer::deserialize(
        const std::string& raw_data_str)
{
    // Copy raw data into a payload
    fastdds::rtps::SerializedPayload_t serialized_payload{(std::uint32_t) raw_data_str.size()};
    serialized_payload.length = raw_data_str.size();

    std::memcpy(
        serialized_payload.data,
        reinterpret_cast<const unsigned char*>(raw_data_str.c_str()),
        raw_data_str.size());

    // Deserialize the payload into a DynamicTypesCollection
    DynamicTypesCollection dynamic_types;
    fastdds::dds::TypeSupport type_support(new DynamicTypesCollectionPubSubType());

    type_support.deserialize(&serialized_payload, &dynamic_types);

    return dynamic_types;
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
