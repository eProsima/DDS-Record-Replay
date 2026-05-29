// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DynamicTypesSupport.cpp
 */

#include "DynamicTypesSupport.hpp"

#include <exception>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>

#include <cpp_utils/Log.hpp>
#include <cpp_utils/utils.hpp>

#include <ddsrecorder_participants/common/serialize/Serializer.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace replayer {
namespace detail {

/**
 * @brief Shared dynamic-type support for replay and conversion modes
 *
 * This file centralizes the logic that used to live only in
 * \c DdsReplayer::register_dynamic_types_:
 * - decode the base64-serialized type data stored in the MCAP attachment
 * - deserialize Fast DDS \c TypeObject data
 * - register it in the Fast DDS TypeObjectRegistry
 *
 * The MCAP-to-SQL converter additionally uses the registered \c TypeObject data to
 * build \c DynamicType instances and deserialize samples into JSON
 */

RegisteredDynamicTypes register_dynamic_types(
        const participants::DynamicTypesCollection& dynamic_types)
{
    RegisteredDynamicTypes registered_types;
    auto& registry = fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry();

    for (const auto& dynamic_type : dynamic_types.dynamic_types())
    {
        try
        {
            const auto type_identifier_str = utils::base64_decode(dynamic_type.type_identifier());
            fastdds::dds::xtypes::TypeIdentifier type_identifier;
            participants::Serializer::deserialize(type_identifier_str, type_identifier);
            (void)type_identifier;

            const auto type_object_str = utils::base64_decode(dynamic_type.type_object());
            RegisteredDynamicType registered_dynamic_type;
            participants::Serializer::deserialize(type_object_str, registered_dynamic_type.type_object);

            const auto ret = registry.register_type_object(
                registered_dynamic_type.type_object,
                registered_dynamic_type.type_identifiers);

            if (ret != fastdds::dds::RETCODE_OK)
            {
                EPROSIMA_LOG_WARNING(
                    DDSREPLAYER,
                    "Failed to register type object for type " << dynamic_type.type_name() << ".");
            }

            registered_types.insert_or_assign(dynamic_type.type_name(), std::move(registered_dynamic_type));
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_WARNING(
                DDSREPLAYER,
                "Failed to decode dynamic type " << dynamic_type.type_name() << ": " << e.what());
        }
    }

    return registered_types;
}

std::map<std::string, fastdds::dds::DynamicType::_ref_type> build_dynamic_types(
        const RegisteredDynamicTypes& registered_types)
{
    std::map<std::string, fastdds::dds::DynamicType::_ref_type> dynamic_types;

    for (const auto& [type_name, registered_dynamic_type] : registered_types)
    {
        try
        {
            auto type_builder =
                    fastdds::dds::DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
                registered_dynamic_type.type_object);

            if (type_builder == nullptr)
            {
                EPROSIMA_LOG_WARNING(
                    DDSREPLAYER,
                    "Failed to create a dynamic type builder for type " << type_name << ".");
                continue;
            }

            auto dynamic_type = type_builder->build();

            if (dynamic_type == nullptr)
            {
                EPROSIMA_LOG_WARNING(
                    DDSREPLAYER,
                    "Failed to build dynamic type " << type_name << ".");
                continue;
            }

            dynamic_types.insert_or_assign(type_name, dynamic_type);
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_WARNING(
                DDSREPLAYER,
                "Failed to build dynamic type " << type_name << ": " << e.what());
        }
    }

    return dynamic_types;
}

std::map<std::string, fastdds::dds::xtypes::TypeIdentifierPair> type_identifiers_from(
        const RegisteredDynamicTypes& registered_types)
{
    std::map<std::string, fastdds::dds::xtypes::TypeIdentifierPair> type_identifiers;

    for (const auto& [type_name, registered_dynamic_type] : registered_types)
    {
        type_identifiers.insert_or_assign(type_name, registered_dynamic_type.type_identifiers);
    }

    return type_identifiers;
}

} // namespace detail
} /* namespace replayer */
} /* namespace ddsrecorder */
} /* namespace eprosima */
