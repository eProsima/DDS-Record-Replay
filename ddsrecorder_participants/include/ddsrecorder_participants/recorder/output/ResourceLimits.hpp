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
 * @file OutputSettings.hpp
 */

#pragma once

#include <cstdint>


namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * Structure encapsulating all resource limits configuration options
 */
struct ResourceLimitsStruct
{
    std::uint64_t max_size_{0};
    std::uint64_t max_file_size_{0};
    std::uint64_t size_tolerance_{1024 * 1024};     // Force the system to have a minimum tolerance of 1MB
    bool file_rotation_{false};
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
