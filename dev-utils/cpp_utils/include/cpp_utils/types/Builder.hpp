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
 * @file Builder.hpp
 *
 * This file contains class Builder definition.
 */

#pragma once

#include <map>
#include <set>

namespace eprosima {
namespace utils {

/**
 * @brief Class that wraps a map functionality to build or get objects.
 *
 * It can be created from a map of key->value or from a map where every value is associated with multiple keys.
 * Both ways it uses an internal map to find the key and return the value expected by argument o by value.
 *
 * @tparam Key Type of the key to look in the map.
 * @tparam Value Type of the value that is stored in map.
 *
 * This is mainly used for \c EnumBuilder where keys are strings and values are enumeration values.
 */
template <typename Key, typename Value>
class Builder
{
public:

    /**
     * @brief Construct a new Builder object by giving the map of values.
     *
     * @param keys_to_values Map of key -> values.
     */
    Builder(
            const std::map<Key, Value>& keys_to_values);

    /**
     * @brief Construct a new Builder object by giving each associated key to each value.
     *
     * Each of the values will be associated to the keys in its map, and could be get by any of them.
     *
     * @param values_to_keys Map of values and their respective keys.
     */
    Builder(
            const std::map<Value, std::set<Key>>& values_to_keys);

    //! Change the internal map of values for a new one.
    void refactor_values(
            const std::map<Key, Value>& keys_to_values);

    //! Change the internal map of values for a new one in the format value -> set(keys)
    void refactor_values(
            const std::map<Value, std::set<Key>>& values_to_keys);

    /**
     * @brief Give the value associated with the key given.
     *
     * @param [in] key key to look in the internal map.
     * @param [out] return_value value associated.
     * @return true if the string \c key has an associated value.
     * @return false otherwise.
     */
    bool find(
            const Key& key,
            Value& return_value) const noexcept;

    /**
     * @brief Give the value associated with the key given or throw exception if not present.
     *
     * @param [in] key key to look in the internal map
     * .
     * @return value associated.
     *
     * @throw \c ValueNotAllowedException if there is no value related with this string.
     */
    Value find(
            const Key& key) const;

protected:

    //! Convert a map of values to set of keys to a map of key -> value.
    std::map<Key, Value> indexed_map_from_values_to_keys_(
            const std::map<Value, std::set<Key>>& values_to_keys);

    //! Map with the keys and the values associated.
    std::map<Key, Value> values_ {};

};

} /* namespace utils */
} /* namespace eprosima */

// Include implementation template file
#include <cpp_utils/types/impl/Builder.ipp>
