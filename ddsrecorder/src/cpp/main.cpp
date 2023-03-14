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

#include <nlohmann/json.hpp>

#include <cpp_utils/event/FileWatcherHandler.hpp>
#include <cpp_utils/event/MultipleEventHandler.hpp>
#include <cpp_utils/event/PeriodicEventHandler.hpp>
#include <cpp_utils/event/SignalEventHandler.hpp>
#include <cpp_utils/exception/ConfigurationException.hpp>
#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/logging/CustomStdLogConsumer.hpp>
#include <cpp_utils/ReturnCode.hpp>
#include <cpp_utils/thread_pool/pool/SlotThreadPool.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/utils.hpp>

#include <ddspipe_core/core/DdsPipe.hpp>
#include <ddspipe_core/dynamic/AllowedTopicList.hpp>
#include <ddspipe_core/dynamic/DiscoveryDatabase.hpp>
#include <ddspipe_core/dynamic/ParticipantsDatabase.hpp>
#include <ddspipe_core/efficiency/payload/FastPayloadPool.hpp>

#include <ddspipe_participants/participant/dynamic_types/DynTypesParticipant.hpp>
#include <ddspipe_participants/participant/dynamic_types/SchemaParticipant.hpp>

#include <ddsrecorder_participants/mcap/McapHandler.hpp>
#include <ddsrecorder_participants/mcap/McapHandlerConfiguration.hpp>

#include <ddsrecorder_yaml/YamlReaderConfiguration.hpp>

#include "user_interface/constants.hpp"
#include "user_interface/arguments_configuration.hpp"
#include "user_interface/ProcessReturnCode.hpp"

#include "command_receiver/CommandReceiver.hpp"

using namespace eprosima::ddspipe;
using namespace eprosima::ddsrecorder;

using CommandCode = eprosima::ddsrecorder::receiver::CommandCode;
using json = nlohmann::json;
using McapHandlerState = eprosima::ddsrecorder::participants::McapHandlerStateCode;

const std::string NEXT_STATE_TAG = "next_state";
constexpr auto string_to_command = eprosima::ddsrecorder::receiver::string_to_enumeration;
constexpr auto string_to_state = eprosima::ddsrecorder::participants::string_to_enumeration;

std::unique_ptr<core::DdsPipe> create_recorder(
        const eprosima::ddsrecorder::yaml::Configuration& configuration,
        std::shared_ptr<eprosima::ddsrecorder::participants::McapHandler>& mcap_handler,
        McapHandlerState init_state)
{
    // Create allowed topics list
    auto allowed_topics = std::make_shared<core::AllowedTopicList>(
        configuration.allowlist,
        configuration.blocklist);

    // Create Discovery Database
    std::shared_ptr<core::DiscoveryDatabase> discovery_database =
            std::make_shared<core::DiscoveryDatabase>();

    // Create Payload Pool
    std::shared_ptr<core::PayloadPool> payload_pool =
            std::make_shared<core::FastPayloadPool>();

    // Create Thread Pool
    std::shared_ptr<eprosima::utils::SlotThreadPool> thread_pool =
            std::make_shared<eprosima::utils::SlotThreadPool>(configuration.n_threads);

    // Create MCAP Handler configuration
    std::string file_name = configuration.recorder_output_file + "_" + eprosima::utils::timestamp_to_string(
        eprosima::utils::now()) + ".mcap";
    eprosima::ddsrecorder::participants::McapHandlerConfiguration handler_config(
        file_name,
        configuration.max_pending_samples,
        configuration.buffer_size,
        configuration.event_window,
        configuration.cleanup_period,
        configuration.log_publish_time);

    // Create MCAP Handler
    mcap_handler = std::make_shared<eprosima::ddsrecorder::participants::McapHandler>(
        handler_config,
        payload_pool,
        init_state);

    // Create DynTypes Participant
    auto dyn_participant = std::make_shared<eprosima::ddspipe::participants::DynTypesParticipant>(
        configuration.simple_configuration,
        payload_pool,
        discovery_database);
    dyn_participant->init();

    // Create Recorder Participant
    auto recorder_participant = std::make_shared<eprosima::ddspipe::participants::SchemaParticipant>(
        configuration.recorder_configuration,
        payload_pool,
        discovery_database,
        mcap_handler);

    // Create and populate Participant Database
    std::shared_ptr<core::ParticipantsDatabase> participant_database =
            std::make_shared<core::ParticipantsDatabase>();

    // Populate Participant Database
    participant_database->add_participant(
        dyn_participant->id(),
        dyn_participant
        );
    participant_database->add_participant(
        recorder_participant->id(),
        recorder_participant
        );

    return std::make_unique<core::DdsPipe>(
        allowed_topics,
        discovery_database,
        payload_pool,
        participant_database,
        thread_pool,
        configuration.builtin_topics,
        true
        );
}

std::unique_ptr<core::DdsPipe> create_recorder(
        const eprosima::ddsrecorder::yaml::Configuration& configuration)
{
    std::shared_ptr<eprosima::ddsrecorder::participants::McapHandler> mcap_handler;
    return create_recorder(configuration, mcap_handler, McapHandlerState::RUNNING);
}

std::unique_ptr<eprosima::utils::event::FileWatcherHandler> create_filewatcher(
        const std::unique_ptr<core::DdsPipe>& recorder,
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
                    eprosima::ddsrecorder::yaml::Configuration new_configuration(file_path);
                    // Create new allowed topics list
                    auto new_allowed_topics = std::make_shared<core::AllowedTopicList>(
                        new_configuration.allowlist,
                        new_configuration.blocklist);
                    recorder->reload_allowed_topics(new_allowed_topics);
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
        const std::unique_ptr<core::DdsPipe>& recorder,
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
                    eprosima::ddsrecorder::yaml::Configuration new_configuration(file_path);
                    // Create new allowed topics list
                    auto new_allowed_topics = std::make_shared<core::AllowedTopicList>(
                        new_configuration.allowlist,
                        new_configuration.blocklist);
                    recorder->reload_allowed_topics(new_allowed_topics);
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
                " is not a valid command (only start/pause/stop/close).");
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
        const McapHandlerState& state)
{
    switch (state)
    {
        case McapHandlerState::RUNNING:
            return CommandCode::start;

        case McapHandlerState::PAUSED:
            return CommandCode::pause;

        case McapHandlerState::STOPPED:
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
    // Configuration File path
    std::string file_path = "";

    // Reload time
    eprosima::utils::Duration_ms reload_time = 0;

    // Maximum timeout
    eprosima::utils::Duration_ms timeout = 0;

    // Debug options
    std::string log_filter = "(DDSPIPE|DDSRECORDER)";
    eprosima::fastdds::dds::Log::Kind log_verbosity = eprosima::fastdds::dds::Log::Kind::Warning;

    // Parse arguments
    ui::ProcessReturnCode arg_parse_result =
            ui::parse_arguments(argc, argv, file_path, reload_time, timeout, log_filter, log_verbosity);

    if (arg_parse_result == ui::ProcessReturnCode::help_argument)
    {
        return static_cast<int>(ui::ProcessReturnCode::success);
    }
    else if (arg_parse_result == ui::ProcessReturnCode::version_argument)
    {
        return static_cast<int>(ui::ProcessReturnCode::success);
    }
    else if (arg_parse_result != ui::ProcessReturnCode::success)
    {
        return static_cast<int>(arg_parse_result);
    }

    // Check file is in args, else get the default file
    if (file_path == "")
    {
        if (is_file_accessible(ui::DEFAULT_CONFIGURATION_FILE_NAME, eprosima::utils::FileAccessMode::read))
        {
            file_path = ui::DEFAULT_CONFIGURATION_FILE_NAME;

            logUser(
                DDSRECORDER_EXECUTION,
                "Not configuration file given, using default file " << file_path << ".");
        }
    }
    else
    {
        // Check file exists and it is readable
        // NOTE: this check is redundant with option parse arg check
        if (!is_file_accessible(file_path.c_str(), eprosima::utils::FileAccessMode::read))
        {
            logError(
                DDSRECORDER_ARGS,
                "File '" << file_path << "' does not exist or it is not accessible.");
            return static_cast<int>(ui::ProcessReturnCode::required_argument_failed);
        }
    }

    logUser(DDSRECORDER_EXECUTION, "Starting DDS Recorder execution.");

    // Logging
    {
        // Remove every consumer
        eprosima::utils::Log::ClearConsumers();

        // Activate log with verbosity, as this will avoid running log thread with not desired kind
        eprosima::utils::Log::SetVerbosity(log_verbosity);

        eprosima::utils::Log::RegisterConsumer(
            std::make_unique<eprosima::utils::CustomStdLogConsumer>(log_filter, log_verbosity));
    }

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
        if (timeout > 0)
        {
            close_handler->register_event_handler<eprosima::utils::event::PeriodicEventHandler>(
                std::make_unique<eprosima::utils::event::PeriodicEventHandler>(
                    []()
                    {
                        /* Do nothing */ },
                    timeout));
        }

        /////
        // DDS Recorder Initialization

        // Load configuration from YAML
        eprosima::ddsrecorder::yaml::Configuration configuration(file_path);

        logUser(DDSRECORDER_EXECUTION, "DDS Recorder running.");

        if (configuration.enable_remote_controller)
        {
            logUser(DDSRECORDER_EXECUTION, "Waiting for instructions...");
            eprosima::ddsrecorder::receiver::CommandReceiver receiver(configuration.controller_domain,
                    configuration.command_topic_name,
                    configuration.status_topic_name, close_handler);
            receiver.init();

            CommandCode prev_command;
            CommandCode command;
            json args;

            // Parse and convert initial state to initial command
            McapHandlerState initial_state;
            bool found = string_to_state(configuration.initial_state, initial_state);
            if (!found)
            {
                logWarning(DDSRECORDER_EXECUTION,
                        "Initial state " << configuration.initial_state <<
                        " is not a valid one (only RUNNING/PAUSED/STOPPED). Using instead default RUNNING initial state...");
                initial_state = McapHandlerState::RUNNING;
            }
            command = state_to_command(initial_state);

            prev_command = CommandCode::close;
            do
            {
                // Skip waiting for commmand if initial_state is RUNNING/PAUSED (only applies to first iteration)
                if (command == CommandCode::stop)
                {
                    //////////////////////////
                    //// STATE -> STOPPED ////
                    //////////////////////////

                    // Publish state if previous -> CLOSED/RUNNING/PAUSED
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

                // STOPPED/CLOSED -> RUNNING/PAUSED
                receiver.publish_status(command, prev_command);

                // Set handler state on creation to avoid race condition (reception of data/schema prior to start/pause)
                if (command == CommandCode::start)
                {
                    initial_state = McapHandlerState::RUNNING;
                }
                else if (command == CommandCode::pause)
                {
                    initial_state = McapHandlerState::PAUSED;
                }
                else
                {
                    // Unreachable
                    eprosima::utils::tsnh(
                        eprosima::utils::Formatter() << "Trying to initiate McapHandler with invalid " << command <<
                            " command.");
                }

                // Reload YAML configuration file, in case it changed during STOPPED state
                // NOTE: Changes to all (but controller specific) recorder configuration options are taken into account
                configuration = eprosima::ddsrecorder::yaml::Configuration(file_path);

                // Create DDS Recorder
                std::shared_ptr<eprosima::ddsrecorder::participants::McapHandler> mcap_handler;
                auto recorder = create_recorder(configuration, mcap_handler, initial_state);

                // Create File Watcher Handler
                std::unique_ptr<eprosima::utils::event::FileWatcherHandler> file_watcher_handler;
                if (file_path != "")
                {
                    file_watcher_handler = create_filewatcher(recorder, file_path);
                }

                // Create Periodic Handler
                std::unique_ptr<eprosima::utils::event::PeriodicEventHandler> periodic_handler;
                if (reload_time > 0 && file_path != "")
                {
                    periodic_handler = create_periodic_handler(recorder, file_path, reload_time);
                }

                // Use flag to avoid ugly warning (start/pause an already started/paused instance)
                bool first_iter = true;
                prev_command = command;
                do
                {
                    /////////////////////////////////
                    //// STATE -> RUNNING/PAUSED ////
                    /////////////////////////////////
                    switch (command)
                    {
                        case CommandCode::start:
                            if (!first_iter)
                            {
                                mcap_handler->start();
                            }
                            if (prev_command == CommandCode::pause)
                            {
                                receiver.publish_status(CommandCode::start, CommandCode::pause);
                            }
                            break;

                        case CommandCode::pause:
                            if (!first_iter)
                            {
                                mcap_handler->pause();
                            }
                            if (prev_command == CommandCode::start)
                            {
                                receiver.publish_status(CommandCode::pause, CommandCode::start);
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
                                mcap_handler->trigger_event();
                                {
                                    // Process next_state argument if provided
                                    auto it = args.find(NEXT_STATE_TAG);
                                    if (it != args.end())
                                    {
                                        std::string next_state_str = *it;
                                        // Case insensitive
                                        eprosima::utils::to_uppercase(next_state_str);
                                        McapHandlerState next_state;
                                        bool found = string_to_state(next_state_str, next_state);
                                        if (!found ||
                                                (next_state != McapHandlerState::RUNNING &&
                                                next_state != McapHandlerState::STOPPED))
                                        {
                                            logWarning(DDSRECORDER_EXECUTION,
                                                    "Value " << next_state_str <<
                                                    " is not a valid event next_state argument (only RUNNING/STOPPED). Ignoring...");

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
            auto recorder = create_recorder(configuration);

            // Create File Watcher Handler
            std::unique_ptr<eprosima::utils::event::FileWatcherHandler> file_watcher_handler;
            if (file_path != "")
            {
                file_watcher_handler = create_filewatcher(recorder, file_path);
            }

            // Create Periodic Handler
            std::unique_ptr<eprosima::utils::event::PeriodicEventHandler> periodic_handler;
            if (reload_time > 0 && file_path != "")
            {
                periodic_handler = create_periodic_handler(recorder, file_path, reload_time);
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
                "Error Loading DDS Recorder Configuration from file " << file_path <<
                ". Error message:\n " <<
                e.what());
        return static_cast<int>(ui::ProcessReturnCode::execution_failed);
    }
    catch (const eprosima::utils::InitializationException& e)
    {
        logError(DDSRECORDER_ERROR,
                "Error Initializing DDS Recorder. Error message:\n " <<
                e.what());
        return static_cast<int>(ui::ProcessReturnCode::execution_failed);
    }

    logUser(DDSRECORDER_EXECUTION, "Finishing DDS Recorder execution correctly.");

    // Force print every log before closing
    eprosima::utils::Log::Flush();

    return static_cast<int>(ui::ProcessReturnCode::success);
}
