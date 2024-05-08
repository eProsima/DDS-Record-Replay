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
 * @file LogConsumerConnection.cpp
 *
 */

#include <cpp_utils/Log.hpp>
#include <cpp_utils/exception/InitializationException.hpp>

#include "event/logging/LogConsumerConnection.hpp"

namespace eprosima {
namespace utils {
namespace event {

LogConsumerConnection::LogConsumerConnection(
        LesseePtr<LogConsumerConnectionCallbackType>&& callback)
    : callback_(std::move(callback))
{
    // Do nothing
}

void LogConsumerConnection::Consume(
        const utils::Log::Entry& entry)
{
    // Check whether the callback still exists
    auto callback_persistent = callback_.lock();

    if (callback_persistent)
    {
        // In case it still exists, call it
        callback_persistent->operator ()(
                entry);
    }
}

} /* namespace event */
} /* namespace utils */
} /* namespace eprosima */
