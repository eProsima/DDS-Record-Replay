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

#include <atomic>
#include <functional>
#include <thread>

#include <cpp_utils/event/EventHandler.hpp>
#include <cpp_utils/library/library_dll.h>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/wait/CounterWaitHandler.hpp>

namespace eprosima {
namespace utils {
namespace event {

/**
 * This object allows to read from a \c istream , commonly from \c std::cin .
 * Every time there is a message written in std::cin AND this object is enabled to read it, the string input
 * is given in callback call.
 *
 * @warning This EventHandler is different than others in the way that it is obligated to give the number of inputs
 * that it must read (and not read until stop). This is because there is no easy way to stop a thread that
 * is waiting in std::cin in a multi-platform way.
 * This also implies that once the handler is waiting for stdin input, there is no way to stop it.
 */
class StdinEventHandler : public EventHandler<std::string>
{
public:

    /**
     * @brief Construct a new Stdin Event Handler with specific callback
     *
     * @param callback : callback to call when there is new data in stdin
     * @param read_lines : whether to read whole lines or read separated in spaces
     * @param lines_to_read : number of lines that this EventHandler must expect. Could be incremented by
     *  \c read_one_more_line .
     * @param source : source \c istream to get data from.
     *
     * @note \c source is very useful for testing purpose.
     * However, it does not make much sense in any other scenario, and it is dangerous to use something different
     * than std::cin, as the object stores a reference, and this referenced istream must survive this object.
     */
    CPP_UTILS_DllAPI
    StdinEventHandler(
            std::function<void(std::string)> callback,
            const bool read_lines = true,
            const int lines_to_read = 0,
            std::istream& source = std::cin);

    /**
     * @brief Destroy the StdinEventHandler object
     *
     * Calls \c unset_callback
     */
    CPP_UTILS_DllAPI
    ~StdinEventHandler();

    /**
     * @brief In order to read more than the lines given by ctor argument, use this method to increase value by 1.
     *
     * There is no easy way to stop a thread reading in a istream.
     * Thus the handler must know how much times it should read.
     * This method increase in 1 the number of times to read.
     *
     * The standard use of this class is to call this method each time a stdin input is expected.
     */
    CPP_UTILS_DllAPI
    void read_one_more_line();

protected:

    /**
     * @brief Internal thread to read from \c source_ .
     *
     * This thread waits first to this object to give it permission to start waiting for data in \c source_ by
     * waiter \c activation_times_ .
     * Every time allowed, it waits for 1 new input and give it by calling callback.
     *
     * @warning callback is called from this method, so until the
     * callback does not finish, the object cannot start reading again.
     */
    CPP_UTILS_DllAPI
    void stdin_listener_thread_routine_() noexcept;

    /**
     * @brief Override \c callback_set_ from \c EventHandler .
     *
     * It starts reading thread.
     *
     * It is already guarded by \c event_mutex_ .
     */
    virtual void callback_set_nts_() noexcept override;

    /**
     * @brief Override \c callback_set_ from \c EventHandler .
     *
     * It stops reading thread.
     *
     * It is already guarded by \c event_mutex_ .
     */
    virtual void callback_unset_nts_() noexcept override;

    //! Reading thread
    std::thread stdin_listener_thread_;

    //! Counter that contains the number of times the thread is allowed to start waiting for data from source_.
    CounterWaitHandler activation_times_;

    /**
     * @brief istream source from where to read.
     *
     * This is very useful for testing.
     * However it is dangerous to have a reference here to an external object.
     * Commonly this will be std::cin that does not die till the end of process.
     */
    std::istream& source_;

    //! Whether to read whole lines or stop reading in a space.
    const bool read_lines_;
};

} /* namespace event */
} /* namespace utils */
} /* namespace eprosima */
