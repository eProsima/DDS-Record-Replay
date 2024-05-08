// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/macros/macros.hpp>
#include <cpp_utils/macros/custom_enumeration.hpp>
#include <cpp_utils/enum/EnumBuilder.hpp>
#include <cpp_utils/types/Singleton.hpp>

namespace test {

/***************************
 * Type_simple
 *
 * Test type with only 2 values and 2 strings to convert to value.
 */
ENUMERATION_BUILDER(
    Type_simple,
    value_1,
    value_2
    );

eProsima_ENUMERATION_BUILDER(
    Type_simple_Builder,
    Type_simple,
    {
        { Type_simple::value_1 COMMA { "value_1" } } COMMA
        { Type_simple::value_2 COMMA { "value_2" } }
    }
    );

/***************************
 * Type_complex
 *
 * Test type with only 2 values and 2 strings to convert to value.
 */
ENUMERATION_BUILDER(
    Type_complex,
    value_1,
    value_other,
    ouiii
    );

eProsima_ENUMERATION_BUILDER(
    Type_complex_Builder,
    Type_complex,
    {
        { Type_complex::value_1 COMMA { "value" } } COMMA
        { Type_complex::value_other COMMA { "other" COMMA "and_other" COMMA "let's try this"} } COMMA
        { Type_complex::ouiii COMMA { "1" COMMA "2" COMMA "3" COMMA "invalid_value" } }
    }
    );

} // namespace test

using namespace eprosima::utils;

/**
 * Test to create an Enumeration builder in site and use it to access values from simple enumeration.
 *
 * CASES:
 * - invalid str
 * - value 1 str
 * - value 2 str
 */
TEST(EnumBuilderTest, test_get_secure_simple)
{
    // Create builder
    EnumBuilder<test::Type_simple> builder(
    {
        { test::Type_simple::value_1, { "value_1" } },
        { test::Type_simple::value_2, { "value_2" } }
    }
        );

    // invalid str
    {
        test::Type_simple enum_value;
        bool ret_code = builder.string_to_enumeration("invalid_value", enum_value);

        ASSERT_FALSE(ret_code);
    }

    // value 1 str
    {
        test::Type_simple enum_value;
        bool ret_code = builder.string_to_enumeration("value_1", enum_value);

        ASSERT_TRUE(ret_code);
        ASSERT_EQ(enum_value, test::Type_simple::value_1);
    }

    // value 2 str
    {
        test::Type_simple enum_value;
        bool ret_code = builder.string_to_enumeration("value_2", enum_value);

        ASSERT_TRUE(ret_code);
        ASSERT_EQ(enum_value, test::Type_simple::value_2);
    }
}

/**
 * Test to create an Enumeration builder in site and use it to access values from simple enumeration.
 *
 * CASES:
 * - invalid str
 * - value 1 str
 * - value 2 str
 */
TEST(EnumBuilderTest, test_get_non_secure_simple)
{
    // Create builder
    EnumBuilder<test::Type_simple> builder(
    {
        { test::Type_simple::value_1, { "value_1" } },
        { test::Type_simple::value_2, { "value_2" } }
    }
        );

    // invalid str
    {
        ASSERT_THROW(
            builder.string_to_enumeration("invalid_value"),
            ValueNotAllowedException);
    }

    // value 1 str
    {
        ASSERT_EQ(
            builder.string_to_enumeration("value_1"), test::Type_simple::value_1);
    }

    // value 2 str
    {
        ASSERT_EQ(
            builder.string_to_enumeration("value_2"), test::Type_simple::value_2);
    }
}

/**
 * Test to create the values from Type_simple enumeration using secure (no exception) function.
 *
 * STEPS:
 * - create builder
 * - invalid str
 * - value 1 str
 * - reinitialize
 * - invalid str
 * - value 1 str
 *
 * - use singleton class
 * - reinitialize singleton class
 * - use singleton class
 */
TEST(EnumBuilderTest, test_get_initialization)
{
    // Auxiliary variable
    test::Type_simple enum_value;

    // create empty builder
    EnumBuilder<test::Type_simple> builder(
    {
        { test::Type_simple::value_1, { "value_1" } },
        { test::Type_simple::value_2, { "value_2" } }
    }
        );

    // invalid str
    {
        ASSERT_FALSE(
            builder.string_to_enumeration("invalid_value", enum_value));
    }

    // value 1 str
    {
        ASSERT_TRUE(
            builder.string_to_enumeration("value_1", enum_value));
    }

    // reinitialize
    builder.refactor_values(
    {
        { test::Type_simple::value_1, { "invalid_value" } }
    });

    // invalid str
    {
        ASSERT_FALSE(
            builder.string_to_enumeration("value_1", enum_value));
        ASSERT_FALSE(
            builder.string_to_enumeration("value_2", enum_value));
    }

    // value 1 str
    {
        ASSERT_TRUE(
            builder.string_to_enumeration("invalid_value", enum_value));
        ASSERT_EQ(enum_value, test::Type_simple::value_1);
    }

    auto singleton_ref = test::Type_simple_Builder::get_instance();
    // use singleton class
    {
        ASSERT_TRUE(
            singleton_ref->string_to_enumeration("value_1", enum_value));
        ASSERT_EQ(enum_value, test::Type_simple::value_1);

        ASSERT_FALSE(
            singleton_ref->string_to_enumeration("invalid_value", enum_value));
    }

    // reinitialize singleton class
    {
        singleton_ref->refactor_values(
        {
            { test::Type_simple::value_1, { "invalid_value" } }
        });
    }

    // use singleton class
    {
        ASSERT_TRUE(
            singleton_ref->string_to_enumeration("invalid_value", enum_value));
        ASSERT_EQ(enum_value, test::Type_simple::value_1);

        ASSERT_FALSE(
            singleton_ref->string_to_enumeration("value_1", enum_value));
    }
}

/**
 * Test to create the values from Type_complex enumeration using secure (no exception) function.
 *
 * CASES:
 * - invalid str
 * - value 1 str
 * - value other str
 * - value oui str
 */
TEST(EnumBuilderTest, test_singleton_complex)
{
    // invalid str
    {
        test::Type_complex enum_value;
        bool ret_code = test::Type_complex_Builder::get_instance()->string_to_enumeration("oui", enum_value);

        ASSERT_FALSE(ret_code);
    }

    // value 1 str
    {
        test::Type_complex enum_value;
        bool ret_code = test::Type_complex_Builder::get_instance()->string_to_enumeration("value", enum_value);

        ASSERT_TRUE(ret_code);
        ASSERT_EQ(enum_value, test::Type_complex::value_1);
    }

    // value other str
    {
        test::Type_complex enum_value;
        std::set<std::string> valid_strings = {"other", "and_other", "let's try this"};

        for (const auto& str : valid_strings)
        {
            bool ret_code = test::Type_complex_Builder::get_instance()->string_to_enumeration(str, enum_value);

            ASSERT_TRUE(ret_code);
            ASSERT_EQ(enum_value, test::Type_complex::value_other);
        }
    }

    // value oui str
    {
        test::Type_complex enum_value;
        std::set<std::string> valid_strings = {"1", "2", "3", "invalid_value"};

        for (const auto& str : valid_strings)
        {
            bool ret_code = test::Type_complex_Builder::get_instance()->string_to_enumeration(str, enum_value);

            ASSERT_TRUE(ret_code);
            ASSERT_EQ(enum_value, test::Type_complex::ouiii);
        }
    }
}

/**
 * Initialize a new singleton builder (with different index) in run time.
 *
 * STEPS:
 * - create new singleton builder object
 * - invalid str with new builder
 * - value 1 str with new builder
 * - value 1 str with default builder
 */
TEST(EnumBuilderTest, test_singleton_simple_other_builder)
{
    // Auxiliary variable
    test::Type_simple enum_value;

    // create new singleton builder object
    InitializableSingleton<EnumBuilder<test::Type_simple>, 66>::initialize<
        const std::map< test::Type_simple, std::set<std::string>>&>(
    {
        { test::Type_simple::value_1, { "some_string" } }
    }
        );
    auto singleton_ref = InitializableSingleton<EnumBuilder<test::Type_simple>, 66>::get_instance();

    // invalid str with new builder
    {
        bool ret_code = singleton_ref->string_to_enumeration("value_1", enum_value);

        ASSERT_FALSE(ret_code);
    }

    // value 1 str with new builder
    {
        bool ret_code = singleton_ref->string_to_enumeration("some_string", enum_value);

        ASSERT_TRUE(ret_code);
        ASSERT_EQ(enum_value, test::Type_simple::value_1);
    }

    // value 1 str with default builder
    {
        bool ret_code = test::Type_simple_Builder::get_instance()->string_to_enumeration("value_1", enum_value);

        ASSERT_TRUE(ret_code);
        ASSERT_EQ(enum_value, test::Type_simple::value_1);
    }
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
