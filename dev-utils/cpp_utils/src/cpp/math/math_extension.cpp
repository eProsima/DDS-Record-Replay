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
 * @file math_extension.cpp
 *
 */

#include <assert.h>

#include <cpp_utils/math/math_extension.hpp>

namespace eprosima {
namespace utils {

bool is_even(
        unsigned int number) noexcept
{
    return (number & 0x1) == 0;
}

bool is_power_of_2(
        unsigned int number) noexcept
{
    return number && (!(number & (number - 1)));
}

unsigned int fast_module(
        unsigned int dividend,
        unsigned int divisor) noexcept
{
    assert(divisor != 0);

    if (dividend < divisor)
    {
        // Optimize to 1 operation [if]
        return dividend;
    }
    else if (dividend == divisor)
    {
        // Optimize to 2 operation [if, if]
        return 0;
    }
    else if (divisor == 2)
    {
        // Optimize to 4 operations [if, if, if, and]
        return dividend & 1;
    }
    else if (is_power_of_2(divisor))
    {
        // Optimize to ~6 operations [if, if, if, if(and), and]
        return dividend & (divisor - 1);
    }
    else
    {
        // Not optimum
        return dividend % divisor;
    }
}

unsigned int fast_division(
        unsigned int dividend,
        unsigned int divisor) noexcept
{
    assert(divisor != 0);

    if (dividend < divisor)
    {
        // Optimize to 1 operation [if]
        return 0;
    }
    else if (dividend == divisor)
    {
        // Optimize to 2 operation [if, if]
        return 1;
    }
    else if (divisor == 1)
    {
        // Optimize to 3 operations [if, if, if]
        return dividend;
    }
    else if (divisor == 2)
    {
        // Optimize to 5 operations [if, if, if, if, swift]
        return (dividend >> 1);
    }
    else if (is_power_of_2(divisor))
    {
        while (divisor != 1)
        {
            dividend >>= 1;
            divisor >>= 1;
        }
        return dividend;
    }
    else
    {
        // Not optimum
        return dividend / divisor;
    }
}

unsigned int arithmetic_progression_sum(
        unsigned int lowest,
        unsigned int interval,
        unsigned int steps) noexcept
{
    return (((2 * lowest + ((steps - 1) * interval)) * steps) / 2);
}

unsigned int fast_exponential(
        unsigned int base,
        unsigned int exponent) noexcept
{
    if (exponent == 0)
    {
        return 1;
    }
    else if (exponent == 1)
    {
        return base;
    }
    else if (exponent % 2 == 0)
    {
        return fast_exponential(base * base, exponent / 2);
    }
    else
    {
        return base * fast_exponential(base, exponent - 1);
    }
}

} /* namespace utils */
} /* namespace eprosima */
