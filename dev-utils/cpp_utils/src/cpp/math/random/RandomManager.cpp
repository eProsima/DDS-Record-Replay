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

/**
 * @file RandomManager.cpp
 *
 */

#include <cstdlib>

#include <cpp_utils/math/random/RandomManager.hpp>

namespace eprosima {
namespace utils {

RandomManager::RandomManager(
        const RandomSeedType& original_seed /* = 1 */ )
    : std_random_generator_(original_seed)
{
    // Do nothing
}

RandomNumberType RandomManager::pure_rand () noexcept
{
    return pure_random_generator_();
}

RandomNumberType RandomManager::sequence_rand () noexcept
{
    return std_random_generator_();
}

void RandomManager::seed (
        const RandomSeedType& seed) noexcept
{
    std_random_generator_.seed(seed);
}

RandomNumberType RandomManager::seeded_rand (
        const RandomSeedType& seed) noexcept
{
    seed_random_generator_.seed(seed);
    return seed_random_generator_();
}

template <>
CPP_UTILS_DllAPI RandomNumberType RandomManager::rand<true> () noexcept
{
    return pure_rand();
}

template <>
CPP_UTILS_DllAPI RandomNumberType RandomManager::rand<false> () noexcept
{
    return sequence_rand();
}

RandomNumberType RandomManager::rand (
        const RandomSeedType& seed) noexcept
{
    return seeded_rand(seed);
}

} /* namespace utils */
} /* namespace eprosima */
