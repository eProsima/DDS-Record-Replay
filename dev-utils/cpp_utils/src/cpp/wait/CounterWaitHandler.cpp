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
 * @file CounterWaitHandler.cpp
 *
 */

#include <cpp_utils/Log.hpp>

#include <cpp_utils/wait/CounterWaitHandler.hpp>

namespace eprosima {
namespace utils {
namespace event {

CounterWaitHandler::CounterWaitHandler(
        CounterType threshold,
        CounterType initial_value,
        bool enabled /* = true */)
    : WaitHandler<CounterType>(initial_value, enabled)
    , threshold_(threshold)
{
}

CounterWaitHandler::~CounterWaitHandler()
{
}

AwakeReason CounterWaitHandler::wait_and_decrement(
        const utils::Duration_ms& timeout /* = 0 */) noexcept
{
    AwakeReason result; // Get value from wait
    CounterType threshold_tmp = threshold_; // Require to set it in predicate

    // Perform blocking wait
    auto lock = blocking_wait_(
        std::function<bool(const CounterType&)>([threshold_tmp](const CounterType& value)
        {
            return value > threshold_tmp;
        }),
        timeout,
        result);

    // Mutex is taken, decrease value by 1 if condition was met
    if (result == AwakeReason::condition_met)
    {
        decrease_1_nts_();
    }

    return result;
}

AwakeReason CounterWaitHandler::wait_threshold_reached(
        const utils::Duration_ms& timeout /* = 0 */) noexcept
{
    // Do wait with mutex taken
    std::unique_lock<std::mutex> lock(wait_condition_variable_mutex_);

    // Check if it is disabled and exit
    if (!enabled())
    {
        return AwakeReason::disabled;
    }

    // Increment number of threads waiting
    // WARNING: mutex must be taken
    threads_waiting_++;

    utils::Timestamp time_to_wait_until;

    // If timeout is 0, use wait, if not use wait for timeout
    if (timeout > 0)
    {
        time_to_wait_until = utils::now() + utils::duration_to_ms(timeout);
    }
    else
    {
        time_to_wait_until = utils::the_end_of_time();
    }

    bool finished_for_condition_met = threshold_reached_cv_.wait_until(
        lock,
        time_to_wait_until,
        [this]
        {
            // Exit if threshold reached or if this has been disabled
            return !enabled_.load() || (value_ == threshold_);
        });

    // Decrement number of threads waiting
    // NOTE: mutex is still taken
    threads_waiting_--;

    // Check awake reason. Mutex is taken so it can not change while checking
    if (!enabled_.load())
    {
        return AwakeReason::disabled;
    }
    else if (finished_for_condition_met)
    {
        return AwakeReason::condition_met;
    }
    else
    {
        return AwakeReason::timeout;
    }
}

CounterWaitHandler& CounterWaitHandler::operator ++()
{
    // NOTE: This operation could be done using the WaitHandler methods, but it will be less efficient than
    // set the actual value directly.

    {
        // Mutex must guard the modification of value_
        std::lock_guard<std::mutex> lock(wait_condition_variable_mutex_);
        value_++;

        // If threshold is reached, notify one waiter
        if (value_ > threshold_)
        {
            wait_condition_variable_.notify_one();
        }
    }

    return *this;
}

void CounterWaitHandler::decrease_1_nts_()
{
    value_--;

    // If value is still higher than threshold, notify one waiter
    if (value_ > threshold_)
    {
        wait_condition_variable_.notify_one();
    }
    // If the threshold is reached, notify threads waiting for this event
    else if (value_ == threshold_)
    {
        threshold_reached_cv_.notify_all();
    }
}

} /* namespace event */
} /* namespace utils */
} /* namespace eprosima */
