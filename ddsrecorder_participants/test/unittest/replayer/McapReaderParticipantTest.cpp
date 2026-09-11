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

#include <gtest/gtest.h>

#include <mcap/types.hpp>

#include <ddsrecorder_participants/constants.hpp>
#include <ddsrecorder_participants/replayer/McapReaderParticipant.hpp>

namespace {

class McapReaderParticipantAccessor : public eprosima::ddsrecorder::participants::McapReaderParticipant
{
public:

    using McapReaderParticipant::get_writer_partition_from_channel_;
};

} // namespace

/**
 * Test that the latest partition entry is selected when a writer changes partitions.
 *
 * CASES:
 * - A writer is recorded first in partition A and then in partition B.
 * - The reader selects partition B from the channel metadata.
 */
TEST(McapReaderParticipantTest, latest_partition_entry_is_used)
{
    mcap::Channel channel;
    channel.metadata[eprosima::ddsrecorder::participants::PARTITIONS] = "writer:A;writer:B;";

    std::string partition;
    ASSERT_TRUE(McapReaderParticipantAccessor::get_writer_partition_from_channel_(
                        channel,
                        "writer",
                        partition));
    ASSERT_EQ(partition, "B");
}

/**
 * Test that the latest partition entry is selected independently for each writer.
 *
 * CASES:
 * - One writer changes from partition A to partition B.
 * - A second writer remains in partition C.
 * - The reader returns the correct current partition for both writers.
 */
TEST(McapReaderParticipantTest, latest_entry_is_selected_per_writer)
{
    mcap::Channel channel;
    channel.metadata[eprosima::ddsrecorder::participants::PARTITIONS] =
            "writer_a:A;writer_b:C;writer_a:B;";

    std::string partition;
    ASSERT_TRUE(McapReaderParticipantAccessor::get_writer_partition_from_channel_(
                        channel,
                        "writer_a",
                        partition));
    ASSERT_EQ(partition, "B");

    ASSERT_TRUE(McapReaderParticipantAccessor::get_writer_partition_from_channel_(
                        channel,
                        "writer_b",
                        partition));
    ASSERT_EQ(partition, "C");
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
