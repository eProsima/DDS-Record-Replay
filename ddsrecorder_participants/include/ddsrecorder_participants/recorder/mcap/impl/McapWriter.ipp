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

#include <cpp_utils/Log.hpp>

#include <ddsrecorder_participants/recorder/mcap/McapWriter.hpp>
#include <ddsrecorder_participants/recorder/output/FullDiskException.hpp>
#include <ddsrecorder_participants/recorder/output/FullFileException.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

template <typename T>
void McapWriter::write(
        const T& data)
{
    std::lock_guard<std::mutex> lock(mutex_);

    try
    {
        write_nts_(data);
    }
    catch (const FullFileException& e)
    {
        try
        {
            on_mcap_full_nts_(e);
            write_nts_(data);
        }
        catch (const FullDiskException& e)
        {
            EPROSIMA_LOG_ERROR(DDSRECORDER_MCAP_HANDLER, "FAIL_MCAP_WRITE | Disk is full. Error message:\n " << e.what());
            on_disk_full_();
        }
    }
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
