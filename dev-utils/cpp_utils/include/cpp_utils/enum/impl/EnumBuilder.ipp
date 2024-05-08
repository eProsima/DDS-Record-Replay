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
 * @file EnumBuilder.ipp
 *
 * This file contains class EnumBuilder implementation.
 */

#pragma once

#include <cpp_utils/exception/ValueNotAllowedException.hpp>

namespace eprosima {
namespace utils {

template<typename E>
bool EnumBuilder<E>::string_to_enumeration(
        const std::string& enum_str,
        E& enum_value) const noexcept
{
    return Builder<std::string, E>::find(enum_str, enum_value);
}

template<typename E>
E EnumBuilder<E>::string_to_enumeration(
        const std::string& enum_str) const
{
    return Builder<std::string, E>::find(enum_str);
}

} /* namespace utils */
} /* namespace eprosima */
