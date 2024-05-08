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

#include <cstddef>
#include <iterator>
#include <map>
#include <mutex>
#include <new>
#include <type_traits>
#include <shared_mutex>

#include <cpp_utils/collection/database/IModificableDatabase.hpp>

namespace eprosima {
namespace utils {

/**
 * @brief Iterator over \c SafeDatabase .
 *
 * This iterator keep the database shared locked until it is destroyed.
 * Thus, the database cannot change (add, modify, erase) while the iterator exists.
 * However, other iterators and read methods could still be used while iterator exists.
 *
 * @attention this iterator blocks access to database, so keep it alive as less as possible.
 *
 * @tparam \c Key key type of the SafeDatabase.
 * @tparam \c Value internal value type of the SafeDatabase.
 *
 * @warning while using this iterator a shared mutex is locked.
 * This shared mutex works differently between Linux and Windows. In windows a unique lock call blocks every other
 * future shared lock, and it keeps the mutex locked until the shared that have it release it.
 * Thus, if using these iterators in a loop, be careful of setting end() (or stop condition variable) before the
 * loop and not in every iteration.
 */
template <typename Key, typename Value>
class SafeDatabaseIterator : public std::map<Key, Value>::const_iterator
{
public:

    SafeDatabaseIterator(
            typename std::map<Key, Value>::const_iterator it,
            std::shared_timed_mutex& mutex);

    ~SafeDatabaseIterator();

private:

    std::shared_timed_mutex& mutex_;
};

/**
 * This class implements the Interface \c IModificableDatabase in a thread safe way.
 *
 * This represents a map of keys and values giving the methods require by IDatabase including modify and erase.
 * It uses an internal std::map to store the data.
 *
 * The iteration over the internal values is thread safe.
 * This means that while there is an alive iterator, the database could not change it state (add, modify, erase).
 * However, it could still be asked for values or iterate from somewhere else.
 *
 * @motivation this class is required to instantiate the interface \c IModificableDatabase and it also represents
 * a thread safe std::map.
 *
 * It adds the method \c at , \c size and some other utilities along with those of the interface.
 *
 * @tparam \c Key type to use as key/index of map.
 * @tparam \c Value type to use as internal value stored on map.
 */
template <typename Key, typename Value>
class SafeDatabase : public IModificableDatabase<Key, Value, SafeDatabaseIterator<Key, Value>>
{
public:

    //! Override \c modify \c IDatabase method.
    bool add(
            Key&& key,
            Value&& value) override;

    //! Override \c modify \c IDynamicDatabase method.
    bool modify(
            const Key& key,
            Value&& value) override;

    //! Override \c add_or_modify \c IDynamicDatabase method.
    bool add_or_modify(
            Key&& key,
            Value&& value) override;

    //! Override \c erase \c IDynamicDatabase method.
    bool erase(
            const Key& key) override;

    //! Override \c is \c IDatabase method.
    bool is(
            const Key& key) const override;

    //! Override \c find \c IDatabase method.
    SafeDatabaseIterator<Key, Value> find(
            const Key& key) const override;

    //! Override \c begin \c IDatabase method.
    SafeDatabaseIterator<Key, Value> begin() const override;

    //! Override \c end \c IDatabase method.
    SafeDatabaseIterator<Key, Value> end() const override;

    /**
     * @brief Add using copy semantics instead of movement.
     *
     * This is useful for those types that do not safe time moving (as native types) or when the scope of the variables
     * to add to the database do not allow movement (const values).
     *
     * @param key index or key for the new element
     * @param value new value to store
     *
     * @return true if the element was correctly added.
     * @return false if key is repeated.
     */
    bool add(
            const Key& key,
            const Value& value);

    /**
     * @brief Return a copy of the value indexed by \c key .
     *
     * Efficient call to return a copy of the internal value stored under \c key index.
     * It throws in case the key is not present.
     *
     * @attention This method is only instantiated for those types that could be copied.
     *
     * @tparam V This must be equal to Database Value type.
     * @warning V must be equal to Database Value type.
     *
     * @param key index of the value to look for.
     *
     * @return copy of the internal value if exist.
     * @throw \c std::out_of_range if key not in database.
     */
    Value at(
            const Key& key) const;

    //! Number of keys stored.
    unsigned int size() const noexcept;

    //! \c add_or_modify using copy semantics instead of movement.
    bool add_or_modify(
            const Key& key,
            const Value& value);

protected:

    /**
     * @brief The data is stored internally in this std map.
     *
     * This is guarded by \c mutex_ .
     * To iterate the map from outside, it uses a custom iterator that locks write access while exist.
     */
    std::map<Key, Value> internal_db_;

    /**
     * @brief Guard access to internal map.
     *
     * It shares lock for read methods (iterate, find, is, at, size)
     * It uses unique lock for write methods (add, modify, erase)
     */
    mutable std::shared_timed_mutex mutex_;
};

} /* namespace utils */
} /* namespace eprosima */

// Include implementation template file
#include <cpp_utils/collection/database/impl/SafeDatabase.ipp>
