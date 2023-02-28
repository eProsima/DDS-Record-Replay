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
 * TODO
 */
struct McapHandlerConfiguration
{
    McapHandlerConfiguration(
            const std::string& file_name,
            const unsigned int& max_pending_samples,
            const unsigned int& buffer_size,
            const unsigned int& downsampling,
            const unsigned int& event_window,
            const unsigned int& cleanup_period)
        : file_name(file_name)
        , max_pending_samples(max_pending_samples)
        , buffer_size(buffer_size)
        , downsampling(downsampling)
        , event_window(event_window)
        , cleanup_period(cleanup_period)
    {
    }

    std::string file_name;

    unsigned int max_pending_samples;

    unsigned int buffer_size;

    unsigned int downsampling;

    unsigned int event_window;

    unsigned int cleanup_period;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
