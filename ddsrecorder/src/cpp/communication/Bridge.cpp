// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file Bridge.cpp
 *
 */

#include <communication/Bridge.hpp>

#include <cpp_utils/exception/UnsupportedException.hpp>
#include <cpp_utils/Log.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {

using namespace eprosima::ddsrecorder::core::types;

Bridge::Bridge(
        std::shared_ptr<ParticipantsDatabase> participants_database,
        std::shared_ptr<PayloadPool> payload_pool,
        std::shared_ptr<utils::SlotThreadPool> thread_pool)
    : participants_(participants_database)
    , payload_pool_(payload_pool)
    , thread_pool_(thread_pool)
    , enabled_(false)
{
}

} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
