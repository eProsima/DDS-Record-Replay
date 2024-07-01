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
 * @file SqlMessage.cpp
 */


#include <map>
#include <string>

#include <nlohmann/json.hpp>

#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include <cpp_utils/Log.hpp>

#include <ddsrecorder_participants/recorder/message/SqlMessage.hpp>


namespace eprosima {
namespace ddsrecorder {
namespace participants {

SqlMessage::SqlMessage(
    const ddspipe::core::types::RtpsPayloadData& data,
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool,
    const ddspipe::core::types::DdsTopic& topic,
    const bool log_publish_time,
    const std::string& key /* = "" */)
    : BaseMessage(data, payload_pool, topic, log_publish_time)
    , writer_guid(data.sample_identity.writer_guid())
    , sequence_number(data.sample_identity.sequence_number())
    , instance_handle(data.instanceHandle)
    , key(key)
{
}

void SqlMessage::set_key(
        const fastdds::dds::DynamicType::_ref_type& dynamic_type)
{
    // Deserialize the payload
    fastdds::dds::DynamicPubSubType pub_sub_type(dynamic_type);
    auto dynamic_data = fastdds::dds::DynamicDataFactory::get_instance()->create_data(dynamic_type);

    pub_sub_type.deserialize(&payload, &dynamic_data);

    // Clear non-key values to free-up unnecessary space
    dynamic_data->clear_nonkey_values();

    // Serialize the key members into a JSON
    const auto ret = fastdds::dds::json_serialize(
            dynamic_data, key, fastdds::dds::DynamicDataJsonFormat::EPROSIMA);

    if (ret != fastdds::dds::RETCODE_OK)
    {
        logWarning(SQL_MESSAGE, "Failed to serialize key members into JSON");
    }

    nlohmann::json key_json = nlohmann::json::parse(key);

    // Remove non-key values
    remove_nonkey_values(dynamic_type, key_json);

    // Serialize the JSON back into a string
    key = key_json.dump();
}

void SqlMessage::remove_nonkey_values(
        const fastdds::dds::DynamicType::_ref_type& dynamic_type,
        nlohmann::json& key_json)
{
    fastdds::dds::DynamicTypeMembersById members_by_id;

    if (dynamic_type->get_all_members(members_by_id) != fastdds::dds::RETCODE_OK)
    {
        logWarning(DDSRECORDER_DYNTYPES_KEY, "Failed to get all members");
        return;
    }

    for (const auto& member_by_id : members_by_id)
    {
        const auto member = member_by_id.second;

        fastdds::dds::MemberDescriptor::_ref_type member_descriptor{
                fastdds::dds::traits<fastdds::dds::MemberDescriptor>::make_shared()};

        if (member->get_descriptor(member_descriptor) != fastdds::dds::RETCODE_OK)
        {
            logWarning(DDSRECORDER_DYNTYPES_KEY, "Failed to get member descriptor");
            continue;
        }

        const auto member_name = static_cast<std::string>(member_descriptor->name());

        if (member_descriptor->is_key())
        {
            // Recursively remove non-key values from nested types
            remove_nonkey_values(member_descriptor->type(), key_json[member_name]);
        }
        else
        {
            // Remove non-key value
            key_json.erase(member_name);
        }
    }
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
