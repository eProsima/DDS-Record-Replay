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
 * @file schema.cpp
 */

#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypePtr.h>

#include <cpp_utils/exception/UnsupportedException.hpp>

#include <auxiliar/dynamic_types/schema_mock.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {
namespace detail {

constexpr const char* HELLO_WORLD_SCHEMA = R"(
uint32 index
string message
)";

constexpr const char* ARRAY_SCHEMA = R"(
uint32 index
int32[3] points
)";

constexpr const char* PLAIN_SCHEMA = R"(
uint32 index
char[20] message
)";

constexpr const char* STRUCT_SCHEMA = R"(
uint32 index
InternalStruct_TypeIntrospectionExample internal_data
================================================================================
MSG: fastdds/InternalStruct_TypeIntrospectionExample
int32 x_member
int32 y_member
int32 z_member
)";

std::string generate_dyn_type_schema_mock(
        const fastrtps::types::DynamicType_ptr& dynamic_type)
{
    std::string type_name = dynamic_type->get_name();

    // WARNING: This is a temporal solution giving the schemas to the TypeIntrospectionExample types
    if (type_name == "HelloWorld_TypeIntrospectionExample")
    {
        return HELLO_WORLD_SCHEMA;
    }
    else if (type_name == "Array_TypeIntrospectionExample")
    {
        return ARRAY_SCHEMA;
    }
    else if (type_name == "Plain_TypeIntrospectionExample")
    {
        return PLAIN_SCHEMA;
    }
    else if (type_name == "Struct_TypeIntrospectionExample")
    {
        return STRUCT_SCHEMA;
    }

    throw utils::UnsupportedException(
        STR_ENTRY << "Type " << type_name << " is not supported.");
}

} /* namespace detail */
} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
