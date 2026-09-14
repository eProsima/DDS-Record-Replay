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

/**
 * @file McapWriterTest.cpp
 */

#include <filesystem>
#include <iterator>
#include <map>
#include <memory>
#include <string>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>
#include <mcap/reader.hpp>

#include <ddsrecorder_participants/constants.hpp>
#include <ddsrecorder_participants/recorder/handler/mcap/McapWriter.hpp>
#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>
#include <ddsrecorder_participants/recorder/output/OutputSettings.hpp>

using namespace eprosima;

namespace {

struct McapWriterTest : public testing::Test
{
    void SetUp() override
    {
        output_file_ = "McapWriterTest_partition_repeated.mcap";
        std::filesystem::remove(output_file_);
        std::filesystem::remove(output_file_ + ".tmp~");
    }

    void TearDown() override
    {
        std::filesystem::remove(output_file_);
        std::filesystem::remove(output_file_ + ".tmp~");
    }

    std::string output_file_;
};

/**
 * Verify that returning to an earlier partition creates a new MCAP channel version.
 *
 * CASES:
 *  - A writer changes partitions A -> B -> A.
 *  - The MCAP metadata contains all three partition states in that order.
 *  - Interleaving another writer does not create a duplicate version when the first writer's
 *    partition has not changed.
 */
TEST_F(McapWriterTest, partition_repeated)
{
    ddsrecorder::participants::OutputSettings output_settings;
    output_settings.filepath = ".";
    output_settings.filename = "McapWriterTest_partition_repeated";
    output_settings.extension = ".mcap";
    output_settings.prepend_timestamp = false;
    output_settings.local_timestamp = false;
    output_settings.resource_limits.max_file_size_ = 1024 * 1024;
    output_settings.resource_limits.max_size_ = output_settings.resource_limits.max_file_size_;

    auto file_tracker = std::make_shared<ddsrecorder::participants::FileTracker>(output_settings);
    mcap::McapWriterOptions mcap_options{"ros2"};
    ddsrecorder::participants::McapWriter writer(output_settings, mcap_options, file_tracker, false);

    ddspipe::core::types::DdsTopic topic;
    topic.m_topic_name = "partition_test_topic";
    topic.type_name = "partition_test_type";

    std::map<ddspipe::core::types::DdsTopic, mcap::Channel> channels;
    channels.emplace(topic, mcap::Channel(topic.m_topic_name, "cdr", 0));
    writer.set_channels(channels);
    writer.enable();

    mcap::Schema schema(topic.type_name, "cdr", "partition test schema");
    writer.write(schema);
    channels.at(topic).schemaId = schema.id;

    const auto write_sample = [&](const std::string& writer_guid, const std::string& partition)
            {
                ddsrecorder::participants::McapMessage message;
                message.topic = topic;
                message.writer_guid_string = writer_guid;
                message.partitions = partition;
                const std::byte data{0};
                message.data = &data;
                message.dataSize = 1;
                message.logTime = 1;
                message.publishTime = 1;
                writer.write(message);
            };

    write_sample("writer_a", "A");
    write_sample("writer_b", "A");
    write_sample("writer_a", "B");
    write_sample("writer_b", "A");
    write_sample("writer_a", "A");
    writer.disable();

    mcap::McapReader reader;
    ASSERT_TRUE(reader.open(output_file_).ok());

    for (const auto& message : reader.readMessages())
    {
        (void) message;
    }

    std::map<mcap::ChannelId, std::string> partition_metadata;
    for (const auto& [channel_id, channel] : reader.channels())
    {
        if (channel->topic != topic.m_topic_name)
        {
            continue;
        }

        const auto metadata_it = channel->metadata.find(ddsrecorder::participants::PARTITIONS);
        ASSERT_NE(metadata_it, channel->metadata.end());
        partition_metadata[channel_id] = metadata_it->second;
    }

    reader.close();

    ASSERT_EQ(partition_metadata.size(), 4u);
    EXPECT_EQ(partition_metadata.begin()->second, "writer_a:A;");
    EXPECT_EQ(std::next(partition_metadata.begin(), 1)->second, "writer_a:A;writer_b:A;");
    EXPECT_EQ(std::next(partition_metadata.begin(), 2)->second, "writer_a:A;writer_b:A;writer_a:B;");
    EXPECT_EQ(std::next(partition_metadata.begin(), 3)->second,
            "writer_a:A;writer_b:A;writer_a:B;writer_a:A;");
}

} // namespace

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
