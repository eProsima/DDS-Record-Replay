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

#include <exception>
#include <memory>

#include <cpp_utils/exception/ConfigurationException.hpp>
#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/logging/StdLogConsumer.hpp>
#include <cpp_utils/Log.hpp>

#include <ddsrecorder_yaml/replayer/YamlReaderConfiguration.hpp>

#include "tool/McapToSqlConverter.hpp"
#include "user_interface/arguments_configuration.hpp"

namespace {

void configure_logging_(
        const eprosima::ddsrecorder::yaml::ReplayerConfiguration& configuration)
{
    const auto log_configuration = configuration.ddspipe_configuration.log_configuration;

    eprosima::utils::Log::ClearConsumers();
    eprosima::utils::Log::SetVerbosity(log_configuration.verbosity);

    if (log_configuration.stdout_enable)
    {
        eprosima::utils::Log::RegisterConsumer(
            std::make_unique<eprosima::utils::StdLogConsumer>(&log_configuration));
    }
}

} // namespace

int main(
        int argc,
        char** argv)
{
    eprosima::ddsrecorder::converter::CommandlineArgsMcapConvert commandline_args;

    const auto arg_parse_result =
            eprosima::ddsrecorder::converter::parse_arguments(argc, argv, commandline_args);

    if (arg_parse_result == eprosima::ddsrecorder::converter::ProcessReturnCode::help_argument ||
            arg_parse_result == eprosima::ddsrecorder::converter::ProcessReturnCode::version_argument)
    {
        return static_cast<int>(eprosima::ddsrecorder::converter::ProcessReturnCode::success);
    }
    else if (arg_parse_result != eprosima::ddsrecorder::converter::ProcessReturnCode::success)
    {
        return static_cast<int>(arg_parse_result);
    }

    logUser(DDSREPLAYER_EXECUTION, "Starting MCAP Convert execution.");

    try
    {
        std::unique_ptr<eprosima::ddsrecorder::yaml::ReplayerConfiguration> configuration;

        if (commandline_args.file_path.empty())
        {
            configuration = std::make_unique<eprosima::ddsrecorder::yaml::ReplayerConfiguration>(
                eprosima::Yaml(), &commandline_args);
        }
        else
        {
            configuration = std::make_unique<eprosima::ddsrecorder::yaml::ReplayerConfiguration>(
                commandline_args.file_path, &commandline_args);
        }

        configure_logging_(*configuration);

        const auto output_file =
                eprosima::ddsrecorder::converter::McapToSqlConverter::resolve_output_file(
            commandline_args.input_file,
            commandline_args.sql_output);

        logUser(DDSREPLAYER_EXECUTION, "MCAP Convert running in MCAP-to-SQL conversion mode.");

        eprosima::ddsrecorder::converter::McapToSqlConverter converter(
            *configuration,
            commandline_args.input_file,
            output_file);
        converter.convert();

        logUser(DDSREPLAYER_EXECUTION, "MCAP-to-SQL conversion finished correctly.");
        logUser(DDSREPLAYER_EXECUTION, "Finishing MCAP Convert execution correctly.");

        eprosima::utils::Log::Flush();
        eprosima::utils::Log::ClearConsumers();

        return static_cast<int>(eprosima::ddsrecorder::converter::ProcessReturnCode::success);
    }
    catch (const eprosima::utils::ConfigurationException& e)
    {
        EPROSIMA_LOG_ERROR(DDSREPLAYER_ERROR,
                "Error initializing MCAP Convert. Error message:\n " << e.what());
        return static_cast<int>(eprosima::ddsrecorder::converter::ProcessReturnCode::execution_failed);
    }
    catch (const eprosima::utils::InitializationException& e)
    {
        EPROSIMA_LOG_ERROR(DDSREPLAYER_ERROR,
                "Error initializing MCAP Convert data. Error message:\n " << e.what());
        return static_cast<int>(eprosima::ddsrecorder::converter::ProcessReturnCode::execution_failed);
    }
    catch (const eprosima::utils::InconsistencyException& e)
    {
        EPROSIMA_LOG_ERROR(DDSREPLAYER_ERROR,
                "Error processing MCAP Convert data. Error message:\n " << e.what());
        return static_cast<int>(eprosima::ddsrecorder::converter::ProcessReturnCode::execution_failed);
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_ERROR(DDSREPLAYER_ERROR,
                "Unexpected error running MCAP Convert. Error message:\n " << e.what());
        return static_cast<int>(eprosima::ddsrecorder::converter::ProcessReturnCode::execution_failed);
    }
}
