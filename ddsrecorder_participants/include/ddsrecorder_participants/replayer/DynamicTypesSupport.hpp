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
 * @file DynamicTypesSupport.hpp
 */

#pragma once

#include <map>
#include <string>

#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollection.hpp>
#include <ddsrecorder_participants/library/library_dll.h>

namespace eprosima {
namespace ddsrecorder {
namespace participants {
namespace detail {

/**
 * @brief Dynamic-type data registered in Fast DDS for later replay or conversion use.
 */
struct DDSRECORDER_PARTICIPANTS_DllAPI RegisteredDynamicType
{
    fastdds::dds::xtypes::TypeIdentifierPair type_identifiers;
    fastdds::dds::xtypes::TypeObject type_object;
};

/**
 * @brief Registered dynamic types indexed by DDS type name.
 */
using RegisteredDynamicTypes = std::map<std::string, RegisteredDynamicType>;

/**
 * @brief Decode and register MCAP-discovered dynamic types in Fast DDS.
 */
DDSRECORDER_PARTICIPANTS_DllAPI
RegisteredDynamicTypes register_dynamic_types(
        const DynamicTypesCollection& dynamic_types);

/**
 * @brief Build Fast DDS \c DynamicType objects from registered \c TypeObject data.
 */
DDSRECORDER_PARTICIPANTS_DllAPI
std::map<std::string, fastdds::dds::DynamicType::_ref_type> build_dynamic_types(
        const RegisteredDynamicTypes& registered_types);

/**
 * @brief Extract only the registered type identifiers from a registered-types map.
 */
DDSRECORDER_PARTICIPANTS_DllAPI
std::map<std::string, fastdds::dds::xtypes::TypeIdentifierPair> type_identifiers_from(
        const RegisteredDynamicTypes& registered_types);

} // namespace detail
} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
