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

#include <gtest/gtest.h>

#include <cpp_utils/testing/gtest_aux.hpp>

#include "McapFileReadWithTypeTest.hpp"


TEST_F(McapFileReadWithTypeTest, trivial)
{
    trivial_test(input_file_type_, publish_type_);
}

TEST_F(McapFileReadWithTypeTest, dds_data_to_check)
{
    data_to_check_test(input_file_type_, publish_type_);
}

TEST_F(McapFileReadWithTypeTest, ros2_data_to_check)
{
    constexpr auto IS_ROS2_TOPIC = true;
    data_to_check_test(input_file_ros2_, publish_type_, IS_ROS2_TOPIC);
}

TEST_F(McapFileReadWithTypeTest, dds_more_playback_rate)
{
    more_playback_rate_test(input_file_type_, publish_type_);
}

TEST_F(McapFileReadWithTypeTest, ros2_more_playback_rate)
{
    constexpr auto IS_ROS2_TOPIC = true;
    more_playback_rate_test(input_file_ros2_, publish_type_, IS_ROS2_TOPIC);
}

TEST_F(McapFileReadWithTypeTest, dds_less_playback_rate)
{
    less_playback_rate_test(input_file_type_, publish_type_);
}

TEST_F(McapFileReadWithTypeTest, ros2_less_playback_rate)
{
    constexpr auto IS_ROS2_TOPIC = true;
    less_playback_rate_test(input_file_ros2_, publish_type_, IS_ROS2_TOPIC);
}

TEST_F(McapFileReadWithTypeTest, begin_time)
{
    begin_time_test(input_file_type_, publish_type_);
}

TEST_F(McapFileReadWithTypeTest, end_time)
{
    end_time_test(input_file_type_, publish_type_);
}

TEST_F(McapFileReadWithTypeTest, start_replay_time_earlier)
{
    start_replay_time_earlier_test(input_file_type_, publish_type_);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
