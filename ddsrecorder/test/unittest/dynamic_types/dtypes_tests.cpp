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

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/macros/custom_enumeration.hpp>
#include <cpp_utils/file/file_utils.hpp>

#include <fastrtps/types/DynamicTypePtr.h>

#include <recorder/dynamic_types/schema.hpp>
#include <recorder/dynamic_types/utils.hpp>

#include "types/all_types.hpp"

using namespace eprosima;

namespace test {

std::string read_msg_from_file_(const std::string& file_name)
{
    return utils::file_to_string(file_name.c_str());
}

std::string file_name_by_type(SupportedType type)
{
    return std::string("resources/") + to_string(type) + ".msg";
}

void compare_schemas(const std::string& schema1, const std::string& schema2)
{
    ASSERT_EQ(schema1, schema2);
}

void execute_test_by_type(SupportedType type)
{
    // Get msg file in string with the value expected to be generated in the schema
    std::string msg_file = read_msg_from_file_(file_name_by_type(type));

    // Get Dynamic type
    fastrtps::types::DynamicType_ptr dyn_type = get_dynamic_type(type);

    // Get schema generated
    std::string schema = ddsrecorder::core::recorder::generate_dyn_type_schema(dyn_type);

    // Compare schemas
    compare_schemas(msg_file, schema);
}

} // namespace test

class ParametrizedTests : public ::testing::TestWithParam<test::SupportedType>
{
public:
    void SetUp()
    {
        type_ = GetParam();
    }

    test::SupportedType type_;
};

/**
 * TODO
 */
TEST_P(ParametrizedTests, msg_schema_generation)
{
    test::execute_test_by_type(type_);
}

INSTANTIATE_TEST_SUITE_P(dtypes_tests, ParametrizedTests, ::testing::Values(
    test::SupportedType::hello_world,
    test::SupportedType::numeric_array,
    test::SupportedType::char_sequence,
    test::SupportedType::basic_struct,
    test::SupportedType::basic_array_struct,
    test::SupportedType::float_bounded_sequence,
    test::SupportedType::arrays_and_sequences,
    test::SupportedType::complex_nested_arrays
));

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
