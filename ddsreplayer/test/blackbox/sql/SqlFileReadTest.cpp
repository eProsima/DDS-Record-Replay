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

#include <chrono>
#include <cstdint>
#include <thread>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <ddspipe_yaml/Yaml.hpp>

#include <ddsrecorder_yaml/replayer/YamlReaderConfiguration.hpp>

#include <tool/DdsReplayer.hpp>

#include "SqlFileReadTest.hpp"
#include "../PartitionSubscriber.hpp"
#include "../../resources/constants.hpp"

using eprosima::ddsrecorder::test::PartitionSubscriber;
using namespace eprosima;


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

/**
 * Verify that partition filtering follows every SQL message partition when a writer changes
 * partitions five times and returns to an earlier partition.
 */
TEST_F(SqlFileReadTest, partition_filter_follows_five_changes)
{
    Yaml yml;
    ddsrecorder::yaml::ReplayerConfiguration configuration(yml);
    configuration.replayer_configuration->domain.domain_id = test::DOMAIN;
    configuration.replayer_configuration->allowed_partition_list.insert("A");
    configuration.replay_types = true;

    const std::string input_file = "../../resources/recordings/basic/partitions_5_changes.db";
    PartitionSubscriber partition_a("A", "HelloWorldTopic", test::DOMAIN);
    ddsrecorder::replayer::DdsReplayer replayer(configuration, input_file);

    std::thread replay_thread([&replayer]()
            {
                replayer.process_file();
            });

    const bool partition_a_reader_ready = partition_a.wait_for_reader(std::chrono::seconds(5));
    replay_thread.join();
    replayer.stop();

    ASSERT_TRUE(partition_a_reader_ready);
    ASSERT_TRUE(partition_a.wait_for_messages(51, std::chrono::seconds(5)));

    const auto partition_a_indexes = partition_a.indexes();
    ASSERT_EQ(partition_a_indexes.size(), 51u);

    for (std::uint32_t i = 0; i < 45; ++i)
    {
        EXPECT_EQ(partition_a_indexes[i], i + 1);
    }
    for (std::uint32_t i = 0; i < 6; ++i)
    {
        EXPECT_EQ(partition_a_indexes[45 + i], 145 + i);
    }
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
