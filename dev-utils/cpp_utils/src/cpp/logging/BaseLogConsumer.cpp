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
 * @file BaseLogConsumer.cpp
 *
 */

#include <regex>

#include <cpp_utils/logging/BaseLogConsumer.hpp>

namespace eprosima {
namespace utils {

BaseLogConsumer::BaseLogConsumer(
        const BaseLogConfiguration* log_configuration)
    : filter_(log_configuration->filter)
    , verbosity_(log_configuration->verbosity)
{
}

bool BaseLogConsumer::accept_entry_(
        const Log::Entry& entry)
{
    // Filter by regex
    std::regex filter_regex(filter_[entry.kind].get_value());
    const bool is_category_valid = std::regex_search(entry.context.category, filter_regex);
    const bool is_message_valid = std::regex_search(entry.message, filter_regex);
    const bool is_content_valid = is_category_valid || is_message_valid;

    // Filter by kind
    const bool is_kind_valid = entry.kind <= verbosity_;

    return is_kind_valid && is_content_valid;
}

} /* namespace utils */
} /* namespace eprosima */
