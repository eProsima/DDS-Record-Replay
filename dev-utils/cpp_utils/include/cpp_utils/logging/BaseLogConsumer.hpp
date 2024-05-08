// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BaseLogConsumer.hpp
 */

#pragma once

#include <cpp_utils/library/library_dll.h>
#include <cpp_utils/Log.hpp>
#include <cpp_utils/logging/BaseLogConfiguration.hpp>

namespace eprosima {
namespace utils {

/**
 * Base Log Consumer with Standard (logical) behaviour.
 *
 * This consumer configures the LogConsumers to filter log entries such that:
 * - Their kind is higher or equal than the verbosity level.
 * - Their category or message matches the filter regex.
 *
 * @attention This consumer filters the entries that it receives, but other entries could be filtered beforehand
 * by Fast DDS Log. To avoid this consumer's filters, set the verbosity to Info and do not filter the content.
 */
class BaseLogConsumer : public utils::LogConsumer
{
public:

    //! Create new BaseLogConsumer with a determined Log Configuration.
    CPP_UTILS_DllAPI
    BaseLogConsumer(
            const BaseLogConfiguration* log_configuration);

    //! Default destructor
    CPP_UTILS_DllAPI
    ~BaseLogConsumer() noexcept = default;

protected:

    //! Whether the entry must be accepted depending on kind and category
    CPP_UTILS_DllAPI
    virtual bool accept_entry_(
            const Log::Entry& entry);

    //! Regex filter for entry category or message.
    LogFilter filter_;

    //! Maximum Log Kind that will be printed.
    VerbosityKind verbosity_;
};

} /* namespace utils */
} /* namespace eprosima */
