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
using McapHandlerState = eprosima::ddsrecorder::participants::McapHandler::StateCode;

std::unique_ptr<core::DdsPipe> create_recorder(
        const eprosima::ddsrecorder::yaml::Configuration& configuration,
        std::shared_ptr<eprosima::ddsrecorder::participants::McapHandler>& mcap_handler,
        McapHandlerState init_state = McapHandlerState::started)
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
        configuration.downsampling,
        configuration.event_window,
        configuration.cleanup_period);

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
    return create_recorder(configuration, mcap_handler, McapHandlerState::started);
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
        eprosima::utils::event::MultipleEventHandler close_handler;

        // First of all, create signal handler so SIGINT and SIGTERM do not break the program while initializing
        close_handler.register_event_handler<eprosima::utils::event::EventHandler<eprosima::utils::event::Signal>,
                eprosima::utils::event::Signal>(
            std::make_unique<eprosima::utils::event::SignalEventHandler<eprosima::utils::event::Signal::sigint>>());     // Add SIGINT
        close_handler.register_event_handler<eprosima::utils::event::EventHandler<eprosima::utils::event::Signal>,
                eprosima::utils::event::Signal>(
            std::make_unique<eprosima::utils::event::SignalEventHandler<eprosima::utils::event::Signal::sigterm>>());    // Add SIGTERM

        // If it must be a maximum time, register a periodic handler to finish handlers
        if (timeout > 0)
        {
            close_handler.register_event_handler<eprosima::utils::event::PeriodicEventHandler>(
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
            eprosima::ddsrecorder::receiver::CommandReceiver receiver(configuration.controller_domain, &close_handler);
            receiver.init();

            // TODO: store CommandCode in YAML configuration, handle invalid option there
            CommandCode command;
            CommandCode prev = CommandCode::CLOSE;
            if (configuration.initial_command == "START")
            {
                command = CommandCode::START;
            }
            else if (configuration.initial_command == "PAUSE")
            {
                command = CommandCode::PAUSE;
            }
            else if (configuration.initial_command == "STOP")
            {
                command = CommandCode::STOP;
            }
            else
            {
                logWarning(DDSRECORDER_EXECUTION,
                        "Command " << configuration.initial_command <<
                        " is not a valid initial command (only START/PAUSE/STOP). Using instead default START initial command...");
                command = CommandCode::START;
            }

            do
            {
                // Skip waiting for commmand if initial_command is START/PAUSE (only applies to first iteration)
                if (command == CommandCode::STOP)
                {
                    ///////////////////////////
                    //// STATUS -> STOPPED ////
                    ///////////////////////////

                    // Publish state if previous -> CLOSED/STARTED/PAUSED
                    if (prev != CommandCode::STOP)
                    {
                        receiver.publish_status(CommandCode::STOP, prev);
                    }

                    prev = CommandCode::STOP;
                    receiver.wait_for_command();
                    command = receiver.command_received();
                    switch (command)
                    {
                        case CommandCode::START:
                        case CommandCode::PAUSE:
                            // Exit STOP status -> proceed
                            break;

                        case CommandCode::EVENT:
                        case CommandCode::STOP:
                            logWarning(DDSRECORDER_EXECUTION,
                                    "Ignoring " << command << " command, recorder not active yet.");
                            command = CommandCode::STOP;  // Stay in STOPPED state
                            continue;

                        case CommandCode::CLOSE:
                        case CommandCode::NONE:
                            // CLOSE command or signal received -> exit
                            continue;

                        default:
                        case CommandCode::UNKNOWN:
                            command = CommandCode::STOP;  // Stay in STOPPED state
                            continue;
                    }
                }

                // STOPPED/CLOSED -> STARTED/PAUSED
                receiver.publish_status(command, prev);

                // Set handler state on creation to avoid race condition (reception of data/schema prior to start/pause)
                McapHandlerState initial_state;
                if (command == CommandCode::START)
                {
                    initial_state = McapHandlerState::started;
                }
                else if (command == CommandCode::PAUSE)
                {
                    initial_state = McapHandlerState::paused;
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
                prev = command;
                do
                {
                    //////////////////////////////////
                    //// STATUS -> STARTED/PAUSED ////
                    //////////////////////////////////
                    switch (command)
                    {
                        case CommandCode::START:
                            if (!first_iter)
                            {
                                mcap_handler->start();
                            }
                            if (prev == CommandCode::PAUSE)
                            {
                                receiver.publish_status(CommandCode::START, CommandCode::PAUSE);
                            }
                            break;

                        case CommandCode::PAUSE:
                            if (!first_iter)
                            {
                                mcap_handler->pause();
                            }
                            if (prev == CommandCode::START)
                            {
                                receiver.publish_status(CommandCode::PAUSE, CommandCode::START);
                            }
                            break;

                        case CommandCode::EVENT:
                            mcap_handler->trigger_event();
                            // TODO: transition to STOP/START/CLOSE if argument non-empty (command=x + continue)
                            break;

                        case CommandCode::STOP:
                        case CommandCode::CLOSE:
                        case CommandCode::NONE:
                            // Unreachable
                            logError(DDSRECORDER_EXECUTION,
                                    "Reached an unstable execution state: command " << command << " case.");
                            continue;

                        default:
                        case CommandCode::UNKNOWN:
                            break;
                    }
                    receiver.wait_for_command();
                    prev = command;
                    command = receiver.command_received();
                    first_iter = false;

                } while (command != CommandCode::STOP && command != CommandCode::CLOSE && command != CommandCode::NONE);
            } while (command != CommandCode::CLOSE && command != CommandCode::NONE);

            // Transition to CLOSED state
            receiver.publish_status(CommandCode::CLOSE, prev);
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
            close_handler.wait_for_event();
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
