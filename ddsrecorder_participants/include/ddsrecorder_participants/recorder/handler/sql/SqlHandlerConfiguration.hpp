// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SqlHandlerConfiguration.hpp
 */

#pragma once

#include <cpp_utils/macros/custom_enumeration.hpp>

#include <ddsrecorder_participants/recorder/handler/BaseHandlerConfiguration.hpp>
#include <ddsrecorder_participants/recorder/output/OutputSettings.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

ENUMERATION_BUILDER(
    DataFormat,
    cdr,
    json,
    both
    );

/**
 * Structure encapsulating the \c SqlHandler configuration options.
 */
struct SqlHandlerConfiguration : public BaseHandlerConfiguration
{
    /**
     * @brief Constructor for \c SqlHandlerConfiguration.
     *
     * @param output_settings:         Configuration settings for the output file where data is to be written.
     * @param max_pending_samples:     Max number of messages to store in memory when schema
     *                                 not yet available.
     * @param buffer_size:             Max number of elements to keep in memory prior to writing
     *                                 in disk (applies to started state).
     * @param event_window:            Keep in memory samples received in time frame [s],
     *                                 to be stored when event triggered (applies to paused state).
     * @param cleanup_period:          Remove from buffer samples older than *now - event_window*
     *                                 with this period [s] (applies to paused state).
     * @param only_with_schema:        Only write messages whose schema is registered
     *                                 (i.e. discard pending samples when leaving RUNNING state).
     * @param record_types:            Whether to store received dynamic types in the output file.
     * @param ros2_types:              Whether to schemas are in OMG IDL or ROS.
     * @param data_format:             Whether to store data in cdr, in json, or in both.
     */
    SqlHandlerConfiguration(
            const OutputSettings& output_settings,
            const int max_pending_samples,
            const unsigned int buffer_size,
            const unsigned int event_window,
            const unsigned int cleanup_period,
            const bool only_with_schema,
            const bool record_types,
            const bool ros2_types,
            const DataFormat data_format)
        : BaseHandlerConfiguration(
            output_settings,
            max_pending_samples,
            buffer_size,
            event_window,
            cleanup_period,
            only_with_schema,
            record_types,
            ros2_types)
        , data_format(data_format)
    {
    }

    //! Whether to store data in cdr, in json, or in both.
    DataFormat data_format;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
