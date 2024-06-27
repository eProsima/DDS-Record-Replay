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
 
 */

#include <nlohmann/json.hpp>

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

#include <ddsrecorder_participants/recorder/logging/DdsRecorderLogConsumer.hpp>
#include <ddsrecorder_yaml/recorder/CommandlineArgsRecorder.hpp>
#include <ddsrecorder_yaml/recorder/YamlReaderConfiguration.hpp>

#include "user_interface/arguments_configuration.hpp"
#include "user_interface/constants.hpp"
#include "user_interface/ProcessReturnCode.hpp"

#include "command_receiver/CommandReceiver.hpp"
#include "tool/DdsRecorder.hpp"

using namespace eprosima::ddspipe;
using namespace eprosima::ddsrecorder::recorder;

using CommandCode = eprosima::ddsrecorder::recorder::receiver::CommandCode;
using DdsRecorderState = eprosima::ddsrecorder::recorder::DdsRecorderStateCode;
using json = nlohmann::json;

const std::string NEXT_STATE_TAG = "next_state";
constexpr auto string_to_command = eprosima::ddsrecorder::recorder::receiver::string_to_enumeration;
// constexpr auto string_to_state = eprosima::ddsrecorder::recorder::string_to_enumeration;  // TODO: fix compilation error

std::unique_ptr<eprosima::utils::event::FileWatcherHandler> create_filewatcher(
        const std::unique_ptr<DdsRecorder>& recorder,
        const std::string& file_path)
{
    // Callback will reload configuration and pass it to DdsPipe
    // WARNING: it is needed to pass file_path, as FileWatcher only retrieves file_name
    std::function<void(std::string)> filewatcher_callback =
            [&recorder, &file_path]
                (std::string file_name)
            {
                logUser(
                    DDSRECORDER_EXECUTION,
                    "FileWatcher notified changes in file " << file_name << ". Reloading configuration");

                try
                {
                    eprosima::ddsrecorder::yaml::RecorderConfiguration new_configuration(file_path);
                    recorder->reload_configuration(new_configuration);
                }
                catch (const std::exception& e)
                {
                    logWarning(DDSRECORDER_EXECUTION,
                            "Error reloading configuration file " << file_name << " with error: " <<
                            e.what());
                }
            };

    // Creating FileWatcher event handler
    return std::make_unique<eprosima::utils::event::FileWatcherHandler>(filewatcher_callback, file_path);
}

std::unique_ptr<eprosima::utils::event::PeriodicEventHandler> create_periodic_handler(
        const std::unique_ptr<DdsRecorder>& recorder,
        const std::string& file_path,
        const eprosima::utils::Duration_ms& reload_time)
{
    // Callback will reload configuration and pass it to DdsPipe
    std::function<void()> periodic_callback =
            [&recorder, &file_path]
                ()
            {
                logUser(
                    DDSRECORDER_EXECUTION,
                    "Periodic Timer raised. Reloading configuration from file " << file_path << ".");

                try
                {
                    eprosima::ddsrecorder::yaml::RecorderConfiguration new_configuration(file_path);
                    recorder->reload_configuration(new_configuration);
                }
                catch (const std::exception& e)
                {
                    logWarning(DDSRECORDER_EXECUTION,
                            "Error reloading configuration file " << file_path << " with error: " <<
                            e.what());
                }
            };

    // Creating periodic handler
    return std::make_unique<eprosima::utils::event::PeriodicEventHandler>(periodic_callback, reload_time);
}

void parse_command(
        const DdsRecorderCommand& command,
        CommandCode& command_code,
        json& args)
{
    command_code = CommandCode::unknown;
    args = {};

    std::string command_str = command.command();
    // Case insensitive
    eprosima::utils::to_lowercase(command_str);
    std::string args_str = command.args();

    bool found = string_to_command(command_str, command_code);
    if (!found)
    {
        logWarning(DDSRECORDER_EXECUTION,
                "Command " << command_str <<
                " is not a valid command (only start/pause/suspend/stop/close).");
    }

    if (args_str != "")
    {
        try
        {
            args = json::parse(args_str);
        }
        catch (const std::exception& e)
        {
            logWarning(
                DDSRECORDER_EXECUTION,
                "Received command argument <" << args_str << "> is not a valid json object : <" << e.what() << ">.");
        }
    }
}

CommandCode state_to_command(
        const DdsRecorderState& state)
{
    switch (state)
    {
        case DdsRecorderState::RUNNING:
            return CommandCode::start;

        case DdsRecorderState::PAUSED:
            return CommandCode::pause;

        case DdsRecorderState::SUSPENDED:
            return CommandCode::suspend;

        case DdsRecorderState::STOPPED:
            return CommandCode::stop;

        default:
            // Unreachable
            eprosima::utils::tsnh(
                eprosima::utils::Formatter() << "Trying to convert to command an invalid state.");
            return CommandCode::stop;
    }
}

int main(
        int argc,
        char** argv)
{
    // Initialize CommandlineArgs
    eprosima::ddsrecorder::yaml::CommandlineArgsRecorder commandline_args;

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
                DDSRECORDER_EXECUTION,
                "No configuration file given, using default file " << commandline_args.file_path << ".");
        }
    }
    else
    {
        // Check file exists and it is readable
        // NOTE: this check is redundant with option parse arg check
        if (!is_file_accessible(commandline_args.file_path.c_str(), eprosima::utils::FileAccessMode::read))
        {
            logError(
                DDSRECORDER_ARGS,
                "File '" << commandline_args.file_path << "' does not exist or it is not accessible.");
            return static_cast<int>(ProcessReturnCode::required_argument_failed);
        }
    }

    logUser(DDSRECORDER_EXECUTION, "Starting DDS Recorder execution.");

    // Encapsulating execution in block to erase all memory correctly before closing process
    try
    {
        // Create a multiple event handler that handles all events that make the recorder stop
        auto close_handler = std::make_shared<eprosima::utils::event::MultipleEventHandler>();

        // First of all, create signal handler so SIGINT and SIGTERM do not break the program while initializing
        close_handler->register_event_handler<eprosima::utils::event::EventHandler<eprosima::utils::event::Signal>,
                eprosima::utils::event::Signal>(
            std::make_unique<eprosima::utils::event::SignalEventHandler<eprosima::utils::event::Signal::sigint>>());     // Add SIGINT
        close_handler->register_event_handler<eprosima::utils::event::EventHandler<eprosima::utils::event::Signal>,
                eprosima::utils::event::Signal>(
            std::make_unique<eprosima::utils::event::SignalEventHandler<eprosima::utils::event::Signal::sigterm>>());    // Add SIGTERM

        // If it must be a maximum time, register a periodic handler to finish handlers
        if (commandline_args.timeout > 0)
        {
            close_handler->register_event_handler<eprosima::utils::event::PeriodicEventHandler>(
                std::make_unique<eprosima::utils::event::PeriodicEventHandler>(
                    []()
                    {
                        /* Do nothing */ },
                    commandline_args.timeout));
        }

        /////
        // DDS Recorder Initialization

        // Load configuration from YAML
        eprosima::ddsrecorder::yaml::RecorderConfiguration configuration(commandline_args.file_path, &commandline_args);

        /////
        // Logging
        {
            const auto log_configuration = configuration.ddspipe_configuration.log_configuration;

            eprosima::utils::Log::ClearConsumers();
            eprosima::utils::Log::SetVerbosity(log_configuration.verbosity);

            // Std Log Consumer
            if (log_configuration.stdout_enable)
            {
                eprosima::utils::Log::RegisterConsumer(
                    std::make_unique<eprosima::utils::StdLogConsumer>(&log_configuration));
            }

            // DDS Recorder Log Consumer
            if (log_configuration.publish.enable)
            {
                eprosima::utils::Log::RegisterConsumer(
                    std::make_unique<eprosima::ddsrecorder::participants::DdsRecorderLogConsumer>(&log_configuration));
            }
        }

        // Verify that the configuration is correct
        eprosima::utils::Formatter error_msg;
        if (!configuration.is_valid(error_msg))
        {
            throw eprosima::utils::ConfigurationException(
                      eprosima::utils::Formatter() <<
                          "Invalid configuration: " << error_msg);
        }

        logUser(DDSRECORDER_EXECUTION, "DDS Recorder running.");

        if (configuration.enable_remote_controller)
        {
            logUser(DDSRECORDER_EXECUTION, "Waiting for instructions...");
            receiver::CommandReceiver receiver(configuration.controller_domain,
                    configuration.command_topic_name,
                    configuration.status_topic_name, close_handler, configuration.simple_configuration);
            receiver.init();

            CommandCode prev_command;
            CommandCode command;
            json args;

            // Parse and convert initial state to initial command
            DdsRecorderState initial_state;
            bool found = string_to_enumeration(configuration.initial_state,
                            initial_state);
            if (!found)
            {
                logWarning(DDSRECORDER_EXECUTION,
                        "Initial state " << configuration.initial_state <<
                        " is not a valid one (only RUNNING/PAUSED/SUSPENDED/STOPPED). Using instead default RUNNING initial state...");
                initial_state = DdsRecorderState::RUNNING;
            }
            command = state_to_command(initial_state);

            prev_command = CommandCode::close;
            do
            {
                // Skip waiting for commmand if initial_state is RUNNING/PAUSED/SUSPENDED (only applies to first iteration)
                if (command == CommandCode::stop)
                {
                    //////////////////////////
                    //// STATE -> STOPPED ////
                    //////////////////////////

                    // Publish state if previous -> CLOSED/RUNNING/PAUSED/SUSPENDED
                    if (prev_command != CommandCode::stop)
                    {
                        receiver.publish_status(CommandCode::stop, prev_command);
                    }

                    prev_command = CommandCode::stop;
                    parse_command(receiver.wait_for_command(), command, args);
                    switch (command)
                    {
                        case CommandCode::start:
                        case CommandCode::pause:
                        case CommandCode::suspend:
                            // Exit STOPPED state -> proceed
                            break;

                        case CommandCode::event:
                        case CommandCode::stop:
                            logWarning(DDSRECORDER_EXECUTION,
                                    "Ignoring " << command << " command, recorder not active yet.");
                            command = CommandCode::stop;  // Stay in STOPPED state
                            continue;

                        case CommandCode::close:
                            // close command or signal received -> exit
                            continue;

                        default:
                        case CommandCode::unknown:
                            command = CommandCode::stop;  // Stay in STOPPED state
                            continue;
                    }
                }

                // STOPPED/CLOSED -> RUNNING/PAUSED/SUSPENDED
                receiver.publish_status(command, prev_command);

                // Set handler state on creation to avoid race condition (reception of data/schema prior to start/pause/suspend)
                if (command == CommandCode::start)
                {
                    initial_state = DdsRecorderState::RUNNING;
                }
                else if (command == CommandCode::pause)
                {
                    initial_state = DdsRecorderState::PAUSED;
                }
                else if (command == CommandCode::suspend)
                {
                    initial_state = DdsRecorderState::SUSPENDED;
                }
                else
                {
                    // Unreachable
                    eprosima::utils::tsnh(
                        eprosima::utils::Formatter() << "Trying to initiate DDS Recorder with invalid " << command <<
                            " command.");
                }

                // Reload YAML configuration file, in case it changed during STOPPED state
                // NOTE: Changes to all (but controller specific) recorder configuration options are taken into account
                configuration = eprosima::ddsrecorder::yaml::RecorderConfiguration(commandline_args.file_path);


                // configuration.recorder_configuration->topic_qos.

                // Create DDS Recorder
                auto recorder = std::make_unique<DdsRecorder>(configuration, initial_state, "", commandline_args.domain);


                // Create File Watcher Handler
                std::unique_ptr<eprosima::utils::event::FileWatcherHandler> file_watcher_handler;
                if (commandline_args.file_path != "")
                {
                    file_watcher_handler = create_filewatcher(recorder, commandline_args.file_path);
                }

                // Create Periodic Handler
                std::unique_ptr<eprosima::utils::event::PeriodicEventHandler> periodic_handler;
                if (commandline_args.reload_time > 0 && commandline_args.file_path != "")
                {
                    periodic_handler = create_periodic_handler(recorder, commandline_args.file_path,
                                    commandline_args.reload_time);
                }

                // Use flag to avoid ugly warning (start/pause an already started/paused instance)
                bool first_iter = true;
                prev_command = command;
                do
                {
                    ///////////////////////////////////////////
                    //// STATE -> RUNNING/PAUSED/SUSPENDED ////
                    ///////////////////////////////////////////
                    switch (command)
                    {
                        case CommandCode::start:
                            if (!first_iter)
                            {
                                recorder->start();
                            }
                            if (prev_command != CommandCode::start)
                            {
                                receiver.publish_status(CommandCode::start, prev_command);
                            }
                            break;

                        case CommandCode::pause:
                            if (!first_iter)
                            {
                                recorder->pause();
                            }
                            if (prev_command != CommandCode::pause)
                            {
                                receiver.publish_status(CommandCode::pause, prev_command);
                            }
                            break;

                        case CommandCode::suspend:
                            if (!first_iter)
                            {
                                recorder->suspend();
                            }
                            if (prev_command != CommandCode::suspend)
                            {
                                receiver.publish_status(CommandCode::suspend, prev_command);
                            }
                            break;

                        case CommandCode::event:
                            if (prev_command != CommandCode::pause)
                            {
                                logWarning(DDSRECORDER_EXECUTION,
                                        "Ignoring event command, instance is not paused.");

                                command = prev_command;  // Back to state before event received
                            }
                            else
                            {
                                recorder->trigger_event();
                                {
                                    // Process next_state argument if provided
                                    auto it = args.find(NEXT_STATE_TAG);
                                    if (it != args.end())
                                    {
                                        std::string next_state_str = *it;
                                        // Case insensitive
                                        eprosima::utils::to_uppercase(next_state_str);
                                        DdsRecorderState next_state;
                                        bool found = string_to_enumeration(
                                            next_state_str, next_state);
                                        if (!found ||
                                                (next_state != DdsRecorderState::RUNNING &&
                                                next_state != DdsRecorderState::SUSPENDED &&
                                                next_state != DdsRecorderState::STOPPED))
                                        {
                                            logWarning(DDSRECORDER_EXECUTION,
                                                    "Value " << next_state_str <<
                                                    " is not a valid event next_state argument (only RUNNING/SUSPENDED/STOPPED). Ignoring...");

                                            // Stay in current state if provided next_state is not valid
                                            command = prev_command;
                                        }
                                        else
                                        {
                                            command = state_to_command(next_state);
                                            continue;
                                        }
                                    }
                                    else
                                    {
                                        // Stay in current state if next_state not provided
                                        command = prev_command;
                                    }
                                }
                            }
                            break;

                        case CommandCode::stop:
                        case CommandCode::close:
                            // Unreachable
                            logError(DDSRECORDER_EXECUTION,
                                    "Reached an unstable execution state: command " << command << " case.");
                            continue;

                        default:
                        case CommandCode::unknown:
                            break;
                    }
                    prev_command = command;
                    parse_command(receiver.wait_for_command(), command, args);
                    first_iter = false;

                } while (command != CommandCode::stop && command != CommandCode::close);
            } while (command != CommandCode::close);

            // Transition to CLOSED state
            receiver.publish_status(CommandCode::close, prev_command);
        }
        else
        {
            // Start recording right away
            auto recorder = std::make_unique<DdsRecorder>(configuration, DdsRecorderState::RUNNING, "", commandline_args.domain);

            // Create File Watcher Handler
            std::unique_ptr<eprosima::utils::event::FileWatcherHandler> file_watcher_handler;
            if (commandline_args.file_path != "")
            {
                file_watcher_handler = create_filewatcher(recorder, commandline_args.file_path);
            }

            // Create Periodic Handler
            std::unique_ptr<eprosima::utils::event::PeriodicEventHandler> periodic_handler;
            if (commandline_args.reload_time > 0 && commandline_args.file_path != "")
            {
                periodic_handler = create_periodic_handler(recorder, commandline_args.file_path,
                                commandline_args.reload_time);
            }

            // Wait until signal arrives
            close_handler->wait_for_event();
        }

        logUser(DDSRECORDER_EXECUTION, "Stopping DDS Recorder.");

        logUser(DDSRECORDER_EXECUTION, "DDS Recorder stopped correctly.");
    }
    catch (const eprosima::utils::ConfigurationException& e)
    {
        logError(DDSRECORDER_ERROR,
                "Error Loading DDS Recorder Configuration from file " << commandline_args.file_path <<
                ". Error message:\n " <<
                e.what());
        return static_cast<int>(ProcessReturnCode::execution_failed);
    }
    catch (const eprosima::utils::InitializationException& e)
    {
        logError(DDSRECORDER_ERROR,
                "Error Initializing DDS Recorder. Error message:\n " <<
                e.what());
        return static_cast<int>(ProcessReturnCode::execution_failed);
    }

    logUser(DDSRECORDER_EXECUTION, "Finishing DDS Recorder execution correctly.");

    // Force print every log before closing
    eprosima::utils::Log::Flush();

    // Delete the consumers before closing
    eprosima::utils::Log::ClearConsumers();

    return static_cast<int>(ProcessReturnCode::success);
}
