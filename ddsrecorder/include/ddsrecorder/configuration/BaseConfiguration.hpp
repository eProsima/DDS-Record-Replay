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
 * @file BaseConfiguration.hpp
 */

#ifndef _DDSRECORDER_CONFIGURATION_BASECONFIGURATION_HPP_
#define _DDSRECORDER_CONFIGURATION_BASECONFIGURATION_HPP_

#include <cpp_utils/Formatter.hpp>

#include <ddsrecorder/library/library_dll.h>

namespace eprosima {
namespace ddsrecorder {
namespace core {
namespace configuration {

/**
 * Configurations in DDS Recorder are data structures with public access to its internal methods.
 * Thus, they are not forced to be correct in construction.
 * This is an Interface class that forces every configuration in ddsrecorder to have an \c is_valid method.
 */
struct BaseConfiguration
{
    DDSRECORDER_DllAPI virtual bool is_valid(
            utils::Formatter& error_msg) const noexcept = 0;
};

} /* namespace configuration */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */

#endif /* _DDSRECORDER_CONFIGURATION_BASECONFIGURATION_HPP_ */
