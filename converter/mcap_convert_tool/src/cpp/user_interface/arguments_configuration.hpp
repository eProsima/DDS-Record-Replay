// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <iosfwd>
#include <string>
#include <vector>

#include <optionparser.h>

#include <cpp_utils/macros/custom_enumeration.hpp>

#include "CommandlineArgsMcapConvert.hpp"
#include "ProcessReturnCode.hpp"

namespace eprosima {
namespace ddsrecorder {
namespace converter {

struct Arg : public option::Arg
{
    static void print_error(
            const char* msg1,
            const option::Option& opt,
            const char* msg2);

    static option::ArgStatus Unknown(
            const option::Option& option,
            bool msg);

    static option::ArgStatus Required(
            const option::Option& option,
            bool msg);

    static option::ArgStatus String(
            const option::Option& option,
            bool msg);

    static option::ArgStatus Readable_File(
            const option::Option& option,
            bool msg);

    static option::ArgStatus Log_Kind_Correct_Argument(
            const option::Option& option,
            bool msg);

    static option::ArgStatus Valid_Options(
            const std::vector<std::string>& valid_options,
            const option::Option& option,
            bool msg);
};

enum optionIndex
{
    UNKNOWN_OPT,
    HELP,
    VERSION,
    INPUT_FILE,
    CONFIGURATION_FILE,
    SQL_OUTPUT,
    ACTIVATE_DEBUG,
    LOG_FILTER,
    LOG_VERBOSITY,
};

extern const option::Descriptor usage[];

ProcessReturnCode parse_arguments(
        int argc,
        char** argv,
        CommandlineArgsMcapConvert& commandline_args);

std::ostream& operator <<(
        std::ostream& output,
        const option::Option& option);

void print_version();

ENUMERATION_BUILDER(
    LogKind,
    error,
    warning,
    info
    );

} /* namespace converter */
} /* namespace ddsrecorder */
} /* namespace eprosima */
