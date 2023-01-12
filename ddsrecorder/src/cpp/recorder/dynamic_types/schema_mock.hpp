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
 * @file schema.hpp
 */

#pragma once

#include <fastrtps/types/DynamicTypePtr.h>

namespace eprosima {
namespace ddsrecorder {
namespace core {
namespace recorder {

/**
 * @brief Generate schema strings for types of IntrospectionHelloWorldExample types.
 *
 * @note this is an old Mock implementation for testing only.
 */
std::string generate_dyn_type_schema_mock(
        const fastrtps::types::DynamicType_ptr& dynamic_type);

} /* namespace recorder */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
