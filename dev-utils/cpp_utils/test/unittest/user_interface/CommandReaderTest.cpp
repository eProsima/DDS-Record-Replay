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

#include <iostream>
#include <queue>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/enum/EnumBuilder.hpp>
#include <cpp_utils/macros/custom_enumeration.hpp>
#include <cpp_utils/user_interface/CommandReader.hpp>

namespace test {

//! Test enumeration with 2 values
ENUMERATION_BUILDER(
    Enum1,
    value_1,
    value_2
    );

//! Test EnumBuilder to create enums of type \c Enum1 .
eprosima::utils::EnumBuilder<Enum1> create_builder()
{
    return eprosima::utils::EnumBuilder<Enum1>(
        {
            {"value_1", Enum1::value_1},
            {"value_2", Enum1::value_2},
        }
        );
}

//! Test enumeration with 3 heterogenous values
ENUMERATION_BUILDER(
    Enum2,
    some_value,
    other,
    AND_THE_END
    );

//! Singleton EnumBuilder for \c Enum2
eProsima_ENUMERATION_BUILDER(
    Enum2_Builder,
    Enum2,
    {
        { Enum2::some_value COMMA { "1" } } COMMA
        { Enum2::AND_THE_END COMMA { "value_2" COMMA "andtheend" COMMA "and_the_end" } }
    }
    );

} /* namespace test */

using namespace eprosima::utils;

/**
 * Create a CommandReader and check it does not break or have memory leaks.
 */
TEST(CommandReaderTest, trivial_create)
{
    auto builder = test::create_builder();
    CommandReader<test::Enum1> reader(builder);
}

/**
 * Read 3 commands of type enum 1.
 */
TEST(CommandReaderTest, read_lines_enum_1)
{
    std::stringstream source;
    source << "value_1" << "\n";
    source << "value_1 arg" << "\n";
    source << "value_2 more than 1 arg" << "\n";

    auto builder = test::create_builder();
    CommandReader<test::Enum1> reader(
        builder,
        source);

    {
        Command<test::Enum1> command_result;
        ASSERT_TRUE(reader.read_next_command(command_result));
        ASSERT_EQ(command_result.command, test::Enum1::value_1);
        ASSERT_EQ(command_result.arguments.size(), 1);
        ASSERT_EQ(command_result.arguments[0], "value_1");
    }

    {
        Command<test::Enum1> command_result;
        ASSERT_TRUE(reader.read_next_command(command_result));
        ASSERT_EQ(command_result.command, test::Enum1::value_1);
        ASSERT_EQ(command_result.arguments.size(), 2);
        ASSERT_EQ(command_result.arguments[0], "value_1");
        ASSERT_EQ(command_result.arguments[1], "arg");
    }

    {
        Command<test::Enum1> command_result;
        ASSERT_TRUE(reader.read_next_command(command_result));
        ASSERT_EQ(command_result.command, test::Enum1::value_2);
        ASSERT_EQ(command_result.arguments.size(), 5);
        ASSERT_EQ(command_result.arguments[0], "value_2");
        ASSERT_EQ(command_result.arguments[1], "more");
        ASSERT_EQ(command_result.arguments[2], "than");
        ASSERT_EQ(command_result.arguments[3], "1");
        ASSERT_EQ(command_result.arguments[4], "arg");
    }
}

/**
 * Read a line that does not contains any allowed command value.
 */
TEST(CommandReaderTest, read_lines_enum_1_negative)
{
    std::stringstream source;
    source << "value_3" << "\n";

    auto builder = test::create_builder();
    CommandReader<test::Enum1> reader(
        builder,
        source);

    {
        Command<test::Enum1> command_result;
        ASSERT_FALSE(reader.read_next_command(command_result));
    }
}

/**
 * Read 2 commands using a singleton EnumBuilder
 */
TEST(CommandReaderTest, read_lines_enum_2_singleton)
{
    std::stringstream source;
    source << "andtheend" << "\n";
    source << "1 args" << "\n";

    CommandReader<test::Enum2> reader(
        *test::Enum2_Builder::get_shared_instance(),
        source);

    {
        Command<test::Enum2> command_result;
        ASSERT_TRUE(reader.read_next_command(command_result));
        ASSERT_EQ(command_result.command, test::Enum2::AND_THE_END);
        ASSERT_EQ(command_result.arguments.size(), 1);
        ASSERT_EQ(command_result.arguments[0], "andtheend");
    }

    {
        Command<test::Enum2> command_result;
        ASSERT_TRUE(reader.read_next_command(command_result));
        ASSERT_EQ(command_result.command, test::Enum2::some_value);
        ASSERT_EQ(command_result.arguments.size(), 2);
        ASSERT_EQ(command_result.arguments[0], "1");
        ASSERT_EQ(command_result.arguments[1], "args");
    }
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
