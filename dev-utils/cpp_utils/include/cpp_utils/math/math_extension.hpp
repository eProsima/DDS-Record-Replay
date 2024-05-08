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
 * @file math.hpp
 *
 * This file contains math generic functions
 */

#pragma once

#include <stdint.h>

#include <cpp_utils/library/library_dll.h>

namespace eprosima {
namespace utils {

/**
 * @brief Optimize % 2 operation
 *
 * @param number to calculate whether it is even
 *
 * @return whether \c number is even
 */
CPP_UTILS_DllAPI bool is_even(
        unsigned int number) noexcept;

/**
 * @brief Calculate whether the argument is a power of 2 value.
 *
 * is_power_of_2(x) <=> E(n) : 2^n = x
 *
 * @param number to calculate whether it is a power of 2
 *
 * @return whether \c number is power of 2
 */
CPP_UTILS_DllAPI bool is_power_of_2(
        unsigned int number) noexcept;

/**
 * @brief Module (%) operation with performance optimization
 *
 * This function optimizes the % operation, that executes a division, by optimizing these cases:
 * - If the dividend is smaller than the divisor, the result is the dividend
 * - If the dividend is equal than the divisor, the result is 0
 * - If the divisor is 2, the result is the dividend % 2 calculated by a logic AND operation
 * - If the divisor is a power of 2, the result is calculated by a logic AND operation
 * - Otherwise uses % operation
 *
 * @param dividend Dividend
 * @param divisor Divisor (must be greater than 0 so the operation make sense)
 *
 * @pre \c divisor must not be 0
 *
 * @return The result of the operation %
 *
 * @attention Do only use this function with non literal values. Literal values are optimized by compiler.
 */
CPP_UTILS_DllAPI unsigned int fast_module(
        unsigned int dividend,
        unsigned int divisor) noexcept;

/**
 * @brief Integer Division (/) operation with performance optimization
 *
 * This function optimizes the / operation by optimizing these cases:
 * - If \c dividend is smaller or equal than the \c, the result is \c dividend
 * - If the \c is 2, the result is \c dividend % 2 calculated by a logic AND operation
 * - If the \c is a power of 2, the result is calculated by a logic AND operation
 * - Otherwise uses / operation
 *
 * @param dividend Dividend
 * @param divisor Divisor
 *
 * @pre \c divisor must not be 0
 *
 * @return The result of the operation /
 *
 * @attention Do only use this function with non literal values. Literal values are optimized by compiler.
 */
CPP_UTILS_DllAPI unsigned int fast_division(
        unsigned int dividend,
        unsigned int divisor) noexcept;

/**
 * @brief Calculate the sum of an arithmetic progression from an initial to a final number.
 *
 * This function uses the fast operation to calculate an arithmetic sum:
 * S = ((a1 + an) / 2) * (n)
 *
 * @pre \c interval must be greater than 0
 * @pre \c steps must be greater than 0
 *
 * @param lowest lowest element of the arithmetic progression
 * @param interval interval between two elements of the arithmetic progression
 * @param steps number of steps of the arithmetic progression
 *
 * @return The result of the sum
 *
 * EXAMPLE OF USE
 *   1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 = arithmetic_progression_sum(1, 1, 10)
 *   0 + 2 + 4 + 6 + 8 = arithmetic_progression_sum(0, 2, 5)
 */
CPP_UTILS_DllAPI unsigned int arithmetic_progression_sum(
        unsigned int lowest,
        unsigned int interval,
        unsigned int steps) noexcept;

/**
 * @brief Calculate power of integers in a fast way
 *
 * calculate a^b or a**b between 2 integers
 *
 * @param base base of the exponential (a)
 * @param exponent exponent (b)
 *
 * @return a multiplied b times.
 *
 * @attention Overflow is not handled.
 */
CPP_UTILS_DllAPI unsigned int fast_exponential(
        unsigned int base,
        unsigned int exponent) noexcept;

} /* namespace utils */
} /* namespace eprosima */
