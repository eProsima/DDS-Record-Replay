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
 * @file LogEventHandler.cpp
 *
 */

#include <cpp_utils/Log.hpp>
#include <cpp_utils/event/LogEventHandler.hpp>
#include <cpp_utils/exception/InitializationException.hpp>

#include "event/logging/LogConsumerConnection.hpp"

namespace eprosima {
namespace utils {
namespace event {

LogEventHandler::LogEventHandler()
    : EventHandler<utils::Log::Entry>()
    , connection_callback_(
        new LogConsumerConnectionCallbackType(
            [this]
                (const utils::Log::Entry& entry)
            {
                this->consume_(entry);
            }))
{
    // Create LogConsumer and register it
    Log::RegisterConsumer(std::make_unique<LogConsumerConnection>(connection_callback_.lease()));
}

LogEventHandler::LogEventHandler(
        std::function<void(utils::Log::Entry)> callback)
    : LogEventHandler()
{
    // Set callback
    set_callback(callback);
}

LogEventHandler::~LogEventHandler()
{
    unset_callback();
}

void LogEventHandler::consume_(
        const utils::Log::Entry& entry)
{
    {
        std::lock_guard<SharedAtomicable<std::vector<utils::Log::Entry>>> lock(entries_consumed_);
        entries_consumed_.push_back(entry);
    }

    event_occurred_(entry);
}

} /* namespace event */
} /* namespace utils */
} /* namespace eprosima */
