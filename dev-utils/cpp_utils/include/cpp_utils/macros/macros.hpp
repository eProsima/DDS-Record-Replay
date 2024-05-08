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
 * @file macros.hpp
 *
 * This file contains constant values common for the whole project
 */

#pragma once

#include <type_traits>

namespace eprosima {
namespace utils {

/////////////////////////
// FORMAT
/////////////////////////

/**
 * @brief Get string of the argument passed to the macro
 *
 * @example
 * STRINGIFY(value) = "value"
 */
#define STRINGIFY(x) #x

//! Same as \c STRINGIFY but adding a comma "," at the end
#define STRINGIFY_WITH_COMMA(x) #x,

#define COMMA ,

#define CONCATENATE(x, y) x ## y

#define CONCATENATE_COMMA(x) x,

/////////////////////////
// TYPES
/////////////////////////

#define ARE_SAME_TYPE(a, b) (typeid(a) == typeid(b))

#define IS_SAME_TYPE_AS_THIS(a) (ARE_SAME_TYPE(a, this))

/**
 * @brief Force the specialization type of a template to be a subclass of a Class.
 *
 * @example
 * FORCE_TEMPLATE_SUBCLASS(A, B)  =  static assert  <=>  B not inherit from A
 *
 * @param base parent class that \c derived must inherit.
 * @param derived specialization class.
 */
#define FORCE_TEMPLATE_SUBCLASS(base, derived) \
    static_assert(std::is_base_of<base, derived>::value, STRINGIFY(derived) " class not derived from " STRINGIFY(base))

/**
 * @brief Get string of the name of the CPP Data Type of the argument
 *
 * @example
 * STRINGIFY(int) = "j"
 * STRINGIFY(string) = "NSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE"
 */
#define TYPE_NAME(x) typeid(x).name()

#define MAX(x, y) x < y ? y : x
#define MIN(x, y) x > y ? y : x

/**
 * @brief Standarize a common way to check if the OS is windows.
 */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || \
    defined(WIN64) || defined(_WIN64) || defined(__WIN64) || \
    defined(_MSC_VER)
#define _EPROSIMA_WINDOWS_PLATFORM 1
#endif // if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(WIN64) || defined(_WIN64) || defined(__WIN64) || defined(_MSC_VER)

} /* namespace utils */
} /* namespace eprosima */
