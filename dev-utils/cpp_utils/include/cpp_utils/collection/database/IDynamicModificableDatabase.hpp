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

#include <cpp_utils/collection/database/IModificableDatabase.hpp>

namespace eprosima {
namespace utils {

/**
 * Database specialization that allow to register callbacks to get notified each time a new element is added,
 * modified or removed.
 */
template <typename Key, typename Value, typename Iterator>
class IDynamicModificableDatabase : public IModificableDatabase<Key, Value, Iterator>,  public IDynamicDatabase<Key,
            Value, Iterator>
{
public:

    /**
     * @brief Registering a callback to receive a notification every time a data is modified in the database.
     *
     * @param callback function to call when value modified.
     */
    virtual void register_callback_modify(void(const Key&, const Value&) && callback) = 0;

    /**
     * @brief Registering a callback to receive a notification every time a data is removed from the database.
     *
     * @param callback function to call when value removed.
     */
    virtual void register_callback_remove(void(const Key&, const Value&) && callback) = 0;

};

} /* namespace utils */
} /* namespace eprosima */
