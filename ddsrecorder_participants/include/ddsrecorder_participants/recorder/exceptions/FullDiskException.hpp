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
 * @file FullDiskException.hpp
 */

#pragma once

#include <cstdint>
#include <string>

#include <cpp_utils/exception/Exception.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * @brief Exception thrown when trying to write to a file that is full.
 */
class FullDiskException : public utils::Exception
{
public:

    FullDiskException(
            const std::string& message);

    virtual ~FullDiskException() noexcept override = default;
};

} // namespace participants
} // namespace ddsrecorder
} // namespace eprosima
