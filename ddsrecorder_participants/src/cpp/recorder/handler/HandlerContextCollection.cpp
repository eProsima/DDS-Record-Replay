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

#include <ddsrecorder_participants/recorder/handler/HandlerContextCollection.hpp>
#include <ddsrecorder_participants/recorder/handler/BaseHandler.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

HandlerContextCollection::HandlerContextCollection()
{
    handlers_.fill(nullptr);
}

utils::ReturnCode HandlerContextCollection::init_handler_context(
        std::shared_ptr<HandlerContext> context)
{
    if (initialized_)
    {
        return utils::ReturnCode::RETCODE_PRECONDITION_NOT_MET;
    }

    if (handlers_[context->kind_] != nullptr)
    {
        return utils::ReturnCode::RETCODE_ERROR;
    }

    handlers_[context->kind_] = std::move(context);
    return utils::ReturnCode::RETCODE_OK;
}

void HandlerContextCollection::start_nts()
{
    bool expected = false;
    initialized_.compare_exchange_weak(expected, true, std::memory_order_relaxed);

    for (auto& handler : handlers_)
    {
        if (handler)
        {
            handler->handler_->start();
        }
    }
}

void HandlerContextCollection::stop_nts()
{
    for (auto& handler : handlers_)
    {
        if (handler)
        {
            handler->handler_->stop();
        }
    }
}

void HandlerContextCollection::pause_nts()
{
    for (auto& handler : handlers_)
    {
        if (handler)
        {
            handler->handler_->pause();
        }
    }
}

void HandlerContextCollection::trigger_event_nts()
{
    for (auto& handler : handlers_)
    {
        if (handler)
        {
            handler->handler_->trigger_event();
        }
    }
}

void HandlerContextCollection::reset_file_trackers_nts()
{
    for (auto& handler : handlers_)
    {
        if (handler)
        {
            handler->file_tracker_.reset();
        }
    }
}

} /* participants */
} /* ddsrecorder */
} /* eprosima */