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
 * @file EnumBuilder.hpp
 *
 * This file contains class EnumBuilder definition.
 */

#pragma once

#include <map>
#include <set>
#include <string>

#include <cpp_utils/types/InitializableSingleton.hpp>
#include <cpp_utils/types/Builder.hpp>

namespace eprosima {
namespace utils {

/**
 * @brief Class that converts a string to an enumeration value.
 *
 * This class has an internal map with values of an enumeration (not necessarily all) that points to set of strings
 * with the strings that each value has associated.
 * In order to retrieve an enumeration value from a string, the string is looked for in the sets until it is found.
 *
 * @note This class could be used as a Singleton associated with an enumeration.
 * Use \c eProsima_ENUMERATION_BUILDER to initialize the values in compilation time and use them in all the process.
 *
 * @example
 * enum class CustomEnumeration{ value_1, value_2 };
 * eProsima_ENUMERATION_BUILDER(CustomEnumerationBuilder, CustomEnumeration,
 *  { { CustomEnumeration::value_1 , "v1" } , { CustomEnumeration::value_2 , "v2" } );
 *
 * // Somewhere in the process
 * auto res = CustomEnumerationBuilder::get_instance()->string_to_enumeration("v2"); // res = CustomEnumeration::value_2
 */
template <typename E>
class EnumBuilder : public Builder<std::string, E>
{
public:

    using Builder<std::string, E>::Builder;

    /**
     * @brief Give the enumeration value associated with the string given.
     *
     * @param [in] enum_str string associated with a value.
     * @param [out] enum_value enumeration value associated.
     * @return true if the string \c enum_str has an associated value.
     * @return false otherwise.
     */
    bool string_to_enumeration(
            const std::string& enum_str,
            E& enum_value) const noexcept;

    /**
     * @brief Give the enumeration value associated with the string given.
     *
     * @param [in] enum_str string associated with a value.
     * @param [out] enum_value enumeration value associated.
     * @return the related enumeration value for the string given.
     *
     * @throw \c ValueNotAllowedException if there is no value related with this string.
     */
    E string_to_enumeration(
            const std::string& enum_str) const;
};

/**
 * Using the following macro along with the declaration of the Enumeration, the values of the enumeration
 * will be associated in compilation time with the strings that create them.
 *
 * Using \c builder_name singleton enables the access to the Builder in the whole process.
 *
 * @param builder_name Name for the Singleton that refers to the Builder.
 * @param enum_name name of the enumeration.
 * @param values_map Map of values with key the enumeration values, and value a set of strings.
 */
#define eProsima_ENUMERATION_BUILDER(builder_name, enum_name, values_map) \
    typedef eprosima::utils::InitializableSingleton<eprosima::utils::EnumBuilder< enum_name >, 0> builder_name; \
    const bool __STATUS_INITIALIZATION_ ## builder_name = \
            builder_name::initialize<const std::map< enum_name, std::set<std::string>>&>( values_map )

} /* namespace utils */
} /* namespace eprosima */

// Include implementation template file
#include <cpp_utils/enum/impl/EnumBuilder.ipp>
