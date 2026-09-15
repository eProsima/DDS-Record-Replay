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

#include <chrono>
#include <cstdint>
#include <string>
#include <thread>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <ddspipe_yaml/Yaml.hpp>

#include <ddsrecorder_yaml/replayer/YamlReaderConfiguration.hpp>

#include <tool/DdsReplayer.hpp>

#include "../../resources/constants.hpp"
#include "../PartitionSubscriber.hpp"

using namespace eprosima;

using eprosima::ddsrecorder::test::PartitionSubscriber;

namespace {

constexpr unsigned int MESSAGES_PER_PARTITION = 50;

} // namespace

/**
 * Verify that the replayer changes the output partition when the MCAP metadata changes for the
 * same writer GUID.
 *
 * CASES:
 *  - Verify that the first 50 samples are received in partition A.
 *  - Verify that the following 50 samples are received in partition B.
 */
TEST(McapPartitionFileReadTest, partition_switch)
{
    Yaml yml;
    ddsrecorder::yaml::ReplayerConfiguration configuration(yml);
    configuration.replayer_configuration->domain.domain_id = test::DOMAIN;
    configuration.replay_types = true;

    const std::string input_file = "../../resources/recordings/basic/one_writer_change_partition.mcap";
    PartitionSubscriber partition_a("A", "HelloWorldTopic", test::DOMAIN);
    PartitionSubscriber partition_b("B", "HelloWorldTopic", test::DOMAIN);
    ddsrecorder::replayer::DdsReplayer replayer(configuration, input_file);

    std::thread replay_thread([&replayer]()
            {
                replayer.process_file();
            });

    const bool partition_a_reader_ready = partition_a.wait_for_reader(std::chrono::seconds(5));
    const bool partition_b_reader_ready = partition_b.wait_for_reader(std::chrono::seconds(5));
    replay_thread.join();
    replayer.stop();

    ASSERT_TRUE(partition_a_reader_ready);
    ASSERT_TRUE(partition_b_reader_ready);
    ASSERT_TRUE(partition_a.wait_for_messages(
                MESSAGES_PER_PARTITION, std::chrono::seconds(5)));
    ASSERT_TRUE(partition_b.wait_for_messages(
                MESSAGES_PER_PARTITION, std::chrono::seconds(5)));

    const auto partition_a_indexes = partition_a.indexes();
    const auto partition_b_indexes = partition_b.indexes();
    ASSERT_EQ(partition_a_indexes.size(), MESSAGES_PER_PARTITION);
    ASSERT_EQ(partition_b_indexes.size(), MESSAGES_PER_PARTITION);

    for (unsigned int i = 0; i < MESSAGES_PER_PARTITION; ++i)
    {
        EXPECT_EQ(partition_a_indexes[i], i + 1);
        EXPECT_EQ(partition_b_indexes[i], MESSAGES_PER_PARTITION + i + 1);
    }
}

/**
 * Verify that partition filtering follows the channel version associated with each message when a
 * writer changes partitions during recording.
 */
TEST(McapPartitionFileReadTest, partition_filter_follows_switch)
{
    Yaml yml;
    ddsrecorder::yaml::ReplayerConfiguration configuration(yml);
    configuration.replayer_configuration->domain.domain_id = test::DOMAIN;
    configuration.replayer_configuration->allowed_partition_list.insert("A");
    configuration.replay_types = true;

    const std::string input_file = "../../resources/recordings/basic/one_writer_change_partition.mcap";
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
    ASSERT_TRUE(partition_a.wait_for_messages(
                MESSAGES_PER_PARTITION, std::chrono::seconds(5)));

    const auto partition_a_indexes = partition_a.indexes();
    ASSERT_EQ(partition_a_indexes.size(), MESSAGES_PER_PARTITION);

    for (unsigned int i = 0; i < MESSAGES_PER_PARTITION; ++i)
    {
        EXPECT_EQ(partition_a_indexes[i], i + 1);
    }
}

/**
 * Verify that partition filtering follows every channel version when a writer changes partitions
 * five times and returns to an earlier partition.
 */
TEST(McapPartitionFileReadTest, partition_filter_follows_five_changes)
{
    Yaml yml;
    ddsrecorder::yaml::ReplayerConfiguration configuration(yml);
    configuration.replayer_configuration->domain.domain_id = test::DOMAIN;
    configuration.replayer_configuration->allowed_partition_list.insert("A");
    configuration.replay_types = true;

    const std::string input_file = "../../resources/recordings/basic/partitions_5_changes.mcap";
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
