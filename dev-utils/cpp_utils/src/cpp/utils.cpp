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
 * @file utils.cpp
 *
 */

#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <iomanip>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>

#include <cpp_utils/exception/PreconditionNotMet.hpp>
#include <cpp_utils/Log.hpp>
#include <cpp_utils/macros/macros.hpp>
#include <cpp_utils/math/math_extension.hpp>
#include <cpp_utils/utils.hpp>

// These libraries are used to execute wildcard using system functions, and depend on the OS
#if _EPROSIMA_WINDOWS_PLATFORM
#include "Shlwapi.h"
#else
#include <fnmatch.h>
#endif // if _EPROSIMA_WINDOWS_PLATFORM

// Includes to use access method. It checks if a file exists and it is readable
#if _EPROSIMA_WINDOWS_PLATFORM
#include <io.h>         // Use _access windows method
#define access _access  // Allow using same method for UNIX and windows
#else
#include <unistd.h>
#endif // if _EPROSIMA_WINDOWS_PLATFORM

namespace eprosima {
namespace utils {

//! Overloaded '|' operator for composing FileAccessMode values.
FileAccessMode operator |(
        FileAccessMode mode_a,
        FileAccessMode mode_b)
{
    return static_cast<FileAccessMode>(static_cast<FileAccessModeType>(mode_a) |
           static_cast<FileAccessModeType>(mode_b));
}

//! Overloaded '&' operator for matching FileAccessMode values.
FileAccessMode operator &(
        FileAccessMode mode_a,
        FileAccessMode mode_b)
{
    return static_cast<FileAccessMode>(static_cast<FileAccessModeType>(mode_a) &
           static_cast<FileAccessModeType>(mode_b));
}

bool match_pattern(
        const std::string& pattern,
        const std::string& str) noexcept
{
#if defined(_WIN32)
    // Windows implementation
    return PathMatchSpec(str.c_str(), pattern.c_str());
#else
    // Posix implementation
    return (fnmatch(pattern.c_str(), str.c_str(), FNM_NOESCAPE) == 0);
#endif // defined(_WIN32)
}

void to_lowercase(
        std::string& st) noexcept
{
    std::transform(st.begin(), st.end(), st.begin(),
            [](unsigned char c)
            {
                return std::tolower(c);
            });
}

void to_uppercase(
        std::string& st) noexcept
{
    std::transform(st.begin(), st.end(), st.begin(),
            [](unsigned char c)
            {
                return std::toupper(c);
            });
}

std::uint64_t to_bytes(
        const std::string& input)
{
    static const std::map<std::string, std::uint64_t> units = {
        {"B", 1},
        {"KB", 1000},
        {"MB", 1000 * 1000},
        {"GB", 1000 * 1000 * 1000},
        {"TB", 1000ULL * 1000 * 1000 * 1000},
        {"PB", 1000ULL * 1000 * 1000 * 1000 * 1000},
        {"KIB", 1024},
        {"MIB", 1024 * 1024},
        {"GIB", 1024 * 1024 * 1024},
        {"TIB", 1024ULL * 1024 * 1024 * 1024},
        {"PIB", 1024ULL * 1024 * 1024 * 1024 * 1024}
    };

    // Find the number and the unit
    std::regex pattern("^(\\d+)\\s*([a-zA-Z]+)$");
    std::smatch matches;

    if (!std::regex_match(input, matches, pattern) || matches.size() != 3)
    {
        throw std::invalid_argument(
                  "The quantity is not in the expected format. It should be a natural number followed by a unit (e.g. 10MB).");
    }

    // Extract the number
    std::string number_str = matches[1].str();
    double number = std::stod(number_str);

    // Extract the unit
    std::string unit_str = matches[2].str();
    to_uppercase(unit_str);

    if (units.find(unit_str) == units.end())
    {
        throw std::invalid_argument(
                  "The unit is not valid. The valid units are: B, KB, MB, GB, TB, PB, KiB, MiB, GiB, TiB, PiB.");
    }

    const auto unit = units.at(unit_str);

    // Check whether the product of number * unit overflows
    // @note: The parentheses are necessary to distinguish std::max from the max macro (Windows).
    if (number > (std::numeric_limits<std::uint64_t>::max)() / unit)
    {
        throw std::invalid_argument("The number is too large to be converted to bytes.");
    }

    // The explicit cast to uint64_t is safe since the number has already been checked to fit.
    // The product is also safe since the possible overflow has also been checked.
    const std::uint64_t bytes = static_cast<std::uint64_t>(number) * unit;

    return bytes;
}

void tsnh(
        const Formatter& formatter)
{
    logError(UTILS_TSNH, "This Should Not Have Happened: " << formatter.to_string());
    Log::Flush();

    abort();
}

bool is_file_accessible(
        const char* file_path,
        FileAccessMode access_mode) noexcept
{
#if defined(_WIN32)
    if ((FileAccessMode::exec& access_mode) == FileAccessMode::exec)
    {
        logWarning(
            UTILS_UTILS,
            "Windows does not allow to check execution permission for file.");
        // Take out the FileAccessMode::exec bit
        access_mode =
                static_cast<FileAccessMode>(static_cast<FileAccessModeType>(access_mode) &
                ~static_cast<FileAccessModeType>(FileAccessMode::exec));
    }
#endif // if defined(_WIN32)
    return access(file_path, static_cast<FileAccessModeType>(access_mode)) != -1;
}

bool replace_first(
        std::string& st,
        std::string const& to_replace,
        std::string const& replace_by)
{
    std::size_t pos = st.find(to_replace);
    if (pos == std::string::npos)
    {
        return false;
    }
    else
    {
        st.replace(pos, to_replace.length(), replace_by);
        return true;
    }
}

unsigned int replace_all(
        std::string& st,
        std::string const& to_replace,
        std::string const& replace_by)
{
    unsigned int replacements = 0;
    while (replace_first(st, to_replace, replace_by))
    {
        replacements++;
    }
    return replacements;
}

unsigned int strip_str(
        std::string& to_strip,
        const std::string& replace_by /* = "" */,
        const std::set<std::string>& undesired_strings /* = {"\r"} */)
{
    unsigned int replacements = 0;
    for (const auto& undesired : undesired_strings)
    {
        replacements += replace_all(to_strip, undesired, replace_by);
    }
    return replacements;
}

std::string number_trailing_zeros_format(
        int value_to_print,
        unsigned int n_chars,
        bool allow_more_chars /* = true */)
{
    if (!allow_more_chars)
    {
        if (value_to_print / fast_exponential(10, n_chars) > 0)
        {
            throw PreconditionNotMet(STR_ENTRY
                          << "Number <" << value_to_print << ">"
                          << " has more than <" << n_chars << "> chars");
        }
    }

    std::ostringstream os;
    os << std::setw(n_chars) << std::setfill('0') << value_to_print;
    return os.str();
}

std::vector<std::string> split_string(
        const std::string& source,
        const std::set<std::string>& delimiters)
{
    std::vector<std::string> result = {source};
    for (const auto& delimiter : delimiters)
    {
        result = split_string(result, delimiter);
    }
    return result;
}

std::vector<std::string> split_string(
        const std::vector<std::string>& source,
        const std::string& delimiter)
{
    std::vector<std::string> result;
    for (const auto& source_str : source)
    {
        auto next_res = split_string(source_str, delimiter);
        result.insert(result.end(), next_res.begin(), next_res.end());
    }
    return result;
}

std::vector<std::string> split_string(
        const std::string& source,
        const std::string& delimiter)
{
    std::vector<std::string> result;
    int start = 0;
    int end = source.find(delimiter);
    while (end != -1)
    {
        result.push_back(source.substr(start, end - start));
        start = end + delimiter.size();
        end = source.find(delimiter, start);
    }
    result.push_back(source.substr(start));
    return result;
}

// FROM: https://gist.github.com/williamdes/308b95ac9ef1ee89ae0143529c361d37
std::string base64_encode(
        const std::string& in)
{
    std::string out;

    int val = 0, valb = -6;
    for (unsigned char c : in)
    {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0)
        {
            out.push_back(base64_alphabet[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
    {
        out.push_back(base64_alphabet[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (out.size() % 4)
    {
        out.push_back('=');
    }
    return out;
}

// FROM: https://gist.github.com/williamdes/308b95ac9ef1ee89ae0143529c361d37
std::string base64_decode(
        const std::string& in)
{
    std::string out;

    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++)
    {
        T[base64_alphabet[i]] = i;
    }

    int val = 0, valb = -8;
    for (unsigned char c : in)
    {
        if (T[c] == -1)
        {
            break;
        }
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0)
        {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

} /* namespace utils */
} /* namespace eprosima */
