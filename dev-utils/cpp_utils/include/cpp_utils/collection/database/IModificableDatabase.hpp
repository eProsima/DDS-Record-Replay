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

#pragma once

#include <cpp_utils/collection/database/IDatabase.hpp>

namespace eprosima {
namespace utils {

/**
 * Inheritance from \c IDatabase that allows to modify or erase a value from a database.
 */
template <typename Key, typename Value, typename Iterator>
class IModificableDatabase : public IDatabase<Key, Value, Iterator>
{
public:

    /**
     * @brief Modify a value that already exist in the database.
     *
     * @param key key indexing the new or existent value.
     * @param value new value to set.
     *
     * @return true if value has been modified.
     * @return false if value did not exist in the database.
     */
    virtual bool modify(
            const Key& key,
            Value&& value) = 0;

    /**
     * @brief Add a new value or modify an existent one with the new value.
     *
     * @param key key indexing the new or existent value.
     * @param value new value to set.
     *
     * @return true if value has been added.
     * @return false if value already exists, in this case the value is modified.
     */
    virtual bool add_or_modify(
            Key&& key,
            Value&& value) = 0;

    /**
     * @brief Remove the value indexed by \c key that already exist in the database.
     *
     * @param key key indexing the value.
     *
     * @return true if value has been removed.
     * @return false if value did not exist in the database.
     */
    virtual bool erase(
            const Key& key) = 0;
};

} /* namespace utils */
} /* namespace eprosima */
