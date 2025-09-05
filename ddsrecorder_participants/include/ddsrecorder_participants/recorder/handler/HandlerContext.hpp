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

#pragma once

#include <functional>
#include <memory>

#include <ddspipe_participants/configuration/ParticipantConfiguration.hpp>

#include <ddsrecorder_participants/recorder/handler/BaseHandlerConfiguration.hpp>

#include <ddsrecorder_participants/library/library_dll.h>

namespace eprosima {
namespace ddspipe {
namespace participants {

class SchemaParticipant;

} /* namespace participants */

namespace core {

class DiscoveryDatabase;
class PayloadPool;
class ParticipantsDatabase;

} /* namespace core */
} /* namespace ddspipe */

namespace ddsrecorder {
namespace participants {

class BaseHandler;
class FileTracker;
class HandlerContextCollection;
enum class BaseHandlerStateCode;

/**
 * @brief Base context class for data handlers.
 *
 * This class contains the shared context used by a specific handler instance,
 * including its kind, core components, and runtime dependencies.
 * Instances should be created through the static \c create_context factory method.
 */
class HandlerContext
{
    //! Allow \c HandlerContextCollection to access protected members.
    friend class HandlerContextCollection;

public:

    /**
     * @brief Enumeration of available handler kinds.
     */
    enum HandlerKind : size_t
    {
        MCAP = 0,  //!< Handler for MCAP recording.
        SQL,       //!< Handler for SQL database output.
        MAX        //!< Sentinel value used for bounds validation.
    };

    /**
     * @brief Virtual destructor.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI virtual ~HandlerContext() = default;

    /**
     * @brief Factory method to create a new handler context.
     *
     * This static method constructs a specific handler context implementation based
     * on the provided handler kind and required initialization dependencies.
     *
     * @param kind Type of handler to create.
     * @param handler_configuration Configuration for the handler to initialize.
     * @param participant_configuration Shared pointer to the participant configuration.
     * @param payload_pool Shared pointer to the payload pool.
     * @param participants_database Shared pointer to the participants database.
     * @param discovery_database Shared pointer to the discovery database.
     * @param init_state Initial handler state code.
     * @param on_disk_full_callback Callback to invoke when disk is full.
     *
     * @return Shared pointer to the created \c HandlerContext.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI static std::shared_ptr<HandlerContext> create_context(
            HandlerKind kind,
            const BaseHandlerConfiguration* handler_configuration,
            std::shared_ptr<ddspipe::participants::ParticipantConfiguration> participant_configuration,
            std::shared_ptr<eprosima::ddspipe::core::PayloadPool> payload_pool,
            std::shared_ptr<eprosima::ddspipe::core::ParticipantsDatabase> participants_database,
            std::shared_ptr<eprosima::ddspipe::core::DiscoveryDatabase> discovery_database,
            const participants::BaseHandlerStateCode& init_state,
            const std::function<void()>& on_disk_full_callback,
            const std::set<std::string> partitionlist);

protected:

    /**
     * @brief Constructor.
     *
     * This constructor is intended to be used by the \c create_context factory method only.
     *
     * @param kind Type of handler.
     * @param handler Shared pointer to the handler instance.
     * @param schema_participant Shared pointer to the schema participant.
     * @param file_tracker Shared pointer to the file tracker.
     */
    HandlerContext(
            HandlerKind kind,
            std::shared_ptr<participants::BaseHandler> handler,
            std::shared_ptr<ddspipe::participants::SchemaParticipant> schema_participant,
            std::shared_ptr<participants::FileTracker> file_tracker);

    //! Type of handler.
    HandlerKind kind_;

    //! Handler instance.
    std::shared_ptr<participants::BaseHandler> handler_;

    //! Schema participant associated with this handler.
    std::shared_ptr<ddspipe::participants::SchemaParticipant> schema_participant_;

    //! File tracker to monitor and manage output files.
    std::shared_ptr<participants::FileTracker> file_tracker_;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */