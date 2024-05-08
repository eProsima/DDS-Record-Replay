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
 * @file utils.hpp
 *
 * This file contains constant values common for the whole project
 */

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <cpp_utils/macros/macros.hpp>
#include <cpp_utils/Formatter.hpp>
#include <cpp_utils/library/library_dll.h>

namespace eprosima {
namespace utils {

using FileAccessModeType = int;

/**
   Enum for all possible access modes
 * Linux: See https://linux.die.net/man/2/access
 * Windows: See https://docs.microsoft.com/es-es/cpp/c-runtime-library/reference/access-waccess?view=msvc-170
 */
enum class FileAccessMode
{
    exist               = 0,
    read                = 4,
    write               = 2,
    exec                = 1,
    read_write          = read | write,
    read_exec           = read | exec,
    read_write_exec     = read | write | exec,
    write_exec          = write | exec,
};

//! Overloaded '|' operator for composing permissions.
FileAccessMode operator |(
        FileAccessMode mode_a,
        FileAccessMode mode_b);

//! Overloaded '&' operator for matching permissions.
FileAccessMode operator &(
        FileAccessMode mode_a,
        FileAccessMode mode_b);

//! Perform the wildcard matching using file comparison method
CPP_UTILS_DllAPI
bool match_pattern(
        const std::string& pattern,
        const std::string& str) noexcept;

/**
 * @brief Convert every alphabetic char in string to lower case
 *
 * @attention This function modifies the object given
 *
 * @param [in,out] st : string to modify
 */
CPP_UTILS_DllAPI
void to_lowercase(
        std::string& st) noexcept;

/**
 * @brief Convert every alphabetic char in string to upper case
 *
 * @attention This function modifies the object given
 *
 * @param [in,out] st : string to modify
 */
CPP_UTILS_DllAPI
void to_uppercase(
        std::string& st) noexcept;

/**
 * @brief Convert a string to a number of bytes.
 *
 * The string must be a number followed by a magnitude (e.g. 10MB, 0.5GiB).
 *
 * @param input string to convert
 * @return number of bytes
 */
CPP_UTILS_DllAPI
std::uint64_t to_bytes(
        const std::string& input);

template <typename T, bool Ptr = false>
std::ostream& element_to_stream(
        std::ostream& os,
        T element);

/**
 * @brief Concatenate serialization of elements in an array separated by \c separator .
 *
 * @tparam T type of each element. This object must have an << operator
 * @param os stream to store the concatenation result
 * @param list list of elements
 * @param separator char or string separator between elements
 * @return std::ostream& with the result stream concatenated
 */
template <typename T, bool Ptr = false>
std::ostream& container_to_stream(
        std::ostream& os,
        std::vector<T> list,
        std::string separator = ";");

//! Concatenate a set by converting to vector.
template <typename T, bool Ptr = false>
std::ostream& container_to_stream(
        std::ostream& os,
        std::set<T> list,
        std::string separator = ";");

template <typename T>
bool set_of_ptr_contains(
        const std::set<std::shared_ptr<T>> set,
        const std::shared_ptr<T> element);

template <typename T>
bool are_set_of_ptr_equal(
        const std::set<std::shared_ptr<T>> set1,
        const std::set<std::shared_ptr<T>> set2);

/**
 * @brief This Should Not Happen.
 *
 * This method should be called when something that should not have happened at all happens.
 * This will show an error log, assert false and throw an exception.
 *
 * Do not use this method when the error could come from user or output interaction, it should only be used
 * for inconsistency inside the program or C++ weird behaviours (e.g. enumeration values out of their range).
 *
 * @param formatter msg of the unexpected case.
 */
CPP_UTILS_DllAPI
void tsnh(
        const Formatter& formatter);

/**
 * @brief Convert a elements set into a shared ptr elements set.
 */
template <typename Parent, typename Child>
std::set<std::shared_ptr<Parent>> convert_set_to_shared(
        std::set<Child> set);

/**
 * @brief Whether a file exist and/or is accessible with specific permissions
 *
 * The accessibility could be checked for different permissions regarding argument \c access_mode ,
 * and each of them are asked by a different \c FileAccessMode :
 * - \c FileAccessMode::exist       : check if the file exist (any argument check this)
 * - \c FileAccessMode::read        : check if the file has read access
 * - \c FileAccessMode::write       : check if the file has write access
 * - \c FileAccessMode::exec        : check if the file has execution access
 *
 * This method could be used by OR \c FileAccessMode to check that file has all permissions in OR as:
 * access_mode = FileAccessMode::read_write -> check that file has read and write access permission.
 *
 * @warning windows does not retrieve information about the execution permission on a file.
 *
 * @param file_path path to the file to check
 * @param access_mode access permission(s) to check in the file
 * @return true if file is accessible regarding the permissions given
 * @return false otherwise
 */
CPP_UTILS_DllAPI
bool is_file_accessible(
        const char* file_path,
        FileAccessMode access_mode = FileAccessMode::exist) noexcept;

/**
 * @brief Common function for every method with \c operator<< to convert it to string.
 *
 * This function uses \c operator<< of class \c T to parse any object to a string.
 *
 * @tparam T type of the element to convert to string
 * @param element element to parse to string
 * @return to string convertion of the element
 */
template <typename T>
CPP_UTILS_DllAPI
std::string generic_to_string(
        const T& element);

template <typename T>
CPP_UTILS_DllAPI
void* copy_to_void_ptr(
        const T* source,
        size_t size = sizeof(T));

CPP_UTILS_DllAPI
bool replace_first(
        std::string& st,
        std::string const& to_replace,
        std::string const& replace_by);

CPP_UTILS_DllAPI
unsigned int replace_all(
        std::string& st,
        std::string const& to_replace,
        std::string const& replace_by);

/**
 * @brief Remove undesired substrings from string.
 *
 * Characters as \r or not UTF-8 could be removed or replaced by a string.
 * By default, it will replace "\n" and "\r" by an empty string (that means remove it).
 *
 * @param st [in, out] string to make changes
 * @param replace_by [in] string to replace the undesired chars
 * @param undesired_strings [in] set of undesired strings (if want to strip by char, just add chars separated)
 *
 * @return number of strings removed.
 */
CPP_UTILS_DllAPI
unsigned int strip_str(
        std::string& to_strip,
        const std::string& replace_by = "",
        const std::set<std::string>& undesired_strings = {"\n", "\r"});

CPP_UTILS_DllAPI
std::string number_trailing_zeros_format(
        int value_to_print,
        unsigned int n_chars,
        bool allow_more_chars = true);

/**
 * @brief Split string \c source by every delimiter in \c delimiters .
 *
 * This method uses split_string(const std::vector<std::string>&, const std::string&) .
 * For more information, please check that documentation.
 */
CPP_UTILS_DllAPI
std::vector<std::string> split_string(
        const std::string& source,
        const std::set<std::string>& delimiters);

/**
 * @brief Split each string in \c source by \c delimiter .
 *
 * This method uses split_string(const std::string&, const std::string&) .
 * For more information, please check that documentation.
 */
CPP_UTILS_DllAPI
std::vector<std::string> split_string(
        const std::vector<std::string>& source,
        const std::string& delimiter);

/**
 * @brief Split a string \c source by \c delimiter .
 *
 * The delimiter will no longer exist in any of the result strings.
 *
 * @example source:"Some String" delimiter:" " result:["Some", "String"]
 * @example source:" " delimiter:" " result:["", ""]
 *
 * @param source string to divide in tokens
 * @param delimiter string to look for in source and divide it with
 *
 * @return vector with strings
 *
 * @warning Some results could be empty strings if the delimiter is at the start, end or repeated along source.
 *
 * @post There will be always at least 1 element in the result vector
 */
CPP_UTILS_DllAPI
std::vector<std::string> split_string(
        const std::string& source,
        const std::string& delimiter);

/**
 * @brief Get std::map keys.
 *
 * Obtain the set of keys relative to a std::map.
 *
 * @param map [in] map whose keys are returned
 *
 * @return map keys
 */
template <typename Key, typename Value>
CPP_UTILS_DllAPI
std::set<Key> get_keys(
        const std::map<Key, Value>& map);

/**
 * @brief Get std::unordered_map keys.
 *
 * Obtain the set of keys relative to a std::unordered_map.
 *
 * @param map [in] map whose keys are returned
 *
 * @return map keys
 */
template <typename Key, typename Value>
CPP_UTILS_DllAPI
std::set<Key> get_keys(
        const std::unordered_map<Key, Value>& map);

/**
 * @brief Encode string using base64.
 *
 * @param in [in] string to encode
 *
 * @return string in base64
 */
CPP_UTILS_DllAPI
std::string base64_encode(
        const std::string& in);

/**
 * @brief Decode base64 string.
 *
 * @param in [in] string in base64 to decode
 *
 * @return decoded string
 */
CPP_UTILS_DllAPI
std::string base64_decode(
        const std::string& in);

//! Set of characters used in base64 encoding/decoding algorithms
const std::string base64_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

} /* namespace utils */
} /* namespace eprosima */

// Include implementation template file
#include <cpp_utils/impl/utils.ipp>
