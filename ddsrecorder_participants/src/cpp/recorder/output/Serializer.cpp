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
        const fastdds::dds::xtypes::TypeIdentifier& type_identifier)
{
    return type_data_to_str_(type_identifier);
}

std::string Serializer::serialize(
        const fastdds::dds::xtypes::TypeObject& type_object)
{
    return type_data_to_str_(type_object);
}

fastdds::rtps::SerializedPayload_t* Serializer::serialize(
        DynamicTypesCollection* dynamic_types)
{
    // Serialize dynamic types collection using CDR
    fastdds::dds::TypeSupport type_support(new DynamicTypesCollectionPubSubType());
    auto serialized_payload = new fastdds::rtps::SerializedPayload_t(type_support.get_serialized_size_provider(dynamic_types)());
    type_support.serialize(dynamic_types, serialized_payload);

    return serialized_payload;
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
