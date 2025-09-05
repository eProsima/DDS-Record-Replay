// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <ddsrecorder_participants/recorder/handler/HandlerContext.hpp>

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/utils.hpp>

#include <ddspipe_core/dynamic/ParticipantsDatabase.hpp>

#include <ddspipe_participants/participant/dynamic_types/SchemaParticipant.hpp>

#include <ddsrecorder_participants/recorder/handler/mcap/McapHandler.hpp>
#include <ddsrecorder_participants/recorder/handler/mcap/McapHandlerConfiguration.hpp>
#include <ddsrecorder_participants/recorder/handler/BaseHandler.hpp>
#include <ddsrecorder_participants/recorder/handler/sql/SqlHandler.hpp>
#include <ddsrecorder_participants/recorder/handler/sql/SqlHandlerConfiguration.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

HandlerContext::HandlerContext(
        HandlerKind kind,
        std::shared_ptr<participants::BaseHandler> handler,
        std::shared_ptr<ddspipe::participants::SchemaParticipant> schema_participant,
        std::shared_ptr<participants::FileTracker> file_tracker)
    : kind_(kind)
    , handler_(std::move(handler))
    , schema_participant_(std::move(schema_participant))
    , file_tracker_(std::move(file_tracker))
{

}

std::shared_ptr<HandlerContext> HandlerContext::create_context(
        HandlerKind kind,
        const participants::BaseHandlerConfiguration* handler_configuration,
        std::shared_ptr<ddspipe::participants::ParticipantConfiguration> participant_configuration,
        std::shared_ptr<eprosima::ddspipe::core::PayloadPool> payload_pool,
        std::shared_ptr<eprosima::ddspipe::core::ParticipantsDatabase> participants_database,
        std::shared_ptr<eprosima::ddspipe::core::DiscoveryDatabase> discovery_database,
        const participants::BaseHandlerStateCode& init_state,
        const std::function<void()>& on_disk_full_callback,
        const std::set<std::string> partitionlist)
{
    std::shared_ptr<HandlerContext> handler_context;
    std::shared_ptr<BaseHandler> handler;

    auto file_tracker = std::make_shared<participants::FileTracker>(
        handler_configuration->output_settings);

    switch (kind)
    {
        case HandlerKind::MCAP:
            handler = std::make_shared<participants::McapHandler>(
                *static_cast<const participants::McapHandlerConfiguration*>(handler_configuration),
                payload_pool,
                file_tracker,
                init_state,
                on_disk_full_callback);

            break;

        case HandlerKind::SQL:
            handler = std::make_shared<participants::SqlHandler>(
                *static_cast<const participants::SqlHandlerConfiguration*>(handler_configuration),
                payload_pool,
                file_tracker,
                init_state,
                on_disk_full_callback,
                partitionlist);
            break;

        default:
            throw eprosima::utils::InitializationException(
                      "Unknown handler kind: " + std::to_string(static_cast<int>(kind)));
    }

    // Create Recorder Participant
    auto participant = std::make_shared<eprosima::ddspipe::participants::SchemaParticipant>(
        participant_configuration,
        payload_pool,
        discovery_database,
        handler);

    // Populate Participant Database with the sql recorder participant
    participants_database->add_participant(
        participant->id(),
        participant
        );

    // Create entry
    handler_context.reset( new HandlerContext(
                kind,
                handler,
                participant,
                file_tracker)
            );

    return handler_context;
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
