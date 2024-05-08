// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PreconditionNotMet.hpp
 */

#pragma once

#include <cpp_utils/exception/Exception.hpp>

namespace eprosima {
namespace utils {

/**
 * @brief Exception thrown when a precondition is not met before calling a function or running a routine.
 *
 * For example:
 * - Reading a file that does not exist or has no read permissions.
 * - One argument does not fulfill the preconditions (a ptr with value nullptr).
 */
class PreconditionNotMet : public Exception
{
    // Use parent class constructors
    using Exception::Exception;
};

} // namespace utils
} // namespace eprosima
