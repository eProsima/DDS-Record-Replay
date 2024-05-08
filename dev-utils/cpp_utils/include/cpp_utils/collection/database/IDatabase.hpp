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

#include <utility>

namespace eprosima {
namespace utils {

/**
 * Class that represents a generic database of values indexed by key.
 *
 * Database is a really extended interface. These are the uses for what this is made:
 * - thread safe in its access and iteration.
 * - easy accessors to internal values.
 * - methods to be notify with changes (specialized classes).
 *
 * @tparam Key type of the key that indexes the values.
 * @tparam Value type of the values stored inside.
 * @tparam Iterator type of the iterator to iterate over elements.
 *
 * @todo this is difficult to use because of Iterator type. This should change in the future.
 */
template <typename Key, typename Value, typename Iterator>
class IDatabase
{
public:

    /**
     * @brief Add a new element to the database by moving it.
     *
     * If the key already exists in the database, the element will not be added.
     *
     * @note this interface only uses movement because it is the only way to avoid unnecessary copies
     * and allow to pass every kind of type.
     * However It may make more difficult to use from a user.
     *
     * @param key index or key for the new element
     * @param value new value to store
     *
     * @return true if the element was correctly added.
     * @return false if key is repeated.
     */
    virtual bool add(
            Key&& key,
            Value&& value) = 0;

    /**
     * @brief Whether an element with this key exist in the database.
     *
     * @param key key to look for.
     * @return true if there is an element stored with such key.
     * @return false if the key does not exist.
     */
    virtual bool is(
            const Key& key) const = 0;

    /**
     * @brief Look for a value indexed with a key.
     *
     * @param key key to look for.
     * @return Iterator pointing to the element to find. It points to end() if the element is not found.
     */
    virtual Iterator find(
            const Key& key) const = 0;

    /**
     * @brief Points to the first element on the database.
     *
     * @note As Database must be thread safe, these iterators force the database to be kept in a coherent state.
     * This means that holding one of this iterators for too long may block other database functionality.
     *
     * @return Iterator point to the first element in the database.
     */
    virtual Iterator begin() const = 0;

    /**
     * @brief Points to the last + 1 element on the database.
     *
     * @note As Database must be thread safe, these iterators force the database to be kept in a coherent state.
     * This means that holding one of this iterators for too long may block other database functionality.
     *
     * @return Iterator point to the end of the database. No element is stored under this iterator.
     */
    virtual Iterator end() const = 0;
};

} /* namespace utils */
} /* namespace eprosima */
