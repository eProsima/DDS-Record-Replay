// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <gtest/gtest.h>

#include <cpp_utils/testing/gtest_aux.hpp>

#include "SqlFileReadTest.hpp"


TEST_F(SqlFileReadTest, trivial)
{
    trivial_test(input_file_);
}

TEST_F(SqlFileReadTest, data_to_check)
{
    data_to_check_test(input_file_);
}

TEST_F(SqlFileReadTest, more_playback_rate)
{
    more_playback_rate_test(input_file_);
}

TEST_F(SqlFileReadTest, less_playback_rate)
{
    less_playback_rate_test(input_file_);
}

TEST_F(SqlFileReadTest, begin_time)
{
    begin_time_test(input_file_);
}

TEST_F(SqlFileReadTest, end_time)
{
    end_time_test(input_file_);
}

TEST_F(SqlFileReadTest, start_replay_time_earlier)
{
    start_replay_time_earlier_test(input_file_);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
