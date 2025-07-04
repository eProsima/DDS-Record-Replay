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

#include <memory>
#include <list>

#include <ddspipe_participants/participant/dynamic_types/ISchemaHandler.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

struct BaseMessage;

enum class BaseHandlerStateCode
{
    STOPPED,                  //! Received messages are not processed.
    RUNNING,                  //! Messages are stored in buffer and dumped to disk when full.
    PAUSED                    //! Messages are stored in buffer and dumped to disk when event triggered.
};

// BaseHandler Mock
class BaseHandler : public ddspipe::participants::ISchemaHandler
{
public:

    virtual ~BaseHandler() = default;

    virtual void enable()
    {

    }

    virtual void disable()
    {

    }

    virtual void start()
    {

    }

    virtual void stop()
    {

    }

    virtual void pause()
    {

    }

    virtual void trigger_event()
    {

    }

    virtual void reset_file_trackers()
    {

    }

    virtual void write_samples_(
            std::list<std::shared_ptr<const BaseMessage>>&)
    {

    }

};

} // namespace participants
} // namespace ddsrecorder
} // namespace eprosima