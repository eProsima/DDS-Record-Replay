// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <string>

#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <fastdds/utils/md5.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {
namespace detail {

inline fastdds::rtps::InstanceHandle_t compute_instance_handle_from_seed(
        const std::string& seed)
{
    fastdds::rtps::InstanceHandle_t handle;
    eprosima::fastdds::MD5 md5;

    md5.init();
    md5.update(
        reinterpret_cast<const unsigned char*>(seed.data()),
        static_cast<unsigned int>(seed.size()));
    md5.finalize();

    for (std::size_t i = 0; i < 16; ++i)
    {
        handle.value[i] = md5.digest[i];
    }

    // Keep Fast DDS from considering this instance handle undefined
    if (!handle.isDefined())
    {
        handle.value[0] = 1;
    }

    return handle;
}

} // namespace detail
} // namespace participants
} // namespace ddsrecorder
} // namespace eprosima
