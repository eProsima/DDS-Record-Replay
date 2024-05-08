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
 * @file StdLogConsumer.hpp
 */

#pragma once

#include <iostream>

#include <cpp_utils/library/library_dll.h>
#include <cpp_utils/Log.hpp>
#include <cpp_utils/logging/BaseLogConfiguration.hpp>
#include <cpp_utils/logging/BaseLogConsumer.hpp>

namespace eprosima {
namespace utils {

/**
 * Std Log Consumer with Standard (logical) behaviour.
 *
 * Registering this consumer in Fast DDS's Log prints the log entries accepted by the BaseLogConsumer.
 * Info messages are printed in std::cout while others are sent to std::cerr.
 */
class StdLogConsumer : public BaseLogConsumer
{
public:

    CPP_UTILS_DllAPI
    StdLogConsumer(
            const BaseLogConfiguration* log_configuration);

    /**
     * @brief Implements the \c BaseLogConsumer \c Consume method.
     *
     * To be consumed, entries must be accepted by the \c BaseLogConsumer, so:
     * - Their kind must be higher or equal than the verbosity level.
     * - Their category or message must match the filter regex.
     *
     * Entries with a kind of Error or Warning will be printed in \c std::cerr.
     * Entries with a kind of Info will be printed in \c std::cout.
     *
     * @param entry entry to consume
     */
    CPP_UTILS_DllAPI
    void Consume(
            const Log::Entry& entry) override;

protected:

    /**
     * @brief Get which stream must be used depending on the entry
     *
     * @param entry to decide the output stream
     *
     * @return \c std::out if entry is Info, \c std::cerr otherwise.
     */
    CPP_UTILS_DllAPI
    virtual std::ostream& get_stream_(
            const Log::Entry& entry);
};

} /* namespace utils */
} /* namespace eprosima */
