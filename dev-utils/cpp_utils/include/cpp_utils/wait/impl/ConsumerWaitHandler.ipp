// Copyright 2022
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
 * @file ConsumerWaitHandler.ipp
 */

#include <cpp_utils/exception/DisabledException.hpp>
#include <cpp_utils/exception/TimeoutException.hpp>
#include <cpp_utils/Log.hpp>

#pragma once

namespace eprosima {
namespace utils {
namespace event {

template <typename T>
ConsumerWaitHandler<T>::ConsumerWaitHandler(
        CounterType initial_value /* = 0 */,
        bool enabled /* = true */)
    : CounterWaitHandler(0, initial_value, enabled)
{
    logDebug(UTILS_WAIT_CONSUMER, "Created Consumer Wait Handler with type " << TYPE_NAME(T) << ".");
}

template <typename T>
CounterType ConsumerWaitHandler<T>::elements_ready_to_consume() const noexcept
{
    return get_value();
}

template <typename T>
void ConsumerWaitHandler<T>::produce(
        T&& value)
{
    add_value_(std::move(value));
    this->operator ++();
}

template <typename T>
void ConsumerWaitHandler<T>::produce(
        const T& value)
{
    add_value_(value);
    this->operator ++();
}

template <typename T>
T ConsumerWaitHandler<T>::consume(
        const utils::Duration_ms& timeout /* = 0 */)
{
    AwakeReason reason = wait_and_decrement(timeout);

    // Check if reason has been condition met, else throw exception
    if (reason == AwakeReason::disabled)
    {
        throw utils::DisabledException("ConsumerWaitHandler has been disabled.");
    }
    else if (reason == AwakeReason::timeout)
    {
        throw utils::TimeoutException("ConsumerWaitHandler awaken by timeout.");
    }
    else
    {
        // This is taken without mutex protection
        return get_next_value_();
    }
}

template <typename T>
AwakeReason ConsumerWaitHandler<T>::wait_all_consumed(
        const utils::Duration_ms& timeout /* = 0 */)
{
    return wait_threshold_reached(timeout);
}

} /* namespace event */
} /* namespace utils */
} /* namespace eprosima */


