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
 * @file McapWriter.cpp
 */

#include <filesystem>
#include <stdexcept>

#include <mcap/internal.hpp>

#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/Formatter.hpp>
#include <cpp_utils/Log.hpp>
#include <cpp_utils/time/time_utils.hpp>

#include <ddsrecorder_participants/recorder/mcap/McapWriter.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

template <typename T>
void McapWriter::write(
        const T& data)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!enabled_)
    {
        logWarning(DDSRECORDER_MCAP_WRITER, "Attempting to write in a disabled writer.");
        return;
    }

    write_nts_(data);
}

template <typename T>
void McapWriter::write_nts_(
        const T& /* data */)
{
    logWarning(DDSRECORDER_MCAP_WRITER, "Attempting to write data of a not-supported type.");
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
