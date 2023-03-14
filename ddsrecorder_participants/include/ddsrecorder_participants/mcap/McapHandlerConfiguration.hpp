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

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Structure encapsulating all of \c McapHandler configuration options.
 */
struct McapHandlerConfiguration
{
    McapHandlerConfiguration(
            const std::string& file_name,
            const unsigned int& max_pending_samples,
            const unsigned int& buffer_size,
            const unsigned int& event_window,
            const unsigned int& cleanup_period,
            const bool& log_publishTime)
        : file_name(file_name)
        , max_pending_samples(max_pending_samples)
        , buffer_size(buffer_size)
        , event_window(event_window)
        , cleanup_period(cleanup_period)
        , log_publishTime(log_publishTime)
    {
    }

    //! Name of MCAP file where data is to be written
    std::string file_name;

    //! Max number of messages to store in memory when schema not yet available
    unsigned int max_pending_samples;

    //! Max number of elements to keep in memory prior to writting in disk (applies to started state)
    unsigned int buffer_size;

    //! Keep in memory samples received in time frame [s], to be stored when event triggered (applies to paused state)
    unsigned int event_window;

    //! Remove from buffer samples older than *now - event_window* with this period [s] (applies to paused state)
    unsigned int cleanup_period;

    //! Store messages with logTime set to sample publication timestamp
    bool log_publishTime;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
