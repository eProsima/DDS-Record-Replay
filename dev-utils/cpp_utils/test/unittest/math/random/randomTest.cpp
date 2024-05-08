// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <array>
#include <algorithm>
#include <cstdlib>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/math/random/RandomManager.hpp>

using namespace eprosima::utils;

namespace test {

constexpr const unsigned int TEST_ITERATIONS = 100;

} /* namespace test */

/*
 * These tests are divided in 2:
 * Trivial: check the correct working of the API with very few examples and could not fail for randomness
 *   These exist for coverage propose.
 * Non Trivial: check that random generation works. However these could fail due to randomness.
 */

/**
 * Test Pure random generation.
 *
 * CASES:
 * - pure_rand
 * - rand<true>
 *
 * @attention this test could eventually fail. However, failing in just calculating 2 pairs of random numbers and
 * retrieving same value in a human live time range should be considered as this class method is not completely correct.
 */
TEST(randomTest, trivial_pure)
{
    // pure_rand
    {
        RandomManager manager;
        auto val_1 = manager.pure_rand();
        auto val_2 = manager.pure_rand();
        ASSERT_NE(val_1, val_2);
    }

    // rand<true>
    {
        RandomManager manager;
        auto val_1 = manager.rand<true>();
        auto val_2 = manager.rand<true>();
        ASSERT_NE(val_1, val_2);
    }
}

/**
 * Test Sequence random generation.
 *
 * CASES:
 * - sequence_rand
 * - rand<false>
 * - rand
 */
TEST(randomTest, trivial_sequence)
{
    for (unsigned int i = 0; i < test::TEST_ITERATIONS; i++)
    {
        RandomManager manager_1(i);
        RandomManager manager_2(i);

        // sequence_rand
        for (unsigned int j = 0; j < test::TEST_ITERATIONS; j++)
        {
            auto val_1 = manager_1.sequence_rand();
            auto val_2 = manager_2.sequence_rand();
            ASSERT_EQ(val_1, val_2);
        }

        // rand<false>
        for (unsigned int j = 0; j < test::TEST_ITERATIONS; j++)
        {
            auto val_1 = manager_1.rand<false>();
            auto val_2 = manager_2.rand<false>();
            ASSERT_EQ(val_1, val_2);
        }

        // rand
        for (unsigned int j = 0; j < test::TEST_ITERATIONS; j++)
        {
            auto val_1 = manager_1.rand();
            auto val_2 = manager_2.rand();
            ASSERT_EQ(val_1, val_2);
        }
    }
}

/**
 * Test changing the seed of managers already created.
 */
TEST(randomTest, trivial_sequence_with_seed)
{
    RandomManager manager_1(1);
    RandomManager manager_2(99);
    for (unsigned int i = 0; i < test::TEST_ITERATIONS; i++)
    {
        manager_1.seed(i);
        manager_2.seed(i);
        // sequence_rand
        for (unsigned int j = 0; j < test::TEST_ITERATIONS; j++)
        {
            auto val_1 = manager_1.sequence_rand();
            auto val_2 = manager_2.sequence_rand();
            ASSERT_EQ(val_1, val_2);
        }
    }
}

/**
 * Test seeded random generation.
 *
 * CASES:
 * - seeded_rand
 * - rand
 */
TEST(randomTest, trivial_seeded)
{
    RandomManager manager_1;
    RandomManager manager_2(33);

    for (unsigned int i = 0; i < test::TEST_ITERATIONS; i++)
    {
        // seeded_rand
        {
            auto val_1 = manager_1.seeded_rand(i);
            auto val_2 = manager_1.seeded_rand(i);
            auto val_3 = manager_2.seeded_rand(i);
            ASSERT_EQ(val_1, val_2);
            ASSERT_EQ(val_1, val_3);
        }

        // rand
        {
            auto val_1 = manager_1.rand(i);
            auto val_2 = manager_1.rand(i);
            auto val_3 = manager_2.rand(i);
            ASSERT_EQ(val_1, val_2);
            ASSERT_EQ(val_1, val_3);
        }
    }
}

/**
 * Test the generation of std random numbers from a default seed.
 *
 * This will use the 2 Random Managers to know the N first values that are expected are the same.
 *
 * STEPS:
 * - calculates the N first numbers from initial_manager
 * - create N Random Manager and generate N values that are equal the initial_manager N ones
 *
 * NOTE: this test could also be done by seeding rand to 1.
 */
TEST(randomTest, get_random_sequence_number)
{
    // calculates the N first numbers from rand()
    RandomManager initial_manager;
    std::array<RandomNumberType, test::TEST_ITERATIONS> rand_numbers;
    for (unsigned int i = 0; i < test::TEST_ITERATIONS; ++i)
    {
        rand_numbers[i] = initial_manager.rand();
    }

    // create N Random Manager and generate N values that are equal the N first ones
    for (unsigned int i = 0; i < test::TEST_ITERATIONS; ++i)
    {
        RandomManager manager;
        for (unsigned int j = 0; j < test::TEST_ITERATIONS; ++j)
        {
            ASSERT_EQ(manager.rand(), rand_numbers[j]) << "Manager " << i << " in iteration " << j;
        }
    }
}

/**
 * Test that the N first pure random numbers are not included in the first N values generated
 * by std::rand .
 *
 * STEPS:
 * - calculates the N first numbers from rand()
 * - create Random Manager and generate N values with method rand<true> that should not be in N first generated
 * - create Random Manager and generate N values with method pure_rand that should not be in N first generated
 */
TEST(randomTest, get_pure_random_number)
{
    // calculates the N first numbers from rand()
    std::array<RandomNumberType, test::TEST_ITERATIONS> rand_numbers;
    for (unsigned int i = 0; i < test::TEST_ITERATIONS; ++i)
    {
        rand_numbers[i] = rand();
    }

    // create Random Manager and generate N values with method rand<true> that should not be in N first generated
    std::array<RandomNumberType, test::TEST_ITERATIONS> rand_manager_numbers;
    {
        RandomManager manager;
        for (unsigned int i = 0; i < test::TEST_ITERATIONS; ++i)
        {
            // Generate new value
            auto new_value = manager.rand<true>();

            // Check that new value is not in array
            ASSERT_EQ(
                std::find(std::begin(rand_numbers), std::end(rand_numbers), new_value),
                std::end(rand_numbers));
            rand_manager_numbers[i] = new_value;
        }
    }

    // create Random Manager and generate N values with method pure_rand that should not be in N first generated,
    // neither in the Random Manager generated ones
    {
        RandomManager manager;
        for (unsigned int i = 0; i < test::TEST_ITERATIONS; ++i)
        {
            // Generate new value
            auto new_value = manager.pure_rand();

            // Check that new value is not in array
            ASSERT_EQ(
                std::find(std::begin(rand_numbers), std::end(rand_numbers), new_value),
                std::end(rand_numbers));

            // Check that new value is not in Random Manager generated array
            ASSERT_EQ(
                std::find(std::begin(rand_manager_numbers), std::end(rand_manager_numbers), new_value),
                std::end(rand_manager_numbers));
        }
    }
}

/**
 * Check that getting seed numbers are obtain different numbers for each seed, but the same for same seed.
 *
 * STEPS:
 * - Call rand(seed) N times, twice per seed to check value is the same, and store each unique value.
 * - Call rand(seed) N times, and check that only occurrence of value in previous array is in the same seed.
 */
TEST(randomTest, get_seed_random_number)
{
    std::array<RandomNumberType, test::TEST_ITERATIONS> seed_numbers;
    RandomManager manager;

    // Call rand(seed) N times, twice per seed to check value is the same, and store each unique value.
    for (unsigned int i = 0; i < test::TEST_ITERATIONS; ++i)
    {
        // Get new seeded value
        auto new_seed_value = manager.rand(i);

        // Check it always gives same value
        ASSERT_EQ(manager.seeded_rand(i), new_seed_value);

        // Store value
        seed_numbers[i] = new_seed_value;
    }

    // Call rand(seed) N times, and check that only occurrence of value in previous array is in the same seed.
    for (unsigned int i = 0; i < test::TEST_ITERATIONS; ++i)
    {
        // Get new seeded value
        auto new_seed_value = manager.rand(i);

        // Check that is the same as the stored value
        ASSERT_EQ(new_seed_value, seed_numbers[i]);

        // Check that any other value is different
        for (unsigned int j = 0; j < test::TEST_ITERATIONS; ++j)
        {
            if (i != j)
            {
                ASSERT_NE(new_seed_value, seed_numbers[j]) <<
                    "Value " << seed_numbers[j] << " seeded with " << j <<
                    " sames as " << new_seed_value << " seeded with " << i;
            }
            else
            {
                ASSERT_EQ(new_seed_value, seed_numbers[j]);
            }
        }
    }
}

/**
 * Test that setting a seed in a RandomManager, the sequence numbers generated are the same for same seed.
 * Test also that are different for different seeds.
 * Test that in an already used manager, setting the seed produces the same sequence.
 */
TEST(randomTest, set_initial_seed)
{
    // array to store random sequences and check that are not repeated with different seeds (initialized to 0).
    std::array<RandomNumberType, test::TEST_ITERATIONS> random_numbers;
    for (unsigned int i = 0; i < test::TEST_ITERATIONS; ++i)
    {
        random_numbers[i] = 0;
    }

    // Test that setting a seed in a RandomManager, the sequence numbers generated are the same for same seed.
    // Test also that are different for different seeds.
    RandomManager manager;

    for (unsigned int i = 0; i < test::TEST_ITERATIONS; ++i)
    {
        // Create a random generator with seed and check that N first sequence values are not the same as other seed
        // Also replace the old values to check in next iteration
        manager.seed(i);
        for (unsigned int j = 0; j < test::TEST_ITERATIONS; ++j)
        {
            auto new_sequence_number = manager.rand();
            ASSERT_NE(new_sequence_number, random_numbers[j]) <<
                "In manager " << i << " in value " << j;
            random_numbers[j] = new_sequence_number;
        }

        // Create a different random generator and check that with same seed the sequence are the same
        RandomManager new_manager(i);
        for (unsigned int j = 0; j < test::TEST_ITERATIONS; ++j)
        {
            auto new_sequence_number = new_manager.sequence_rand();
            ASSERT_EQ(new_sequence_number, random_numbers[j]);
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
