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

/**
 * @file SqlReaderParticipant.cpp
 */

#include <map>
#include <string>

#include <sqlite3.h>

#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/Log.hpp>
#include <cpp_utils/memory/Heritable.hpp>
#include <cpp_utils/time/time_utils.hpp>

#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddsrecorder_participants/common/serialize/Serializer.hpp>
#include <ddsrecorder_participants/common/time_utils.hpp>
#include <ddsrecorder_participants/constants.hpp>
#include <ddsrecorder_participants/replayer/SqlReaderParticipant.hpp>
namespace eprosima {
namespace ddsrecorder {
namespace participants {

SqlReaderParticipant::SqlReaderParticipant(
        const std::shared_ptr<BaseReaderParticipantConfiguration>& configuration,
        const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
        const std::string& file_path)
    : BaseReaderParticipant(configuration, payload_pool, file_path)
{
}

SqlReaderParticipant::~SqlReaderParticipant()
{
}

void SqlReaderParticipant::process_summary(
        std::set<utils::Heritable<ddspipe::core::types::DdsTopic>>& topics,
        DynamicTypesCollection& types)
{
    open_file_();

    exec_sql_statement_("SELECT name, type, qos FROM Topics;", {}, [&](sqlite3_stmt* stmt)
    {
        // Create a DdsTopic to publish the message
        const std::string topic_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const std::string type_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const auto topic = utils::Heritable<ddspipe::core::types::DdsTopic>::make_heritable(
                create_topic_(topic_name, type_name, true));

        // Apply the QoS stored in the MCAP file as if they were the discovered QoS.
        const auto topic_qos_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const auto topic_qos = Serializer::deserialize<ddspipe::core::types::TopicQoS>(topic_qos_str);

        topic->topic_qos.set_qos(topic_qos, utils::FuzzyLevelValues::fuzzy_level_fuzzy);

        // Store the topic in the cache
        topics_[topic_name] = *topic;

        // Store the topic in the set
        topics.insert(topic);
    });

    exec_sql_statement_("SELECT name, information, object FROM Types;", {}, [&](sqlite3_stmt* stmt)
    {
        // Read the type data from the database
        const std::string type_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const std::string type_information = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const std::string type_object = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        // Create a DynamicType to store the type data
        DynamicType type;

        type.type_name(type_name);
        type.type_information(type_information);
        type.type_object(type_object);

        // Store the DynamicType in the DynamicTypesCollection
        types.dynamic_types().push_back(type);
    });

    close_file_();
}

void SqlReaderParticipant::process_messages()
{
    open_file_();

    // Define the time to start replaying messages
    const auto initial_timestamp = when_to_start_replay_(configuration_->start_replay_time);

    const auto begin_time = to_sql_timestamp(
            configuration_->begin_time.is_set() ?
            configuration_->begin_time.get_reference() :
            utils::the_beginning_of_time());

    const auto end_time = to_sql_timestamp(
            configuration_->end_time.is_set() ?
            configuration_->end_time.get_reference() :
            utils::the_end_of_time());

    exec_sql_statement_(
        "SELECT log_time, topic, data, data_size FROM Messages "
        "WHERE log_time >= ? AND log_time <= ? "
        "ORDER BY log_time;",
        {begin_time, end_time},
        [&](sqlite3_stmt* stmt)
    {
        const auto log_time = to_std_timestamp(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));

        // Store the timestamp of the first recorded message
        static utils::Timestamp first_message_timestamp = log_time;

        // Create a DdsTopic to publish the message
        const std::string topic_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const auto topic = find_topic_(topic_name);

        // Find the reader for the topic
        if (readers_.find(topic) == readers_.end())
        {
            logError(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                    "Failed to replay message in topic " << topic << ": topic not found, skipping...");
            return;
        }

        logInfo(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                "Scheduling message to be replayed in topic " << topic << ".");

        // Set publication delay from original log time and configured playback rate
        const auto delay = (log_time - first_message_timestamp) / configuration_->rate;
        const auto delay_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(delay);
        const auto time_to_write =
                std::chrono::time_point_cast<utils::Timestamp::duration>(initial_timestamp + delay_ns);

        // Create a RtpsPayloadData from the raw data
        const auto raw_data = sqlite3_column_blob(stmt, 2);
        const auto raw_data_size = sqlite3_column_int(stmt, 3);
        auto data = create_payload_(raw_data, raw_data_size);

        // Set source timestamp
        // NOTE: this is important for QoS such as LifespanQosPolicy
        data->source_timestamp = fastrtps::rtps::Time_t(to_ticks(time_to_write) / 1e9);

        // Wait until it's time to write the message
        wait_until_timestamp_(time_to_write);

        logInfo(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                "Replaying message in topic " << topic << ".");

        // Insert new data in internal reader queue
        readers_[topic]->simulate_data_reception(std::move(data));
    });

    close_file_();
}

void SqlReaderParticipant::open_file_()
{
    const auto ret = sqlite3_open(file_path_.c_str(), &database_);

    if (ret != SQLITE_OK)
    {
        const std::string error_msg = utils::Formatter() << "Failed to open SQL file " << file_path_
                                                         << " for reading: " << sqlite3_errmsg(database_);
        sqlite3_close(database_);

        logError(DDSREPLAYER_SQL_READER_PARTICIPANT, "FAIL_SQL_OPEN | " << error_msg);
        throw utils::InitializationException(error_msg);
    }
}

void SqlReaderParticipant::close_file_()
{
    sqlite3_close(database_);
}

ddspipe::core::types::DdsTopic SqlReaderParticipant::find_topic_(
        const std::string& topic_name)
{
    // Check if the topic is in the cache
    if (topics_.find(topic_name) != topics_.end())
    {
        return topics_[topic_name];
    }

    const auto type_name = find_type_of_topic_(topic_name);

    // Create the DdsTopic
    const auto topic = create_topic_(topic_name, type_name, false);

    // Store the topic in the cache
    topics_[topic_name] = topic;

    return topic;
}

std::string SqlReaderParticipant::find_type_of_topic_(
        const std::string& topic_name)
{
    std::string type_name;

    exec_sql_statement_("SELECT type FROM Topics WHERE name = ?;", {topic_name}, [&](sqlite3_stmt* stmt)
    {
        if (!type_name.empty())
        {
            const std::string error_msg = utils::Formatter() << "Multiple types found for topic " << topic_name;

            logError(DDSREPLAYER_SQL_READER_PARTICIPANT, "FAIL_SQL_READ | " << error_msg);
            throw std::runtime_error(error_msg);
        }

        type_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    });

    return type_name;
}

void SqlReaderParticipant::exec_sql_statement_(
    const std::string& statement,
    const std::vector<std::string>& bind_values,
    const std::function<void(sqlite3_stmt*)>& process_row)
{
    sqlite3_stmt* stmt;

    // Prepare the SQL statement
    const auto ret = sqlite3_prepare_v2(database_, statement.c_str(), -1, &stmt, nullptr);

    if (ret != SQLITE_OK)
    {
        const std::string error_msg = utils::Formatter() << "Failed to prepare SQL statement: "
                                                         << sqlite3_errmsg(database_);
        sqlite3_finalize(stmt);

        logError(DDSREPLAYER_SQL_READER_PARTICIPANT, "FAIL_SQL_READ | " << error_msg);
        throw std::runtime_error(error_msg);
    }

    // Guard the statement to ensure it's always finalized
    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt_guard(stmt, sqlite3_finalize);

    // Bind the values to the statement
    for (int i = 0; i < (int) bind_values.size(); i++)
    {
        const auto bind_ret = sqlite3_bind_text(stmt, i+1, bind_values[i].c_str(), -1, SQLITE_STATIC);

        if (bind_ret != SQLITE_OK)
        {
            const std::string error_msg = utils::Formatter() << "Failed to bind SQL statement to read messages: "
                                                            << sqlite3_errmsg(database_);

            logError(DDSREPLAYER_SQL_READER_PARTICIPANT, "FAIL_SQL_READ | " << error_msg);
            throw utils::InconsistencyException(error_msg);
        }
    }

    // Step through the statement and process the rows
    int step_ret;

    while ((step_ret = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        process_row(stmt);
    }

    if (step_ret != SQLITE_DONE)
    {
        const std::string error_msg = utils::Formatter() << "Failed to fetch data: "
                                                         << sqlite3_errmsg(database_);

        logError(DDSREPLAYER_SQL_READER_PARTICIPANT, "FAIL_SQL_READ | " << error_msg);
        throw std::runtime_error(error_msg);
    }
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
