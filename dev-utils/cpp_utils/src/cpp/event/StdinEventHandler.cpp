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

#include <thread>

#include <cpp_utils/Log.hpp>
#include <cpp_utils/event/StdinEventHandler.hpp>
#include <cpp_utils/exception/InitializationException.hpp>

namespace eprosima {
namespace utils {
namespace event {

StdinEventHandler::StdinEventHandler(
        std::function<void(std::string)> callback,
        const bool read_lines /* = true */,
        const int lines_to_read /* = 0 */,
        std::istream& source /* = std::cin */)
    : activation_times_(0, lines_to_read, false)
    , source_(source)
    , read_lines_(read_lines)
{
    set_callback(callback);
}

StdinEventHandler::~StdinEventHandler()
{
    unset_callback();
}

void StdinEventHandler::read_one_more_line()
{
    ++activation_times_;
}

void StdinEventHandler::stdin_listener_thread_routine_() noexcept
{
    auto awake_reason = activation_times_.wait_and_decrement();
    while (awake_reason == AwakeReason::condition_met)
    {
        std::string read_str;

        // Read lines or separated by spaces
        if (read_lines_)
        {
            getline(source_, read_str);
        }
        else
        {
            source_ >> read_str;
        }
        event_occurred_(read_str);

        awake_reason = activation_times_.wait_and_decrement();
    }
}

void StdinEventHandler::callback_set_nts_() noexcept
{
    activation_times_.enable();
    stdin_listener_thread_ = std::thread(&StdinEventHandler::stdin_listener_thread_routine_, this);
}

void StdinEventHandler::callback_unset_nts_() noexcept
{
    activation_times_.disable();
    stdin_listener_thread_.join();
}

} /* namespace event */
} /* namespace utils */
} /* namespace eprosima */
