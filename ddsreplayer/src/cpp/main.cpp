// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file main.cpp
 *
 */

#include <thread>

#include <cpp_utils/event/FileWatcherHandler.hpp>
#include <cpp_utils/event/MultipleEventHandler.hpp>
#include <cpp_utils/event/PeriodicEventHandler.hpp>
#include <cpp_utils/event/SignalEventHandler.hpp>
#include <cpp_utils/exception/ConfigurationException.hpp>
#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/logging/BaseLogConfiguration.hpp>
#include <cpp_utils/logging/StdLogConsumer.hpp>
#include <cpp_utils/ReturnCode.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/types/Fuzzy.hpp>
#include <cpp_utils/utils.hpp>

#include <ddspipe_core/logging/DdsLogConsumer.hpp>

#include <ddsrecorder_yaml/replayer/CommandlineArgsReplayer.hpp>
#include <ddsrecorder_yaml/replayer/YamlReaderConfiguration.hpp>

#include "user_interface/arguments_configuration.hpp"
#include "user_interface/constants.hpp"
#include "user_interface/ProcessReturnCode.hpp"

#include "tool/DdsReplayer.hpp"

using namespace eprosima::ddspipe;
using namespace eprosima::ddsrecorder::replayer;

std::unique_ptr<eprosima::utils::event::FileWatcherHandler> create_filewatcher(
        const std::unique_ptr<DdsReplayer>& replayer,
        const std::string& file_path)
{
    // Callback will reload configuration and pass it to DdsPipe
    // WARNING: it is needed to pass file_path, as FileWatcher only retrieves file_name
    std::function<void(std::string)> filewatcher_callback =
            [&replayer, &file_path]
                (std::string file_name)
            {
                logUser(
                    DDSREPLAYER_EXECUTION,
                    "FileWatcher notified changes in file " << file_name << ". Reloading configuration");

                try
                {
                    eprosima::ddsrecorder::yaml::ReplayerConfiguration new_configuration(file_path);
                    replayer->reload_configuration(new_configuration);
                }
                catch (const std::exception& e)
                {
                    logWarning(DDSREPLAYER_EXECUTION,
                            "Error reloading configuration file " << file_name << " with error: " <<
                            e.what());
                }
            };

    // Creating FileWatcher event handler
    return std::make_unique<eprosima::utils::event::FileWatcherHandler>(filewatcher_callback, file_path);
}

std::unique_ptr<eprosima::utils::event::PeriodicEventHandler> create_periodic_handler(
        const std::unique_ptr<DdsReplayer>& replayer,
        const std::string& file_path,
        const eprosima::utils::Duration_ms& reload_time)
{
    // Callback will reload configuration and pass it to DdsPipe
    std::function<void()> periodic_callback =
            [&replayer, &file_path]
                ()
            {
                logUser(
                    DDSREPLAYER_EXECUTION,
                    "Periodic Timer raised. Reloading configuration from file " << file_path << ".");

                try
                {
                    eprosima::ddsrecorder::yaml::ReplayerConfiguration new_configuration(file_path);
                    replayer->reload_configuration(new_configuration);
                }
                catch (const std::exception& e)
                {
                    logWarning(DDSREPLAYER_EXECUTION,
                            "Error reloading configuration file " << file_path << " with error: " <<
                            e.what());
                }
            };

    // Creating periodic handler
    return std::make_unique<eprosima::utils::event::PeriodicEventHandler>(periodic_callback, reload_time);
}

int main(
        int argc,
        char** argv)
{
    // Initialize CommandlineArgsReplayer
    eprosima::ddsrecorder::yaml::CommandlineArgsReplayer commandline_args;

    // Parse arguments
    ProcessReturnCode arg_parse_result =
            parse_arguments(argc, argv, commandline_args);

    if (arg_parse_result == ProcessReturnCode::help_argument)
    {
        return static_cast<int>(ProcessReturnCode::success);
    }
    else if (arg_parse_result == ProcessReturnCode::version_argument)
    {
        return static_cast<int>(ProcessReturnCode::success);
    }
    else if (arg_parse_result != ProcessReturnCode::success)
    {
        return static_cast<int>(arg_parse_result);
    }

    // Check file is in args, else get the default file
    if (commandline_args.file_path == "")
    {
        if (is_file_accessible(DEFAULT_CONFIGURATION_FILE_NAME, eprosima::utils::FileAccessMode::read))
        {
            commandline_args.file_path = DEFAULT_CONFIGURATION_FILE_NAME;

            logUser(
                DDSREPLAYER_EXECUTION,
                "Not configuration file given, using default file " << commandline_args.file_path << ".");
        }
    }
    else
    {
        // Check file exists and it is readable
        // NOTE: this check is redundant with option parse arg check
        if (!is_file_accessible(commandline_args.file_path.c_str(), eprosima::utils::FileAccessMode::read))
        {
            logError(
                DDSREPLAYER_ARGS,
                "File '" << commandline_args.file_path << "' does not exist or it is not accessible.");
            return static_cast<int>(ProcessReturnCode::required_argument_failed);
        }
    }

    logUser(DDSREPLAYER_EXECUTION, "Starting DDS Replayer execution.");

    // Encapsulating execution in block to erase all memory correctly before closing process
    try
    {
        // Create a multiple event handler that handles all events that make the replayer stop
        auto close_handler = std::make_shared<eprosima::utils::event::MultipleEventHandler>();

        // First of all, create signal handler so SIGINT and SIGTERM do not break the program while initializing
        close_handler->register_event_handler<eprosima::utils::event::EventHandler<eprosima::utils::event::Signal>,
                eprosima::utils::event::Signal>(
            std::make_unique<eprosima::utils::event::SignalEventHandler<eprosima::utils::event::Signal::sigint>>());     // Add SIGINT
        close_handler->register_event_handler<eprosima::utils::event::EventHandler<eprosima::utils::event::Signal>,
                eprosima::utils::event::Signal>(
            std::make_unique<eprosima::utils::event::SignalEventHandler<eprosima::utils::event::Signal::sigterm>>());    // Add SIGTERM

        /////
        // DDS Replayer Initialization

        // Load configuration from YAML
        eprosima::ddsrecorder::yaml::ReplayerConfiguration configuration(commandline_args.file_path, &commandline_args);

        /////
        // Logging
        {
            const auto log_configuration = configuration.ddspipe_configuration.log_configuration;

            eprosima::utils::Log::ClearConsumers();
            eprosima::utils::Log::SetVerbosity(log_configuration.verbosity);

            // Stdout Log Consumer
            if (log_configuration.stdout_enable)
            {
                eprosima::utils::Log::RegisterConsumer(
                    std::make_unique<eprosima::utils::StdLogConsumer>(&log_configuration));
            }

            // DDS Log Consumer
            if (log_configuration.publish.enable)
            {
                eprosima::utils::Log::RegisterConsumer(
                    std::make_unique<eprosima::ddspipe::core::DdsLogConsumer>(&log_configuration));
            }
        }


        // Use MCAP input from YAML configuration file if not provided via executable arg
        if (commandline_args.input_file == "")
        {
            if (configuration.input_file != "")
            {
                commandline_args.input_file = configuration.input_file;
                // Check file exists and it is readable
                if (!is_file_accessible(commandline_args.input_file.c_str(), eprosima::utils::FileAccessMode::read))
                {
                    logError(
                        DDSREPLAYER_ARGS,
                        "File '" << commandline_args.input_file << "' does not exist or it is not accessible.");
                    return static_cast<int>(ProcessReturnCode::required_argument_failed);
                }
            }
            else
            {
                logError(
                    DDSREPLAYER_ARGS,
                    "An input MCAP file must be provided through argument '-i' / '--input-file' " <<
                        "or under 'input-file' YAML tag.");
                return static_cast<int>(ProcessReturnCode::required_argument_failed);
            }
        }
        else
        {
            // Do nothing (readable file verification already done when parsing YAML file)
        }

        logUser(DDSREPLAYER_EXECUTION, "DDS Replayer running.");


        // Create replayer instance
        auto replayer = std::make_unique<DdsReplayer>(configuration, commandline_args.input_file, commandline_args.domain);

        // Create File Watcher Handler
        std::unique_ptr<eprosima::utils::event::FileWatcherHandler> file_watcher_handler;
        if (commandline_args.file_path != "")
        {
            file_watcher_handler = create_filewatcher(replayer, commandline_args.file_path);
        }

        // Create Periodic Handler
        std::unique_ptr<eprosima::utils::event::PeriodicEventHandler> periodic_handler;
        if (commandline_args.reload_time > 0 && commandline_args.file_path != "")
        {
            periodic_handler = create_periodic_handler(replayer, commandline_args.file_path,
                            commandline_args.reload_time);
        }

        // Start replaying data
        bool read_success;
        std::thread process_mcap_thread([&]
                {
                    try
                    {
                        replayer->process_mcap();
                        read_success = true;
                    }
                    catch (const eprosima::utils::InconsistencyException& e)
                    {
                        logError(DDSREPLAYER_ERROR,
                        "Error processing MCAP file. Error message:\n " <<
                            e.what());
                        read_success = false;
                    }
                    close_handler->simulate_event_occurred();
                });

        // Wait until signal arrives (or all messages in MCAP file sent)
        close_handler->wait_for_event();

        // Disable inner pipe, which would abort replaying messages in case execution stopped by signal
        replayer->stop();

        process_mcap_thread.join();

        if (!read_success)
        {
            // An exception was captured in the MCAP reading thread
            return static_cast<int>(ProcessReturnCode::execution_failed);
        }

        logUser(DDSREPLAYER_EXECUTION, "Stopping DDS Replayer.");

        logUser(DDSREPLAYER_EXECUTION, "DDS Replayer stopped correctly.");
    }
    catch (const eprosima::utils::ConfigurationException& e)
    {
        logError(DDSREPLAYER_ERROR,
                "Error Loading DDS Replayer Configuration from file " << commandline_args.file_path <<
                ". Error message:\n " <<
                e.what());
        return static_cast<int>(ProcessReturnCode::execution_failed);
    }
    catch (const eprosima::utils::InitializationException& e)
    {
        logError(DDSREPLAYER_ERROR,
                "Error Initializing DDS Replayer. Error message:\n " <<
                e.what());
        return static_cast<int>(ProcessReturnCode::execution_failed);
    }

    logUser(DDSREPLAYER_EXECUTION, "Finishing DDS Replayer execution correctly.");

    // Force print every log before closing
    eprosima::utils::Log::Flush();

    // Delete the consumers before closing
    eprosima::utils::Log::ClearConsumers();

    return static_cast<int>(ProcessReturnCode::success);
}
