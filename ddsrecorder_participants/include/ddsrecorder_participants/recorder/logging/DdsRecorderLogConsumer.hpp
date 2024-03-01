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
 * @file DdsRecorderLogConsumer.hpp
 */

#pragma once

#include <ddspipe_core/configuration/DdsPipeLogConfiguration.hpp>
#include <ddspipe_core/logging/DdsLogConsumer.hpp>

#include <ddsrecorder_participants/library/library_dll.h>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * DDS Recorder Log Consumer with Standard (logical) behaviour.
 *
 * Registering this consumer in Fast DDS's Log publishes the log entries accepted by the \c BaseLogConsumer.
 */
class DdsRecorderLogConsumer : public ddspipe::core::DdsLogConsumer
{
public:

    //! Create a new \c DdsRecorderLogConsumer from a \c DdsPipeLogConfiguration .
    DDSRECORDER_PARTICIPANTS_DllAPI
    DdsRecorderLogConsumer(
            const ddspipe::core::DdsPipeLogConfiguration* configuration);
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
