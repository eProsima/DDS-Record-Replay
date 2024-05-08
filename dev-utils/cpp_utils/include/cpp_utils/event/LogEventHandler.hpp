// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file LogEventHandler.hpp
 */

#pragma once

#include <atomic>
#include <functional>

#include <cpp_utils/Log.hpp>
#include <cpp_utils/types/Atomicable.hpp>
#include <cpp_utils/event/EventHandler.hpp>
#include <cpp_utils/memory/owner_ptr.hpp>
#include <cpp_utils/library/library_dll.h>

namespace eprosima {
namespace utils {
namespace event {

//! Data Type to be shared between a LogEventHandler and a LogConsumerConnection.
using LogConsumerConnectionCallbackType = std::function<void (const utils::Log::Entry&)>;

/**
 * It implements the functionality to raise callback every time a Log msg is consumed.
 *
 * As Fast DDS Log requires to own a unique_ptr with a consumer, this class is separated from \c LogConsumer.
 * The actual \c LogConsumer used is of class \c LogConsumerConnection and every time it consumes an \c Entry ,
 * it calls this class.
 * As \c LogConsumerConnection variable will survive this object, an owner/lessee object is shared between
 * both objects, so connection keeps calling this callback as long as this object lives, and after this death
 * it will do nothing.
 */
class LogEventHandler : public EventHandler<utils::Log::Entry>
{
public:

    /**
     * Construct without callback.
     *
     * It registers in Log the LogConsumer that will call this object.
     */
    CPP_UTILS_DllAPI LogEventHandler();

    /**
     * Construct a Log Event Handler with callback and enable it.
     *
     * It registers in Log the LogConsumer that will call this object.
     *
     * Calls \c set_callback .
     *
     * @param callback callback to call every time a log entry is consumed.
     */
    CPP_UTILS_DllAPI LogEventHandler(
            std::function<void(utils::Log::Entry)> callback);

    /**
     * @brief Destroy the LogEventHandler object
     *
     * Calls \c unset_callback
     */
    CPP_UTILS_DllAPI ~LogEventHandler();

protected:

    /**
     * @brief Consumes an \c Entry given from the \c LogConsumerConnection .
     *
     * @param entry entry consumed.
     */
    CPP_UTILS_DllAPI virtual void consume_(
            const utils::Log::Entry& entry);

    /**
     * @brief Shared object between this and \c LogConsumerConnection registered.
     *
     * When this is destroyed, the ptr is released and the lessee in \c LogConsumerConnection will no longer
     * be valid, so that object will do nothing with any new Entry.
     */
    OwnerPtr<LogConsumerConnectionCallbackType> connection_callback_;

    /**
     * @brief Vector of Log entries consumed so far.
     *
     * Guarded by itself.
     */
    SharedAtomicable<std::vector<utils::Log::Entry>> entries_consumed_;
};

} /* namespace event */
} /* namespace utils */
} /* namespace eprosima */
