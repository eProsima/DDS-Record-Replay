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

#include "arguments_configuration.hpp"

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include <cpp_utils/Log.hpp>
#include <cpp_utils/utils.hpp>

#include <ddsrecorder_participants/library/config.h>

namespace eprosima {
namespace ddsrecorder {
namespace converter {

const option::Descriptor usage[] = {
    {
        optionIndex::UNKNOWN_OPT,
        0,
        "",
        "",
        Arg::None,
        "Usage: MCAP Convert \n" \
        "Convert an MCAP recording into the DDS Record & Replay SQLite schema.\n" \
        "General options:"
    },

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

    {
        optionIndex::VERSION,
        0,
        "v",
        "version",
        Arg::None,
        "  -v \t--version\t  \t" \
        "Print version, branch and commit hash."
    },

    {
        optionIndex::UNKNOWN_OPT, 0, "", "", Arg::None,
        "\nApplication parameters"
    },

    {
        optionIndex::INPUT_FILE,
        0,
        "i",
        "input-file",
        Arg::Readable_File,
        "  -i \t--input-file\t  \t" \
        "Path to the input MCAP file."
    },

    {
        optionIndex::CONFIGURATION_FILE,
        0,
        "c",
        "config-path",
        Arg::Readable_File,
        "  -c \t--config-path\t  \t" \
        "Path to the optional configuration file (yaml format)."
    },

    {
        optionIndex::SQL_OUTPUT,
        0,
        "",
        "sql-output",
        Arg::String,
        "  \t--sql-output\t  \t" \
        "Output SQLite file path. [Default: <input_file_stem>.db]."
    },

    {
        optionIndex::UNKNOWN_OPT, 0, "", "", Arg::None,
        "\nDebug parameters"
    },

    {
        optionIndex::ACTIVATE_DEBUG,
        0,
        "d",
        "debug",
        Arg::None,
        "  -d \t--debug\t  \t" \
        "Set log verbosity to Info \t" \
        "(Using this option with --log-filter and/or --log-verbosity will head to undefined behaviour)."
    },

    {
        optionIndex::LOG_FILTER,
        0,
        "",
        "log-filter",
        Arg::String,
        "  \t--log-filter\t  \t" \
        "Set a Regex Filter to filter by category the info and warning log entries. " \
        "[Default = \"DDSREPLAYER\"]. "
    },

    {
        optionIndex::LOG_VERBOSITY,
        0,
        "",
        "log-verbosity",
        Arg::Log_Kind_Correct_Argument,
        "  \t--log-verbosity\t  \t" \
        "Set a Log Verbosity Level higher or equal the one given. " \
        "(Values accepted: \"info\",\"warning\",\"error\" no Case Sensitive) " \
        "[Default = \"warning\"]. "
    },

    {
        optionIndex::UNKNOWN_OPT, 0, "", "", Arg::None,
        "\n"
    },

    { 0, 0, 0, 0, 0, 0 }
};

void print_version()
{
    std::cout
        << "DDS Record & Replay "
        << DDSRECORDER_PARTICIPANTS_VERSION_STRING
        << "\ncommit hash: "
        << DDSRECORDER_PARTICIPANTS_COMMIT_HASH
        << std::endl;
}

ProcessReturnCode parse_arguments(
        int argc,
        char** argv,
        CommandlineArgsMcapConvert& commandline_args)
{
    int columns;
#if defined(_WIN32)
    char* buf = nullptr;
    size_t sz = 0;
    if (_dupenv_s(&buf, &sz, "COLUMNS") == 0 && buf != nullptr)
    {
        columns = std::strtol(buf, nullptr, 10);
        free(buf);
    }
    else
    {
        columns = 80;
    }
#else
    columns = getenv("COLUMNS") ? atoi(getenv("COLUMNS")) : 180;
#endif // if defined(_WIN32)

    if (argc > 0)
    {
        argc -= (argc > 0);
        argv += (argc > 0);

        option::Stats stats(usage, argc, argv);
        std::vector<option::Option> options(stats.options_max);
        std::vector<option::Option> buffer(stats.buffer_max);
        option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);

        if (parse.error())
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return ProcessReturnCode::incorrect_argument;
        }

        if (parse.nonOptionsCount())
        {
            EPROSIMA_LOG_ERROR(DDSREPLAYER_ARGS, "ERROR: Unknown argument: <" << parse.nonOption(0) << ">.");
            option::printUsage(fwrite, stdout, usage, columns);
            return ProcessReturnCode::incorrect_argument;
        }

        if (options[optionIndex::HELP])
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return ProcessReturnCode::help_argument;
        }

        if (options[optionIndex::VERSION])
        {
            print_version();
            return ProcessReturnCode::version_argument;
        }

        for (int i = 0; i < parse.optionsCount(); ++i)
        {
            option::Option& opt = buffer[i];
            switch (opt.index())
            {
                case optionIndex::INPUT_FILE:
                    commandline_args.input_file = opt.arg;
                    break;

                case optionIndex::CONFIGURATION_FILE:
                    commandline_args.file_path = opt.arg;
                    break;

                case optionIndex::SQL_OUTPUT:
                    commandline_args.sql_output = opt.arg;
                    break;

                case optionIndex::ACTIVATE_DEBUG:
                    commandline_args.log_filter[utils::VerbosityKind::Error].set_value("");
                    commandline_args.log_filter[utils::VerbosityKind::Warning].set_value("DDSREPLAYER");
                    commandline_args.log_filter[utils::VerbosityKind::Info].set_value("DDSREPLAYER");
                    commandline_args.log_verbosity = utils::VerbosityKind::Info;
                    break;

                case optionIndex::LOG_FILTER:
                    commandline_args.log_filter[utils::VerbosityKind::Error].set_value(opt.arg);
                    commandline_args.log_filter[utils::VerbosityKind::Warning].set_value(opt.arg);
                    commandline_args.log_filter[utils::VerbosityKind::Info].set_value(opt.arg);
                    break;

                case optionIndex::LOG_VERBOSITY:
                    commandline_args.log_verbosity =
                            utils::VerbosityKind(static_cast<int>(from_string_LogKind(opt.arg)));
                    break;

                case optionIndex::UNKNOWN_OPT:
                    EPROSIMA_LOG_ERROR(DDSREPLAYER_ARGS, opt << " is not a valid argument.");
                    option::printUsage(fwrite, stdout, usage, columns);
                    return ProcessReturnCode::incorrect_argument;

                default:
                    break;
            }
        }
    }
    else
    {
        option::printUsage(fwrite, stdout, usage, columns);
        return ProcessReturnCode::incorrect_argument;
    }

    utils::Formatter error_msg;
    if (!commandline_args.is_valid(error_msg))
    {
        EPROSIMA_LOG_ERROR(DDSREPLAYER_ARGS, error_msg);
        option::printUsage(fwrite, stdout, usage, columns);
        return ProcessReturnCode::incorrect_argument;
    }

    return ProcessReturnCode::success;
}

option::ArgStatus Arg::Unknown(
        const option::Option& option,
        bool msg)
{
    if (msg)
    {
        EPROSIMA_LOG_ERROR(
            DDSREPLAYER_ARGS,
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
        EPROSIMA_LOG_ERROR(DDSREPLAYER_ARGS, "Option '" << option << "' required.");
    }
    return option::ARG_ILLEGAL;
}

option::ArgStatus Arg::String(
        const option::Option& option,
        bool msg)
{
    if (option.arg != 0 && option.arg[0] != 0)
    {
        return option::ARG_OK;
    }

    if (msg)
    {
        EPROSIMA_LOG_ERROR(DDSREPLAYER_ARGS, "Option '" << option << "' requires a text argument.");
    }
    return option::ARG_ILLEGAL;
}

option::ArgStatus Arg::Readable_File(
        const option::Option& option,
        bool msg)
{
    if (option.arg != 0 && is_file_accessible(option.arg, eprosima::utils::FileAccessMode::read))
    {
        return option::ARG_OK;
    }

    if (msg)
    {
        EPROSIMA_LOG_ERROR(DDSREPLAYER_ARGS,
                "Option '" << option << "' requires a readable file as argument.");
    }
    return option::ARG_ILLEGAL;
}

option::ArgStatus Arg::Valid_Options(
        const std::vector<std::string>& valid_options,
        const option::Option& option,
        bool msg)
{
    if (option.arg == nullptr || option.arg[0] == 0)
    {
        if (msg)
        {
            EPROSIMA_LOG_ERROR(DDSREPLAYER_ARGS, "Option '" << option.name << "' requires a text argument.");
        }
        return option::ARG_ILLEGAL;
    }

    const std::string arg(option.arg);

    if (std::find(valid_options.begin(), valid_options.end(), arg) != valid_options.end())
    {
        return option::ARG_OK;
    }

    if (msg)
    {
        utils::Formatter error_msg;
        error_msg << "Option '" << option.name << "' requires one of the following values: ";
        for (const auto& valid_option : valid_options)
        {
            error_msg << "'" << valid_option << "' ";
        }
        EPROSIMA_LOG_ERROR(DDSREPLAYER_ARGS, error_msg);
    }

    return option::ARG_ILLEGAL;
}

option::ArgStatus Arg::Log_Kind_Correct_Argument(
        const option::Option& option,
        bool msg)
{
    static const std::vector<std::string> VALID_OPTIONS = {
        "error",
        "warning",
        "info"
    };

    return Valid_Options(VALID_OPTIONS, option, msg);
}

std::ostream& operator <<(
        std::ostream& output,
        const option::Option& option)
{
    output << option.name;
    return output;
}

void Arg::print_error(
        const char* msg1,
        const option::Option& opt,
        const char* msg2)
{
    std::cerr << msg1 << opt.name << msg2;
}

} /* namespace converter */
} /* namespace ddsrecorder */
} /* namespace eprosima */
