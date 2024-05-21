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

#include <nlohmann/json.hpp>

#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicDataHelper.hpp>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/TypesBase.h>

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
    , instance_handle(data.instanceHandle)
    , key(key)
{
}

void SqlMessage::set_key(
        fastrtps::types::DynamicType_ptr dynamic_type)
{
    // Deserialize the payload
    fastrtps::types::DynamicPubSubType pub_sub_type(dynamic_type);
    fastrtps::types::DynamicData_ptr dynamic_data;
    dynamic_data = fastrtps::types::DynamicDataFactory::get_instance()->create_data(dynamic_type);

    pub_sub_type.deserialize(&payload, dynamic_data.get());

    // Clear non-key values to free-up unnecessary space
    // dynamic_data->clear_nonkey_values();

    // Serialize the key members into a JSON
    nlohmann::json key_json;

    std::map<fastrtps::types::MemberId, fastrtps::types::DynamicTypeMember*> members_map;
    dynamic_type->get_all_members(members_map);

    for (const auto& member : members_map)
    {
        if (!member.second->key_annotation())
        {
            // The member is not a key
            continue;
        }

        const auto descriptor = member.second->get_descriptor();

        if (descriptor == nullptr)
        {
            // The member has no descriptor
            continue;
        }

        switch (descriptor->get_kind())
        {
            case fastrtps::types::TK_BOOLEAN:
            {
                key_json[descriptor->get_name()] = dynamic_data->get_bool_value(descriptor->get_id());
                break;
            }
            case fastrtps::types::TK_BYTE:
            {
                key_json[descriptor->get_name()] = dynamic_data->get_byte_value(descriptor->get_id());
                break;
            }
            case fastrtps::types::TK_INT16:
            {
                key_json[descriptor->get_name()] = dynamic_data->get_int16_value(descriptor->get_id());
                break;
            }
            case fastrtps::types::TK_INT32:
            {
                key_json[descriptor->get_name()] = dynamic_data->get_int32_value(descriptor->get_id());
                break;
            }
            case fastrtps::types::TK_INT64:
            {
                key_json[descriptor->get_name()] = dynamic_data->get_int64_value(descriptor->get_id());
                break;
            }
            case fastrtps::types::TK_UINT16:
            {
                key_json[descriptor->get_name()] = dynamic_data->get_uint16_value(descriptor->get_id());
                break;
            }
            case fastrtps::types::TK_UINT32:
            {
                key_json[descriptor->get_name()] = dynamic_data->get_uint32_value(descriptor->get_id());
                break;
            }
            case fastrtps::types::TK_UINT64:
            {
                key_json[descriptor->get_name()] = dynamic_data->get_uint64_value(descriptor->get_id());
                break;
            }
            case fastrtps::types::TK_FLOAT32:
            {
                key_json[descriptor->get_name()] = dynamic_data->get_float32_value(descriptor->get_id());
                break;
            }
            case fastrtps::types::TK_FLOAT64:
            {
                key_json[descriptor->get_name()] = dynamic_data->get_float64_value(descriptor->get_id());
                break;
            }
            case fastrtps::types::TK_FLOAT128:
            {
                key_json[descriptor->get_name()] = dynamic_data->get_float128_value(descriptor->get_id());
                break;
            }
            case fastrtps::types::TK_CHAR8:
            {
                key_json[descriptor->get_name()] = dynamic_data->get_char8_value(descriptor->get_id());
                break;
            }
            case fastrtps::types::TK_CHAR16:
            {
                key_json[descriptor->get_name()] = dynamic_data->get_char16_value(descriptor->get_id());
                break;
            }
            case fastrtps::types::TK_STRING8:
            {
                key_json[descriptor->get_name()] = dynamic_data->get_string_value(descriptor->get_id());
                break;
            }
            case fastrtps::types::TK_STRING16:
            {
                key_json[descriptor->get_name()] = dynamic_data->get_wstring_value(descriptor->get_id());
                break;
            }
            case fastrtps::types::TK_ALIAS:
            {
                // TODO
                break;
            }
            case fastrtps::types::TK_ENUM:
            {
                key_json[descriptor->get_name()] = dynamic_data->get_enum_value(descriptor->get_id());
                break;
            }
            case fastrtps::types::TK_BITMASK:
            {
                // TODO
                break;
            }
            case fastrtps::types::TK_ANNOTATION:
            {
                // TODO
                break;
            }
            case fastrtps::types::TK_STRUCTURE:
            {
                // TODO
                break;
            }
            case fastrtps::types::TK_UNION:
            {
                // TODO
                break;
            }
            case fastrtps::types::TK_BITSET:
            {
                // TODO
                break;
            }
            case fastrtps::types::TK_SEQUENCE:
            {
                // TODO
                break;
            }
            case fastrtps::types::TK_ARRAY:
            {
                // TODO
                break;
            }
        }
    }

    // Dump the JSON into a string
    key = key_json.dump();
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
