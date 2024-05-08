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

namespace eprosima {
namespace utils {


template <typename Key, typename Value>
SafeDatabaseIterator<Key, Value>::SafeDatabaseIterator(
        typename std::map<Key, Value>::const_iterator it,
        std::shared_timed_mutex& mutex)
    : std::map<Key, Value>::const_iterator(it)
    , mutex_(mutex)
{
    mutex_.lock_shared();
}

template <typename Key, typename Value>
SafeDatabaseIterator<Key, Value>::~SafeDatabaseIterator()
{
    mutex_.unlock_shared();
}

template <typename Key, typename Value>
bool SafeDatabase<Key, Value>::add(
        Key&& key,
        Value&& value)
{
    std::unique_lock<std::shared_timed_mutex> _(mutex_);

    auto res = internal_db_.insert(std::move(std::pair<Key, Value>(std::move(key), std::move(value))));

    return res.second;
}

template <typename Key, typename Value>
bool SafeDatabase<Key, Value>::modify(
        const Key& key,
        Value&& value)
{
    std::unique_lock<std::shared_timed_mutex> _(mutex_);

    auto it = internal_db_.find(key);
    if (it == internal_db_.end())
    {
        return false;
    }

    it->second = std::move(value);

    return true;
}

template <typename Key, typename Value>
bool SafeDatabase<Key, Value>::erase(
        const Key& key)
{
    std::unique_lock<std::shared_timed_mutex> _(mutex_);

    return internal_db_.erase(key) != 0;
}

template <typename Key, typename Value>
bool SafeDatabase<Key, Value>::is(
        const Key& key) const
{
    std::shared_lock<std::shared_timed_mutex> _(mutex_);

    return internal_db_.find(key) != internal_db_.end();
}

template <typename Key, typename Value>
SafeDatabaseIterator<Key, Value> SafeDatabase<Key, Value>::find(
        const Key& key) const
{
    std::shared_lock<std::shared_timed_mutex> _(mutex_);

    return SafeDatabaseIterator<Key, Value>(internal_db_.find(key), mutex_);
}

template <typename Key, typename Value>
SafeDatabaseIterator<Key, Value> SafeDatabase<Key, Value>::begin() const
{
    return SafeDatabaseIterator<Key, Value>(internal_db_.begin(), mutex_);
}

template <typename Key, typename Value>
SafeDatabaseIterator<Key, Value> SafeDatabase<Key, Value>::end() const
{
    return SafeDatabaseIterator<Key, Value>(internal_db_.end(), mutex_);
}

template <typename Key, typename Value>
bool SafeDatabase<Key, Value>::add(
        const Key& key,
        const Value& value)
{
    return add(std::move(Key(key)), std::move(Value(value)));
}

template <typename Key, typename Value>
Value SafeDatabase<Key, Value>::at(
        const Key& key) const
{
    std::shared_lock<std::shared_timed_mutex> _(mutex_);

    return internal_db_.at(key);
}

template <typename Key, typename Value>
unsigned int SafeDatabase<Key, Value>::size() const noexcept
{
    std::shared_lock<std::shared_timed_mutex> _(mutex_);

    return internal_db_.size();
}

template <typename Key, typename Value>
bool SafeDatabase<Key, Value>::add_or_modify(
        Key&& key,
        Value&& value)
{
    std::unique_lock<std::shared_timed_mutex> _(mutex_);

    auto it = internal_db_.find(key);
    if (it == internal_db_.end())
    {
        // Add new value
        internal_db_.insert(std::move(std::pair<Key, Value>(std::move(key), std::move(value))));
        return true;
    }
    else
    {
        // Modify already existent value
        it->second = std::move(value);
        return false;
    }
}

template <typename Key, typename Value>
bool SafeDatabase<Key, Value>::add_or_modify(
        const Key& key,
        const Value& value)
{
    return add_or_modify(std::move(Key(key)), std::move(Value(value)));
}

} /* namespace utils */
} /* namespace eprosima */
