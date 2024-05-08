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

#include <algorithm>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/math/math_extension.hpp>

using namespace eprosima::utils;

namespace eprosima {
namespace utils {
namespace test {

constexpr const unsigned int NUMBERS_TO_TEST = 1000;
constexpr const unsigned int NUMBERS_TO_TEST_SHORT = 100;

bool compare_is_even(
        unsigned int number)
{
    return (
        is_even(number)
        ==
        (number % 2 == 0));
}

bool compare_is_power_of_2(
        unsigned int number)
{
    // Check power of 2 numbers until they are equal or greater number
    unsigned int x__ = 1;
    while (x__ < number)
    {
        x__ *= 2;
    }

    // If x == number it means number is a power of 2
    // Notice that 0 will not be power of 2, as expected
    bool it_is_actually_power_of_2 = (x__ == number);

    return (
        is_power_of_2(number)
        ==
        it_is_actually_power_of_2);
}

bool compare_fast_module(
        unsigned int dividend,
        unsigned int divisor)
{
    return (
        fast_module(dividend, divisor)
        ==
        dividend % divisor);
}

bool compare_fast_division(
        unsigned int dividend,
        unsigned int divisor)
{
    return (
        fast_division(dividend, divisor)
        ==
        dividend / divisor);
}

bool compare_arithmetic_progression_sum(
        unsigned int lowest,
        unsigned int interval,
        unsigned int steps)
{
    unsigned int current_number = lowest;
    unsigned int real_result = 0;
    for (unsigned int i = 0; i < steps; ++i)
    {
        real_result += current_number;
        current_number += interval;
    }

    return (
        arithmetic_progression_sum(lowest, interval, steps)
        ==
        real_result);
}

bool compare_fast_exponential(
        unsigned int base,
        unsigned int exponent)
{
    unsigned int result = 1;
    for (unsigned int i = 0; i < exponent; i++)
    {
        result *= base;
    }

    return (
        fast_exponential(base, exponent)
        ==
        result);
}

} /* namespace test */
} /* namespace utils */
} /* namespace eprosima */

/**
 * Test \c is_even method
 */
TEST(mathTest, is_even)
{
    // calculate module in many cases
    for (unsigned int number = 0; number < test::NUMBERS_TO_TEST; ++number)
    {
        ASSERT_TRUE(test::compare_is_even(number))
            << number;
    }
}

/**
 * Test \c is_even method
 */
TEST(mathTest, is_power_of_2)
{
    // calculate module in many cases
    for (unsigned int number = 0; number < test::NUMBERS_TO_TEST; ++number)
    {
        ASSERT_TRUE(test::compare_is_power_of_2(number))
            << number;
    }
}

/**
 * Test \c fast_module method
 */
TEST(mathTest, fast_module)
{
    // calculate module in many cases
    for (unsigned int dividend = 0; dividend < test::NUMBERS_TO_TEST; ++dividend)
    {
        for (unsigned int divisor = 1; divisor < test::NUMBERS_TO_TEST; ++divisor)
        {
            ASSERT_TRUE(test::compare_fast_module(dividend, divisor))
                << dividend << " % " << divisor;
        }
    }
}

/**
 * Test \c fast_division method
 */
TEST(mathTest, fast_division)
{
    // calculate module in many cases
    for (unsigned int dividend = 0; dividend < test::NUMBERS_TO_TEST; ++dividend)
    {
        for (unsigned int divisor = 1; divisor < test::NUMBERS_TO_TEST; ++divisor)
        {
            ASSERT_TRUE(test::compare_fast_division(dividend, divisor))
                << dividend << " % " << divisor;
        }
    }
}

/**
 * Test \c arithmetic_progression_sum method
 */
TEST(mathTest, arithmetic_progression_sum)
{
    // calculate module in many cases
    for (unsigned int lowest = 0; lowest < test::NUMBERS_TO_TEST_SHORT; ++lowest)
    {
        for (unsigned int interval = 1; interval < test::NUMBERS_TO_TEST_SHORT; ++interval)
        {
            for (unsigned int steps = 1; steps < test::NUMBERS_TO_TEST_SHORT; ++steps)
            {
                ASSERT_TRUE(test::compare_arithmetic_progression_sum(lowest, interval, steps))
                    << lowest << " , " << interval << " , " << steps;
            }
        }
    }
}

/**
 * Test \c fast_exponential method
 */
TEST(mathTest, fast_exponential)
{
    // calculate module in many cases
    for (unsigned int base = 0; base < test::NUMBERS_TO_TEST; ++base)
    {
        for (unsigned int exponent = 0; exponent < test::NUMBERS_TO_TEST; ++exponent)
        {
            ASSERT_TRUE(test::compare_fast_exponential(base, exponent))
                << base << " ** " << exponent;
        }
    }
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
