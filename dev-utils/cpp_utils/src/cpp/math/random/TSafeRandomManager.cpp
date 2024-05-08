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
 * @file TSafeRandomManager.cpp
 *
 */

#include <cstdlib>
#include <mutex>

#include <cpp_utils/math/random/TSafeRandomManager.hpp>

namespace eprosima {
namespace utils {

TSafeRandomManager::TSafeRandomManager(
        const RandomSeedType& original_seed /* = 1 */ )
    : std_random_generator_(original_seed)
{
    // Do nothing
}

RandomNumberType TSafeRandomManager::pure_rand () noexcept
{
    return pure_random_generator_();
}

RandomNumberType TSafeRandomManager::sequence_rand () noexcept
{
    std::lock_guard<Atomicable<std::mt19937>> guard(std_random_generator_);
    return std_random_generator_();
}

void TSafeRandomManager::seed (
        const RandomSeedType& seed) noexcept
{
    std::lock_guard<Atomicable<std::mt19937>> guard(std_random_generator_);
    std_random_generator_.seed(seed);
}

RandomNumberType TSafeRandomManager::seeded_rand (
        const RandomSeedType& seed) noexcept
{
    std::lock_guard<Atomicable<std::mt19937>> guard(seed_random_generator_);
    seed_random_generator_.seed(seed);
    return seed_random_generator_();
}

template <>
CPP_UTILS_DllAPI RandomNumberType TSafeRandomManager::rand<true> () noexcept
{
    return pure_rand();
}

template <>
CPP_UTILS_DllAPI RandomNumberType TSafeRandomManager::rand<false> () noexcept
{
    return sequence_rand();
}

RandomNumberType TSafeRandomManager::rand (
        const RandomSeedType& seed) noexcept
{
    return seeded_rand(seed);
}

} /* namespace utils */
} /* namespace eprosima */
