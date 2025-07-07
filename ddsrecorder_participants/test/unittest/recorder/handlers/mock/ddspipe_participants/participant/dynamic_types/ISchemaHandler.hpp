// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SchemaHandler.hpp
 */

#pragma once

#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DynamicType;

namespace xtypes {

class TypeIdentifier;

} // namespace xtypes
} // namespace dds
} // namespace fastdds

namespace ddspipe {
namespace core {
namespace types {

class DdsTopic;
class RtpsPayloadData;

} // namespace types
} // namespace core

namespace participants {

/**
 * TODO
 */
class ISchemaHandler
{
public:

    virtual void add_schema(
            const fastdds::dds::DynamicType::_ref_type&,
            const fastdds::dds::xtypes::TypeIdentifier& )
    {

    }

    virtual void add_data(
            const ddspipe::core::types::DdsTopic&,
            ddspipe::core::types::RtpsPayloadData& )
    {

    }

};

} /* namespace participants */
} /* namespace ddspipe */
} /* namespace eprosima */