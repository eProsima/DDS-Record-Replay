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
 * @file BaseLogConfiguration.cpp
 *
 */

#include <cpp_utils/logging/BaseLogConfiguration.hpp>
#include <cpp_utils/utils.hpp>

namespace eprosima {
namespace utils {


BaseLogConfiguration::BaseLogConfiguration()
    : verbosity{VerbosityKind::Warning, FuzzyLevelValues::fuzzy_level_default}
{
    filter[VerbosityKind::Info].set_value("", FuzzyLevelValues::fuzzy_level_default);
    filter[VerbosityKind::Warning].set_value("", FuzzyLevelValues::fuzzy_level_default);
    filter[VerbosityKind::Error].set_value("", FuzzyLevelValues::fuzzy_level_default);
}

bool BaseLogConfiguration::is_valid(
        Formatter& error_msg) const noexcept
{
    return true;
}

void BaseLogConfiguration::set(
        const utils::Fuzzy<VerbosityKind>& log_verbosity) noexcept
{
    if (verbosity.get_level() <= log_verbosity.get_level())
    {
        verbosity = log_verbosity;
    }
}

void BaseLogConfiguration::set(
        const LogFilter& log_filter)
{
    if (filter[VerbosityKind::Error].get_level() <= log_filter.at(VerbosityKind::Error).get_level())
    {
        filter[VerbosityKind::Error] = log_filter.at(VerbosityKind::Error);
    }

    if (filter[VerbosityKind::Warning].get_level() <= log_filter.at(VerbosityKind::Warning).get_level())
    {
        filter[VerbosityKind::Warning] = log_filter.at(VerbosityKind::Warning);
    }

    if (filter[VerbosityKind::Info].get_level() <= log_filter.at(VerbosityKind::Info).get_level())
    {
        filter[VerbosityKind::Info] = log_filter.at(VerbosityKind::Info);
    }
}

std::ostream& operator <<(
        std::ostream& os,
        const Fuzzy<VerbosityKind>& kind)
{
    os << "Fuzzy{Level(" << kind.get_level_as_str() << ") " << kind.get_reference() << "}";
    return os;
}

std::ostream& operator <<(
        std::ostream& os,
        const LogFilter& filter)
{
    os << "Log Filter: {Kind: Error, Regex: " << filter.at(VerbosityKind::Error).get_value() << "}; "
       << "{Kind: Warning, Regex: " << filter.at(VerbosityKind::Warning).get_value() << "}; "
       << "{Kind: Info, Regex: " << filter.at(VerbosityKind::Info).get_value() << "}";

    return os;
}

} /* namespace utils */
} /* namespace eprosima */
