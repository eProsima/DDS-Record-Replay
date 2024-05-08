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
 * @file Builder.ipp
 *
 * This file contains class Builder implementation.
 */

#pragma once

#include <cpp_utils/exception/ValueNotAllowedException.hpp>

namespace eprosima {
namespace utils {

template <typename Key, typename Value>
Builder<Key, Value>::Builder(
        const std::map<Key, Value>& keys_to_values)
    : values_(keys_to_values)
{
    // Do nothing
}

template <typename Key, typename Value>
Builder<Key, Value>::Builder(
        const std::map<Value, std::set<Key>>& values_to_keys)
    : Builder(indexed_map_from_values_to_keys_(values_to_keys))
{
    // Do nothing
}

template <typename Key, typename Value>
void Builder<Key, Value>::refactor_values(
        const std::map<Key, Value>& keys_to_values)
{
    values_ = indexed_map_from_values_to_keys_(keys_to_values);
}

template <typename Key, typename Value>
void Builder<Key, Value>::refactor_values(
        const std::map<Value, std::set<Key>>& values_to_keys)
{
    values_ = indexed_map_from_values_to_keys_(values_to_keys);
}

template <typename Key, typename Value>
bool Builder<Key, Value>::find(
        const Key& key,
        Value& return_value) const noexcept
{
    auto it = values_.find(key);
    if (it != values_.end())
    {
        return_value = it->second;
        return true;
    }
    else
    {
        return false;
    }
}

template <typename Key, typename Value>
Value Builder<Key, Value>::find(
        const Key& key) const
{
    Value value;
    if (find(key, value))
    {
        return value;
    }
    else
    {
        throw ValueNotAllowedException(
                  STR_ENTRY <<
                      "Value " << key << " is not valid for enumeration " << TYPE_NAME(this) << "."
                  );
    }
}

template <typename Key, typename Value>
std::map<Key, Value> Builder<Key, Value>::indexed_map_from_values_to_keys_(
        const std::map<Value, std::set<Key>>& values_to_keys)
{
    std::map<Key, Value> result;
    for (const auto& it : values_to_keys)
    {
        for (const auto& k : it.second)
        {
            result[k] = it.first;
        }
    }
    return result;
}

} /* namespace utils */
} /* namespace eprosima */
