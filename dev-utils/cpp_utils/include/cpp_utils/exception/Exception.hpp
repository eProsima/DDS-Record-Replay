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
 * @file Exception.hpp
 */

#pragma once

#include <exception>
#include <string>

#include <cpp_utils/Formatter.hpp>
#include <cpp_utils/library/library_dll.h>

namespace eprosima {
namespace utils {

/**
 * @brief Base class for all exceptions thrown by the eProsima DDSRouter library.
 *
 */
class Exception : public std::exception
{

public:

    /**
     * @brief Construct a new ddsrouter::Exception object
     *
     * @param message The message to be returned by what()
     */
    CPP_UTILS_DllAPI Exception(
            const char* message) noexcept;

    /**
     * @brief Construct a new ddsrouter::Exception object
     *
     * @param message The message to be returned by what()
     */
    CPP_UTILS_DllAPI Exception(
            const std::string& message);

    /**
     * @brief Construct a new ddsrouter::Exception object by concatenating streams in a \c Formatter
     *
     * This constructor allows to concatenate several streams in the same object in the same constructor call.
     * For example: Exception(Formatter() << " object1 stream: " << obj1 << " object2 stream: " << obj2);
     *
     * @param formatter The \c Formatter object where streams are concatenated
     */
    CPP_UTILS_DllAPI Exception(
            const utils::Formatter& formatter);

    /**
     * @brief Copies the ddsrouter::Exception object into a new one
     *
     * @param other The original exception object to copy
     */
    CPP_UTILS_DllAPI Exception(
            const Exception& other) = default;

    /**
     * @brief Copies the ddsrouter::Exception object into the current one
     *
     * @param other The original exception object to copy
     * @return the current ddsrouter::Exception object after the copy
     */
    CPP_UTILS_DllAPI Exception& operator =(
            const Exception& other) = default;

    /**
     * @brief Returns the explanatory string of the exception
     *
     * @return Null-terminated string with the explanatory information
     */
    CPP_UTILS_DllAPI virtual const char* what() const noexcept override;

protected:

    std::string message_;
};

} // namespace utils
} // namespace eprosima
