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

#include <array>
#include <atomic>
#include <memory>

#include <ddsrecorder_participants/library/library_dll.h>

#include <cpp_utils/ReturnCode.hpp>

#include "HandlerContext.hpp"

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * @brief Collection of HandlerContext objects, indexed by HandlerKind.
 * This class implements ISchemaHandler with the idea of using it as a SchemaHandler.
 * The collection is meant to manage different handler contexts (e.g., MCAP, SQL) in a unified way.
 * Collection is meant to use in a lock-free way, with the assumption that after the collection is initialized,
 * no additional contexts will be added nor removed.
 *
 * No remove operations are provided, as the collection is expected to be initialized once and used thereafter.
 *
 */
class HandlerContextCollection
{
    using CollectionType = std::array<std::shared_ptr<HandlerContext>, HandlerContext::HandlerKind::MAX>;

public:

    /**
     * @brief Constructor.
     *
     * Constructs an empty collection with no initialized handler contexts.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI HandlerContextCollection();

    /**
     * @brief Initialize a handler context of the collection.
     *
     * Inserts a new handler context into the collection.
     *
     * @pre No other operation can be performed on the collection once it is initialized.
     * This method must be called only during the setup phase of the application.
     *
     * @param context The handler context to add.
     *
     * @return
     * - \c RETCODE_OK if the context was added successfully.
     * - \c RETCODE_ERROR if a context with the same kind already exists.
     * - \c RETCODE_PRECONDITION_NOT_MET if the collection is already initialized.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI utils::ReturnCode init_handler_context(
            std::shared_ptr<HandlerContext> context);

    /**
     * @brief Starts all handler contexts in the collection.
     *
     * This method marks the collection as initialized and starts each individual handler context.
     * Should be called once after all handlers have been configured.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI void start_nts();

    /**
     * @brief Stops all handler contexts in the collection.
     *
     * This method requests each handler context to stop gracefully.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI void stop_nts();

    /**
     * @brief Pauses all handler contexts in the collection.
     *
     * Suspends processing or output in all registered handlers, if supported by the handler implementation.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI void pause_nts();

    /**
     * @brief Triggers an event on all handler contexts in the collection.
     *
     * This function can be used to manually flush or checkpoint data depending on the handler's behavior.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI void trigger_event_nts();

    /**
     * @brief Resets the file trackers of all handler contexts in the collection.
     *
     * Useful for rolling over output files, clearing file statistics, or resetting internal state.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI void reset_file_trackers_nts();

protected:

    //! Indicates whether the collection has been started.
    std::atomic<bool> initialized_{false};

    //! Internal array of handler contexts, indexed by \c HandlerKind.
    CollectionType handlers_;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
