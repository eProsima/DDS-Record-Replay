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

#include <ddsrecorder_participants/recorder/handler/BaseHandler.hpp>
#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>

namespace eprosima {
namespace ddspipe {
namespace core {

    class PayloadPool;

} /* namespace core */
} /* namespace ddspipe */

namespace ddsrecorder {
namespace participants {

struct McapHandlerConfiguration;

class McapHandler : public BaseHandler
{
public:

    McapHandler() = default;

    McapHandler(
            const McapHandlerConfiguration&,
            const std::shared_ptr<ddspipe::core::PayloadPool>&,
            std::shared_ptr<FileTracker>,
            const BaseHandlerStateCode& init_state = BaseHandlerStateCode::RUNNING,
            const std::function<void()>& on_disk_full_lambda = nullptr)
    {

    }
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
