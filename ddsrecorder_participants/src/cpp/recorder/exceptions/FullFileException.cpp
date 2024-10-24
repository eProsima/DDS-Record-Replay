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
 * @file FullFileException.cpp
 */

#include <ddsrecorder_participants/recorder/exceptions/FullFileException.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

FullFileException::FullFileException(
        const std::string& message,
        const std::uint64_t data_size_to_write)
    : Exception(message)
    , data_size_to_write_(data_size_to_write)
{
}

std::uint64_t FullFileException::data_size_to_write() const
{
    return data_size_to_write_;
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
