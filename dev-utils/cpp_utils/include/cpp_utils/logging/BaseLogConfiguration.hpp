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
// limitations under the License\.

#pragma once

#include <cpp_utils/library/library_dll.h>
#include <cpp_utils/Log.hpp>
#include <cpp_utils/types/Fuzzy.hpp>

#include <map>
#include <ostream>
#include <string>

namespace eprosima {
namespace utils {

using VerbosityKind = Log::Kind;
using LogFilter = std::map<VerbosityKind, Fuzzy<std::string>>;

/**
 * The collection of settings related to Logging.
 *
 * The Logging settings are:
 *  - Verbosity
 *  - Filter
 */
struct BaseLogConfiguration
{
    /////////////////////////
    // CONSTRUCTORS
    /////////////////////////

    //! Default BaseLogConfiguration constructor
    CPP_UTILS_DllAPI
    BaseLogConfiguration();

    /////////////////////////
    // METHODS
    /////////////////////////

    /**
     * @brief \c is_valid method.
     */
    CPP_UTILS_DllAPI
    virtual bool is_valid(
            Formatter& error_msg) const noexcept;

    /**
     * @brief Replace verbosity with a given log_verbosity if verbosity has a lower fuzzy level.
     */
    CPP_UTILS_DllAPI
    void set(
            const utils::Fuzzy<VerbosityKind>& log_verbosity) noexcept;

    /**
     * @brief Replace filter with a given log_filter if filter has a lower fuzzy level.
     */
    CPP_UTILS_DllAPI
    void set(
            const LogFilter& log_filter);

    /////////////////////////
    // VARIABLES
    /////////////////////////

    //! Verbosity kind
    Fuzzy<VerbosityKind> verbosity;

    //! Log Filter
    LogFilter filter;
};

/**
 * @brief \c VerbosityKind to stream Fuzzy level
 */
CPP_UTILS_DllAPI
std::ostream& operator <<(
        std::ostream& os,
        const Fuzzy<VerbosityKind>& kind);

/**
 * @brief \c LogFilter to stream serialization
 */
CPP_UTILS_DllAPI
std::ostream& operator <<(
        std::ostream& os,
        const LogFilter& filter);

} /* namespace utils */
} /* namespace eprosima */
