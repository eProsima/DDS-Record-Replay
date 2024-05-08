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
 * @file StdLogConsumer.cpp
 *
 */

#include <iostream>

#include <cpp_utils/logging/StdLogConsumer.hpp>

namespace eprosima {
namespace utils {

StdLogConsumer::StdLogConsumer(
        const BaseLogConfiguration* log_configuration)
    : BaseLogConsumer(log_configuration)
{
}

void StdLogConsumer::Consume(
        const Log::Entry& entry)
{
    if (!accept_entry_(entry))
    {
        return;
    }

    std::ostream& stream = get_stream_(entry);
    print_timestamp(stream, entry, true);
    print_header(stream, entry, true);
    print_message(stream, entry, true);
    print_context(stream, entry, true);
    print_new_line(stream, true);
    stream.flush();
}

std::ostream& StdLogConsumer::get_stream_(
        const Log::Entry& entry)
{
    switch (entry.kind)
    {
        case Log::Kind::Error:
        case Log::Kind::Warning:
            return std::cerr;

        default:
            return std::cout;
    }
}

} /* namespace utils */
} /* namespace eprosima */
