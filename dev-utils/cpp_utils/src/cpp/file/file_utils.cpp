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
 * @file file_utils.cpp
 */

#include <iostream> // TODO remove
#include <fstream>

#include <cpp_utils/exception/PreconditionNotMet.hpp>
#include <cpp_utils/file/file_utils.hpp>
#include <cpp_utils/utils.hpp>

namespace eprosima {
namespace utils {

std::string file_to_string(
        const char* file_name,
        bool strip_chars /* = true */)
{
    auto strings = file_to_strings(file_name, strip_chars, false);
    std::string result;

    for (const auto& st : strings)
    {
        result += st + "\n";
    }

    return result;
}

std::vector<std::string> file_to_strings(
        const char* file_name,
        bool strip_chars /* = true */,
        bool strip_empty_lines /* = false */)
{
    std::string new_line;
    std::vector<std::string> result;

    // Open file
    std::ifstream file(file_name);

    if (!file.is_open())
    {
        throw PreconditionNotMet(
                  STR_ENTRY << "File <" << file_name << "> could not be read.");
    }

    while (getline(file, new_line))
    {
        if (strip_chars)
        {
            strip_str(new_line);
        }

        if (!(strip_empty_lines && new_line.empty()))
        {
            result.push_back(new_line);
        }
    }

    return result;
}

} /* namespace utils */
} /* namespace eprosima */
