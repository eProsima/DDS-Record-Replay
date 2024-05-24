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

#include <ddsrecorder_participants/replayer/SqlReaderParticipant.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

SqlReaderParticipant::SqlReaderParticipant(
        const std::shared_ptr<McapReaderParticipantConfiguration>& configuration,
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

    exec_sql_statement_("SELECT * FROM Topics;", [&](sqlite3_stmt* stmt)
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
        topics.insert(utils::Heritable<ddspipe::core::types::DdsTopic>::make_heritable(topic));
    });

    exec_sql_statement_("SELECT * FROM Types;", [&](sqlite3_stmt* stmt)
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

    exec_sql_statement_("SELECT * FROM Messages ORDER BY publish_time;", [&](sqlite3_stmt* stmt)
    {
        // Create a DdsTopic to publish the message
        const std::string topic_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const auto topic = find_topic_(topic_name);

        // Find the reader for the topic
        if (readers_.find(topic) == readers_.end())
        {
            logError(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                    "Failed to replay message in topic " << topic << ": topic not found, skipping...");
            return;
        }

        const auto reader = readers_[topic];

        // Create a RtpsPayloadData from the raw data
        const auto raw_data = sqlite3_column_blob(stmt, 1);
        const auto raw_data_size = sqlite3_column_int(stmt, 2);
        auto data = create_payload_(raw_data, raw_data_size);

        logInfo(DDSREPLAYER_MCAP_READER_PARTICIPANT,
                "Scheduling message to be replayed in topic " << topic << ".");

        // Wait until it's time to write the message
        wait_until_timestamp_(utils::now());

        logInfo(DDSREPLAYER_MCAP_READER_PARTICIPANT, "Replaying message in topic " << topic << ".");

        // Insert new data in internal reader queue
        reader->simulate_data_reception(std::move(data));
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
    // Find the type of the Topic with name topic_name
    const char* select_statement = R"(
        SELECT type FROM Topics
        WHERE name = ?;
    )";

    sqlite3_stmt* statement;
    const auto prep_ret = sqlite3_prepare_v2(database_, select_statement, -1, &statement, nullptr);

    if (prep_ret != SQLITE_OK)
    {
        const std::string error_msg = utils::Formatter() << "Failed to prepare SQL statement to read messages: "
                                                        << sqlite3_errmsg(database_);
        sqlite3_finalize(statement);

        logError(DDSREPLAYER_SQL_READER_PARTICIPANT, "FAIL_SQL_READ | " << error_msg);
        throw utils::InconsistencyException(error_msg);
    }

    const auto bind_ret = sqlite3_bind_text(statement, 1, topic_name.c_str(), -1, SQLITE_STATIC);

    if (bind_ret != SQLITE_OK)
    {
        const std::string error_msg = utils::Formatter() << "Failed to bind SQL statement to read messages: "
                                                        << sqlite3_errmsg(database_);
        sqlite3_finalize(statement);

        logError(DDSREPLAYER_SQL_READER_PARTICIPANT, "FAIL_SQL_READ | " << error_msg);
        throw utils::InconsistencyException(error_msg);
    }

    std::string type_name;

    switch (sqlite3_step(statement))
    {
        case SQLITE_ROW:
        {
            // Store the type in the topic
            type_name = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
            break;
        }
        case SQLITE_DONE:
        {
            logError(DDSREPLAYER_SQL_READER_PARTICIPANT,
                     "FAIL_SQL_READ | No type found for topic '" << topic_name << "'.");
            break;
        }
        default:
        {
            const std::string error_msg = utils::Formatter() << "Failed to execute query: "
                                                            << sqlite3_errmsg(database_);
            sqlite3_finalize(statement);

            // Maybe we should finalize statements on top?

            logError(DDSREPLAYER_SQL_READER_PARTICIPANT, "FAIL_SQL_READ | " << error_msg);
            throw utils::InconsistencyException(error_msg);
        }
    }

    return type_name;
}

void SqlReaderParticipant::exec_sql_statement_(
    const std::string& statement,
    const std::function<void(sqlite3_stmt*)>& process_row)
{
    sqlite3_stmt* stmt;

    const auto ret = sqlite3_prepare_v2(database_, statement.c_str(), -1, &stmt, nullptr);

    if (ret != SQLITE_OK)
    {
        const std::string error_msg = utils::Formatter() << "Failed to prepare SQL statement: "
                                                         << sqlite3_errmsg(database_);
        sqlite3_finalize(stmt);

        logError(DDSREPLAYER_SQL_READER_PARTICIPANT, "FAIL_SQL_READ | " << error_msg);
        throw std::runtime_error(error_msg);
    }

    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt_guard(stmt, sqlite3_finalize);

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
