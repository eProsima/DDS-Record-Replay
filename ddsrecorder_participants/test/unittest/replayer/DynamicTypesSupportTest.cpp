// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <string>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <ddsrecorder_participants/replayer/DynamicTypesSupport.hpp>

using namespace eprosima::ddsrecorder::participants;

/**
 * Dependency fragments must be recognizable by their key prefix.
 */
TEST(DynamicTypesSupportTest, dependency_keys_are_recognized)
{
    ASSERT_TRUE(is_dependency_type_key(std::string(DEPENDENCY_KEY_PREFIX) + "abcd1234"));
    ASSERT_TRUE(is_dependency_type_key(DEPENDENCY_KEY_PREFIX));
}

/**
 * Real DDS type names must never be mistaken for dependency fragments.
 *
 * The names below are the regression case for the old naming scheme, which stored a dependency of
 * type "Foo" under the synthetic name "Foo_0". A genuine type actually named "Foo_0" then collided
 * with it in the SQL Types table, whose primary key is the type name.
 */
TEST(DynamicTypesSupportTest, real_type_names_are_not_dependency_keys)
{
    ASSERT_FALSE(is_dependency_type_key("Foo"));
    ASSERT_FALSE(is_dependency_type_key("Foo_0"));
    ASSERT_FALSE(is_dependency_type_key("Inner"));
    ASSERT_FALSE(is_dependency_type_key("eprosima::fastdds::statistics::EntityCount_2"));
    ASSERT_FALSE(is_dependency_type_key(""));

    // A type name cannot contain '/', so it cannot reproduce the prefix even partially
    ASSERT_FALSE(is_dependency_type_key("__dep__"));
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
