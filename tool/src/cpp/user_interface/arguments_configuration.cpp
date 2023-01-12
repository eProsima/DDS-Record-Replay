// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file arguments_configuration.cpp
 *
 */

#include <iostream>
#include <string>
#include <vector>

#include <ddsrecorder/library/config.h>
#include <cpp_utils/Log.hpp>
#include <cpp_utils/utils.hpp>

#include "arguments_configuration.hpp"

namespace eprosima {
namespace ddsrecorder {
namespace ui {

const option::Descriptor usage[] = {
    {
        optionIndex::UNKNOWN_OPT,
        0,
        "",
        "",
        Arg::None,
        "Usage: Fast DDS Router \n" \
        "Connect different DDS networks via DDS through LAN or WAN.\n" \
        "It will build a communication bridge between the different " \
        "Participants included in the provided configuration file.\n" \
        "To stop the execution gracefully use SIGINT (C^) or SIGTERM (kill) signals.\n" \
        "General options:"
    },

    ////////////////////
    // Help options
    {
        optionIndex::UNKNOWN_OPT, 0, "", "", Arg::None,
        "\nApplication help and information."
    },

    {
        optionIndex::HELP,
        0,
        "h",
        "help",
        Arg::None,
        "  -h \t--help\t  \t" \
        "Print this help message."
    },

    { 0, 0, 0, 0, 0, 0 }
};

option::ArgStatus Arg::Unknown(
        const option::Option& option,
        bool msg)
{
    if (msg)
    {
        logError(
            DDSRECORDER_ARGS,
            "Unknown option '" << option << "'. Use -h to see this executable possible arguments.");
    }
    return option::ARG_ILLEGAL;
}

option::ArgStatus Arg::Required(
        const option::Option& option,
        bool msg)
{
    if (option.arg != 0 && option.arg[0] != 0)
    {
        return option::ARG_OK;
    }

    if (msg)
    {
        logError(DDSRECORDER_ARGS, "Option '" << option << "' required.");
    }
    return option::ARG_ILLEGAL;
}

option::ArgStatus Arg::Numeric(
        const option::Option& option,
        bool msg)
{
    char* endptr = 0;
    if (option.arg != 0 && std::strtol(option.arg, &endptr, 10))
    {
    }
    if (endptr != option.arg && *endptr == 0)
    {
        return option::ARG_OK;
    }

    if (msg)
    {
        logError(DDSRECORDER_ARGS, "Option '" << option << "' requires a numeric argument.");
    }
    return option::ARG_ILLEGAL;
}

option::ArgStatus Arg::Float(
        const option::Option& option,
        bool msg)
{
    char* endptr = 0;
    if (option.arg != 0 && std::strtof(option.arg, &endptr))
    {
    }
    if (endptr != option.arg && *endptr == 0)
    {
        return option::ARG_OK;
    }

    if (msg)
    {
        logError(DDSRECORDER_ARGS, "Option '" << option << "' requires a float argument.");
    }
    return option::ARG_ILLEGAL;
}

option::ArgStatus Arg::String(
        const option::Option& option,
        bool msg)
{
    if (option.arg != 0)
    {
        return option::ARG_OK;
    }
    if (msg)
    {
        logError(DDSRECORDER_ARGS, "Option '" << option << "' requires a text argument.");
    }
    return option::ARG_ILLEGAL;
}

option::ArgStatus Arg::Readable_File(
        const option::Option& option,
        bool msg)
{
    if (option.arg != 0)
    {
        // Windows has not unistd library, so to check if file is readable use a _access method (definition on top)
        if (is_file_accessible(option.arg, utils::FileAccessMode::read))
        {
            return option::ARG_OK;
        }
    }
    if (msg)
    {
        logError(DDSRECORDER_ARGS, "Option '" << option << "' requires an existing readable file as argument.");
    }
    return option::ARG_ILLEGAL;
}

option::ArgStatus Arg::Log_Kind_Correct_Argument(
        const option::Option& option,
        bool msg)
{
    return Arg::Valid_Options(string_vector_LogKind(), option, msg);
}

option::ArgStatus Arg::Valid_Options(
        const std::vector<std::string>& valid_options,
        const option::Option& option,
        bool msg)
{
    if (nullptr == option.arg)
    {
        if (msg)
        {
            logError(DDSRECORDER_ARGS, "Option '" << option.name << "' requires a text argument.");
        }
        return option::ARG_ILLEGAL;
    }

    if (std::find(valid_options.begin(), valid_options.end(), std::string(option.arg)) != valid_options.end())
    {
        return option::ARG_OK;
    }
    else if (msg)
    {
        utils::Formatter error_msg;
        error_msg << "Option '" << option.name << "' requires a one of this values: {";
        for (const auto& valid_option : valid_options)
        {
            error_msg << "\"" << valid_option << "\";";
        }
        error_msg << "}.";

        logError(DDSRECORDER_ARGS, error_msg);
    }

    return option::ARG_ILLEGAL;
}

std::ostream& operator <<(
        std::ostream& output,
        const option::Option& option)
{
    output << std::string(option.name, option.name + option.namelen);
    return output;
}

} /* namespace ui */
} /* namespace ddsrecorder */
} /* namespace eprosima */
