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

namespace eprosima {
namespace ddsrecorder {
namespace replayer {
namespace detail {

/**
 * @brief Dynamic-type data registered in Fast DDS for later replay or conversion use.
 *
 * This structure stores the same pieces that the former
 * \c DdsReplayer::register_dynamic_types_ implementation produced inline:
 * - the \c TypeObject decoded from the MCAP attachment
 * - the \c TypeIdentifierPair assigned by the Fast DDS TypeObjectRegistry
 *
 * Keeping both values allows callers to either:
 * - attach type identifiers back to discovered topics for replay, or
 * - build \c DynamicType instances for payload deserialization in conversion mode
 */
struct RegisteredDynamicType
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
 *
 * @param dynamic_types Collection read from the MCAP \c dynamic_types attachment.
 *
 * @return A map with the registered type data indexed by type name.
 *
 * @note This helper was extracted from the original
 *       \c DdsReplayer::register_dynamic_types_ implementation so the same
 *       registration path can be reused by replay mode and by MCAP-to-SQL conversion mode.
 */
RegisteredDynamicTypes register_dynamic_types(
        const participants::DynamicTypesCollection& dynamic_types);

/**
 * @brief Build Fast DDS \c DynamicType objects from registered \c TypeObject data.
 *
 * @param registered_types Types previously decoded and registered with Fast DDS.
 *
 * @return A map of built \c DynamicType instances indexed by DDS type name.
 *
 * @note This is used by the SQL converter to deserialize serialized MCAP payloads
 *       with the same Fast DDS dynamic-type machinery used elsewhere in the workspace.
 */
std::map<std::string, fastdds::dds::DynamicType::_ref_type> build_dynamic_types(
        const RegisteredDynamicTypes& registered_types);

/**
 * @brief Extract only the registered type identifiers from a registered-types map.
 *
 * @param registered_types Types previously decoded and registered with Fast DDS.
 *
 * @return A map from DDS type name to \c TypeIdentifierPair.
 *
 * @note Replay mode uses this helper to populate the built-in topics inserted into DDS Pipe.
 */
std::map<std::string, fastdds::dds::xtypes::TypeIdentifierPair> type_identifiers_from(
        const RegisteredDynamicTypes& registered_types);

} // namespace detail
} /* namespace replayer */
} /* namespace ddsrecorder */
} /* namespace eprosima */
