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

#include <cstdio>
#include <memory>
#include <set>
#include <string>

#include <cpp_utils/memory/Heritable.hpp>
#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <sqlite/sqlite3.h>

#include <ddspipe_core/efficiency/payload/FastPayloadPool.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddsrecorder_participants/common/serialize/Serializer.hpp>
#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollection.hpp>
#include <ddsrecorder_participants/replayer/BaseReaderParticipantConfiguration.hpp>
#include <ddsrecorder_participants/replayer/SqlReaderParticipant.hpp>

using namespace eprosima;
using namespace eprosima::ddsrecorder::participants;

namespace {

constexpr const char* TEST_TYPE_NAME = "TestType";
constexpr const char* TOPIC_WITH_SAMPLES = "TopicWithSamples";
constexpr const char* READER_ONLY_TOPIC = "ReaderOnlyTopic";

/**
 * @brief Minimal SQL recording with two topics: one with a captured sample, and one that was only
 * announced (as the recorder does for every discovered DataReader) and thus has no \c Messages row.
 */
class SqlRecording
{
public:

    SqlRecording(
            const std::string& file_path)
        : file_path_(file_path)
    {
        std::remove(file_path_.c_str());
    }

    //! Write the recording. Separate from the constructor so that gtest assertions can be used.
    void build(
            const bool with_partitions_row)
    {
        ASSERT_EQ(SQLITE_OK, sqlite3_open(file_path_.c_str(), &database_));

        // Schema as written by SqlWriter
        exec("CREATE TABLE Types (name TEXT PRIMARY KEY NOT NULL, information TEXT NOT NULL, "
                "object TEXT NOT NULL, is_ros2_type TEXT NOT NULL);");
        exec("CREATE TABLE Topics (name TEXT NOT NULL, type TEXT NOT NULL, qos TEXT NOT NULL, "
                "is_ros2_topic TEXT NOT NULL, PRIMARY KEY(name, type));");
        exec("CREATE TABLE Messages (writer_guid TEXT NOT NULL, sequence_number INTEGER NOT NULL, "
                "data_json TEXT, data_cdr BLOB, data_cdr_size INTEGER, topic TEXT NOT NULL, "
                "type TEXT NOT NULL, key TEXT NOT NULL, log_time DATETIME NOT NULL, "
                "publish_time DATETIME NOT NULL, PRIMARY KEY(writer_guid, sequence_number));");
        exec("CREATE TABLE Partitions (name TEXT NOT NULL, PRIMARY KEY(name));");
        exec("CREATE TABLE TopicsPartitions (topic TEXT NOT NULL, type TEXT NOT NULL, "
                "partition TEXT NOT NULL, PRIMARY KEY(topic, type, partition));");
        exec("CREATE TABLE MessagesPartitions (writer_guid TEXT NOT NULL, sequence_number INTEGER NOT NULL, "
                "partition TEXT NOT NULL, PRIMARY KEY(writer_guid, sequence_number, partition));");

        const ddspipe::core::types::TopicQoS default_qos{};
        std::string qos_str;
        ASSERT_TRUE(Serializer::serialize<ddspipe::core::types::TopicQoS>(default_qos, qos_str));
        qos_str = sql_escaped(qos_str);

        exec("INSERT INTO Types VALUES ('" + std::string(TEST_TYPE_NAME) + "', '', '', 'false');");
        exec("INSERT INTO Partitions VALUES ('PartitionA');");

        // A topic with one captured sample
        exec("INSERT INTO Topics VALUES ('" + std::string(TOPIC_WITH_SAMPLES) + "', '" + TEST_TYPE_NAME + "', '" +
                qos_str + "', 'false');");
        exec("INSERT INTO TopicsPartitions VALUES ('" + std::string(TOPIC_WITH_SAMPLES) + "', '" + TEST_TYPE_NAME +
                "', 'PartitionA');");
        exec("INSERT INTO Messages VALUES ('01.02.03.04|0.0.1.3', 1, NULL, X'00', 1, '" +
                std::string(TOPIC_WITH_SAMPLES) + "', '" + TEST_TYPE_NAME +
                "', '', '2024-01-01 00:00:00.000', '2024-01-01 00:00:00.000');");
        exec("INSERT INTO MessagesPartitions VALUES ('01.02.03.04|0.0.1.3', 1, 'PartitionA');");

        // A topic that was only announced: no Messages row joins it, so the summary query aggregates
        // its writer guids to NULL
        exec("INSERT INTO Topics VALUES ('" + std::string(READER_ONLY_TOPIC) + "', '" + TEST_TYPE_NAME + "', '" +
                qos_str + "', 'false');");

        if (with_partitions_row)
        {
            // Only the partitions column is non-NULL, as when the announced reader had partitions
            exec("INSERT INTO TopicsPartitions VALUES ('" + std::string(READER_ONLY_TOPIC) + "', '" + TEST_TYPE_NAME +
                    "', 'PartitionA');");
        }
    }

    ~SqlRecording()
    {
        sqlite3_close(database_);
        std::remove(file_path_.c_str());
    }

    const std::string& file_path() const
    {
        return file_path_;
    }

protected:

    //! Escape a value so that it can be embedded in a SQL string literal
    static std::string sql_escaped(
            const std::string& value)
    {
        std::string escaped;
        for (const auto character : value)
        {
            escaped += character;
            if (character == '\'')
            {
                escaped += character;
            }
        }
        return escaped;
    }

    void exec(
            const std::string& statement)
    {
        char* error_msg = nullptr;
        const auto ret = sqlite3_exec(database_, statement.c_str(), nullptr, nullptr, &error_msg);
        const std::string error = error_msg != nullptr ? error_msg : "";
        sqlite3_free(error_msg);
        ASSERT_EQ(SQLITE_OK, ret) << statement << ": " << error;
    }

    std::string file_path_;
    sqlite3* database_ = nullptr;
};

std::set<utils::Heritable<ddspipe::core::types::DdsTopic>> read_summary(
        const std::string& file_path,
        const std::set<std::string>& allowed_partitions)
{
    auto configuration = std::make_shared<BaseReaderParticipantConfiguration>();
    auto payload_pool = std::make_shared<ddspipe::core::FastPayloadPool>();

    SqlReaderParticipant participant(configuration, payload_pool, file_path);
    participant.add_partition_list(allowed_partitions);

    std::set<utils::Heritable<ddspipe::core::types::DdsTopic>> topics;
    DynamicTypesCollection types;

    participant.process_summary(topics, types);

    return topics;
}

bool contains_topic(
        const std::set<utils::Heritable<ddspipe::core::types::DdsTopic>>& topics,
        const std::string& topic_name)
{
    for (const auto& topic : topics)
    {
        if (topic->m_topic_name == topic_name)
        {
            return true;
        }
    }
    return false;
}

} // namespace

/*!
 * @test. Regression test. A topic with no captured samples must not break the summary.
 *
 * The summary query aggregates the writer guids of a topic with GROUP_CONCAT over a chain of LEFT
 * JOINs, so a topic that was announced but whose data was never captured aggregates to NULL. The
 * recorder writes such a Topics row for every discovered DataReader, and a capture point commonly
 * sees the announcements of a topic without seeing its (often unicast) data.
 */
TEST(SqlReaderParticipantTest, topic_without_samples_is_processed)
{
    SqlRecording recording("SqlReaderParticipantTest_no_samples.db");
    ASSERT_NO_FATAL_FAILURE(recording.build(false));

    const auto topics = read_summary(recording.file_path(), {});

    ASSERT_TRUE(contains_topic(topics, TOPIC_WITH_SAMPLES));
    ASSERT_TRUE(contains_topic(topics, READER_ONLY_TOPIC));
}

/*!
 * @test Regression test. Same as above, but the announced topic does have partitions registered, so only its writer
 * guids aggregate to NULL.
 */
TEST(SqlReaderParticipantTest, topic_without_samples_with_partitions_is_processed)
{
    SqlRecording recording("SqlReaderParticipantTest_no_samples_partitions.db");
    ASSERT_NO_FATAL_FAILURE(recording.build(true));

    const auto topics = read_summary(recording.file_path(), {});

    ASSERT_TRUE(contains_topic(topics, TOPIC_WITH_SAMPLES));
    ASSERT_TRUE(contains_topic(topics, READER_ONLY_TOPIC));
}

/*!
 * @test Regression test. A topic with no captured samples must not break the summary when a partition filter is set
 * either, which is the path that registers the writer guids that do not pass the filter.
 */
TEST(SqlReaderParticipantTest, topic_without_samples_is_filtered_out)
{
    SqlRecording recording("SqlReaderParticipantTest_no_samples_filtered.db");
    ASSERT_NO_FATAL_FAILURE(recording.build(true));

    const auto topics = read_summary(recording.file_path(), {"AnotherPartition"});

    ASSERT_FALSE(contains_topic(topics, TOPIC_WITH_SAMPLES));
    ASSERT_FALSE(contains_topic(topics, READER_ONLY_TOPIC));
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
