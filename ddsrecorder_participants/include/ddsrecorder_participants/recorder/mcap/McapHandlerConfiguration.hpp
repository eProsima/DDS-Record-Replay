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
 * @file McapHandlerConfiguration.hpp
 */

#pragma once

#include <mcap/mcap.hpp>

#include <ddsrecorder_participants/recorder/output/BaseHandlerConfiguration.hpp>
#include <ddsrecorder_participants/recorder/output/OutputSettings.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Structure encapsulating all of \c McapHandler configuration options.
 */
struct McapHandlerConfiguration : public BaseHandlerConfiguration
{
    McapHandlerConfiguration(
            const OutputSettings& output_settings,
            const int max_pending_samples,
            const unsigned int buffer_size,
            const unsigned int event_window,
            const unsigned int cleanup_period,
            const bool log_publishTime,
            const bool only_with_schema,
            const mcap::McapWriterOptions& mcap_writer_options,
            const bool record_types,
            const bool ros2_types)
        : BaseHandlerConfiguration(
                output_settings,
                max_pending_samples,
                buffer_size,
                event_window,
                cleanup_period,
                log_publishTime,
                only_with_schema,
                record_types,
                ros2_types)
        , mcap_writer_options(mcap_writer_options)
    {
    }

    //! Mcap writer configuration options
    mcap::McapWriterOptions mcap_writer_options;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
