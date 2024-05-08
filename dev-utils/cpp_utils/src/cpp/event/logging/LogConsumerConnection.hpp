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
 * @file LogConsumerConnection.hpp
 */

#pragma once

#include <atomic>
#include <functional>

#include <cpp_utils/Log.hpp>
#include <cpp_utils/event/LogEventHandler.hpp>
#include <cpp_utils/memory/owner_ptr.hpp>
#include <cpp_utils/library/library_dll.h>

namespace eprosima {
namespace utils {
namespace event {

/**
 * This class represents a \c LogConsumer that will be registered in Fast DDS Log as a unique_ptr and with each
 * \c Entry consumed it will call a shared function from a \c LogEventHandler .
 * As long as the EventHandler exists, it will manage these callbacks. When it dies, this object ptr to the function
 * will no longer be valid and thus it will do nothing.
 */
class LogConsumerConnection : public utils::LogConsumer
{
public:

    //! Construct this class with a Lessee from the actual shared ptr.
    CPP_UTILS_DllAPI LogConsumerConnection(
            LesseePtr<LogConsumerConnectionCallbackType>&& callback);

    //! Default destructor
    CPP_UTILS_DllAPI ~LogConsumerConnection() noexcept = default;

    /**
     * @brief Implements \c LogConsumer \c Consume method.
     *
     * This will call \c callback_ as longs as it is valid.
     * Notice that while calling it the function could not be removed, as it will be guarded from a \c GuardedPtr .
     *
     * @param entry entry to consume
     */
    CPP_UTILS_DllAPI void Consume(
            const utils::Log::Entry& entry) override;

protected:

    //! Lessee to the shared callback object.
    LesseePtr<LogConsumerConnectionCallbackType> callback_;
};

} /* namespace event */
} /* namespace utils */
} /* namespace eprosima */
