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
 * @file file_utils.hpp
 */

#pragma once

#include <string>
#include <vector>

#include <cpp_utils/library/library_dll.h>

namespace eprosima {
namespace utils {

/**
 * @brief Read a file and convert it to a single string.
 *
 * @param file_name name of the file to read
 * @param strip_chars whether to eliminate not desired characters from string (as windows line breaks)
 *
 * @return String with the whole file
 */
CPP_UTILS_DllAPI std::string file_to_string(
        const char* file_name,
        bool strip_chars = true);

/**
 * @brief Read a file and convert it to a string per line in file.
 *
 * @param file_name name of the file to read
 * @param strip_chars whether to eliminate not desired characters from string (as windows line breaks)
 * @param strip_empty_lines whether empty lines should be removed or else add it as a value of the vector
 *
 * @return Vector of strings with the whole file
 */
CPP_UTILS_DllAPI std::vector<std::string> file_to_strings(
        const char* file_name,
        bool strip_chars = true,
        bool strip_empty_lines = false);

} /* namespace utils */
} /* namespace eprosima */
