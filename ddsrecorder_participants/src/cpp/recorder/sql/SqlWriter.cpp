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
 * @file SqlWriter.cpp
 */

#include <stdexcept>

#include <sqlite3.h>

#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/Formatter.hpp>
#include <cpp_utils/Log.hpp>

#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddsrecorder_participants/recorder/message/SqlMessage.hpp>
#include <ddsrecorder_participants/recorder/monitoring/producers/DdsRecorderStatusMonitorProducer.hpp>
#include <ddsrecorder_participants/recorder/output/Serializer.hpp>
#include <ddsrecorder_participants/recorder/sql/SqlWriter.hpp>
#include <ddsrecorder_participants/recorder/sql/utils.hpp>

#if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v1/DynamicTypesCollection.hpp>
#else
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v2/DynamicTypesCollection.hpp>
#endif // if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13

namespace eprosima {
namespace ddsrecorder {
namespace participants {

SqlWriter::SqlWriter(
        const OutputSettings& configuration,
        std::shared_ptr<FileTracker>& file_tracker,
        const bool record_types)
    : BaseWriter(configuration, file_tracker, record_types, MIN_SQL_SIZE)
{
}

void SqlWriter::open_new_file_nts_(
        const std::uint64_t min_file_size)
{
    try
    {
        file_tracker_->new_file(min_file_size);
    }
    catch (const std::invalid_argument& e)
    {
        throw FullDiskException(
                    "The minimum SQL size (" + utils::from_bytes(min_file_size) + ") is greater than the maximum SQL "
                    "size (" + utils::from_bytes(configuration_.max_file_size) + ").");
    }

    const auto filename = file_tracker_->get_current_filename();

    // Create SQLite database
    const auto ret = sqlite3_open(filename.c_str(), &database_);

    if (ret != SQLITE_OK)
    {
        const std::string error_msg = utils::Formatter() << "Failed to open SQL file " << filename
                                                         << " for writing: " << sqlite3_errmsg(database_);
        sqlite3_close(database_);

        logError(DDSRECORDER_SQL_WRITER, "FAIL_SQL_OPEN | " << error_msg);
        throw utils::InitializationException(error_msg);
    }

    // Create Types table
    const std::string create_types_table{
    R"(
        CREATE TABLE IF NOT EXISTS Types (
            name TEXT PRIMARY KEY NOT NULL,
            information TEXT NOT NULL,
            object TEXT NOT NULL
        );
    )"};

    create_sql_table_("Types", create_types_table);

    // Create Topics table
    const std::string create_topics_table{
    R"(
        CREATE TABLE IF NOT EXISTS Topics (
            name TEXT PRIMARY KEY NOT NULL,
            type TEXT NOT NULL,
            qos TEXT NOT NULL,
            FOREIGN KEY(type) REFERENCES Types(name)
        );
    )"};

    create_sql_table_("Topics", create_topics_table);

    // Create Messages table
    const std::string create_messages_table{
    R"(
        CREATE TABLE IF NOT EXISTS Messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            data BLOB NOT NULL,
            data_size INTEGER NOT NULL,
            topic TEXT NOT NULL,
            key TEXT NOT NULL,
            log_time DATETIME NOT NULL,
            publish_time DATETIME NOT NULL,
            FOREIGN KEY(topic) REFERENCES Topics(name)
        );
    )"};

    create_sql_table_("Messages", create_messages_table);
}

void SqlWriter::close_current_file_nts_()
{
    sqlite3_close(database_);
    file_tracker_->close_file();
}

template <>
void SqlWriter::write_nts_(
        const DynamicType& dynamic_type)
{
    logInfo(DDSRECORDER_SQL_WRITER, "Writing dynamic type " << dynamic_type.type_name() << ".");

    // Define the SQL statement
    const char* insert_statement = R"(
        INSERT INTO Types (name, information, object)
        VALUES (?, ?, ?);
    )";

    // Prepare the SQL statement
    sqlite3_stmt* statement;
    const auto prep_ret = sqlite3_prepare_v2(database_, insert_statement, -1, &statement, nullptr);

    if (prep_ret != SQLITE_OK)
    {
        const std::string error_msg = utils::Formatter() << "Failed to prepare SQL statement to write dynamic type: "
                                                         << sqlite3_errmsg(database_);
        sqlite3_finalize(statement);

        logError(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << error_msg);
        throw utils::InconsistencyException(error_msg);
    }

    // Bind the DynamicType to the SQL statement
    sqlite3_bind_text(statement, 1, dynamic_type.type_name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, dynamic_type.type_information().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, dynamic_type.type_object().c_str(), -1, SQLITE_TRANSIENT);

    // Execute the SQL statement
    const auto step_ret = sqlite3_step(statement);

    if (step_ret != SQLITE_DONE)
    {
        const std::string error_msg = utils::Formatter() << "Failed to write dynamic type to SQL database: "
                                                         << sqlite3_errmsg(database_);
        sqlite3_finalize(statement);

        logError(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << error_msg);
        throw utils::InconsistencyException(error_msg);
    }

    // Finalize the SQL statement
    sqlite3_finalize(statement);
}

template <>
void SqlWriter::write_nts_(
        const SqlMessage& msg)
{
    if (!enabled_)
    {
        logWarning(DDSRECORDER_SQL_WRITER, "Attempting to write a message in a disabled writer.");
        return;
    }

    logInfo(DDSRECORDER_SQL_WRITER, "Writing message: " << utils::from_bytes(msg.get_data_size()) << ".");

    // Define the SQL statement
    const char* insert_statement = R"(
        INSERT INTO Messages (data, data_size, topic, key, log_time, publish_time)
        VALUES (?, ?, ?, ?, ?, ?);
    )";

    // Prepare the SQL statement
    sqlite3_stmt* statement;
    const auto prep_ret = sqlite3_prepare_v2(database_, insert_statement, -1, &statement, nullptr);

    if (prep_ret != SQLITE_OK)
    {
        const std::string error_msg = utils::Formatter() << "Failed to prepare SQL statement to write message: "
                                                         << sqlite3_errmsg(database_);
        sqlite3_finalize(statement);

        logError(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << error_msg);
        throw utils::InconsistencyException(error_msg);
    }

    // Bind the SqlMessage to the SQL statement
    sqlite3_bind_blob(statement, 1, msg.get_data(), msg.get_data_size(), SQLITE_TRANSIENT);
    sqlite3_bind_int64(statement, 2, msg.get_data_size());
    sqlite3_bind_text(statement, 3, msg.topic.topic_name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 4, msg.key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 5, to_sql_timestamp(msg.log_time).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 6, to_sql_timestamp(msg.publish_time).c_str(), -1, SQLITE_TRANSIENT);

    // Execute the SQL statement
    const auto step_ret = sqlite3_step(statement);

    if (step_ret != SQLITE_DONE)
    {
        const std::string error_msg = utils::Formatter() << "Failed to write message to SQL database: "
                                                         << sqlite3_errmsg(database_);
        sqlite3_finalize(statement);

        logError(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << error_msg);
        throw utils::InconsistencyException(error_msg);
    }

    // Finalize the SQL statement
    sqlite3_finalize(statement);
}

template <>
void SqlWriter::write_nts_(
        const ddspipe::core::types::DdsTopic& topic)
{
    if (!enabled_)
    {
        logWarning(DDSRECORDER_SQL_WRITER, "Attempting to write a topic in a disabled writer.");
        return;
    }

    logInfo(DDSRECORDER_SQL_WRITER, "Writing topic " << topic.topic_name() << ".");

    // Define the SQL statement
    const char* insert_statement = R"(
        INSERT INTO Topics (name, type, qos)
        VALUES (?, ?, ?);
    )";

    // Prepare the SQL statement
    sqlite3_stmt* statement;
    const auto prep_ret = sqlite3_prepare_v2(database_, insert_statement, -1, &statement, nullptr);

    if (prep_ret != SQLITE_OK)
    {
        const std::string error_msg = utils::Formatter() << "Failed to prepare SQL statement to write topic: "
                                                        << sqlite3_errmsg(database_);
        sqlite3_finalize(statement);

        logError(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << error_msg);
        throw utils::InconsistencyException(error_msg);
    }

    // Bind the Topic to the SQL statement
    sqlite3_bind_text(statement, 1, topic.topic_name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, topic.type_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, Serializer::serialize(topic.topic_qos).c_str(), -1, SQLITE_TRANSIENT);

    // Execute the SQL statement
    const auto step_ret = sqlite3_step(statement);

    if (step_ret != SQLITE_DONE)
    {
        const std::string error_msg = utils::Formatter() << "Failed to write topic to SQL database: "
                                                        << sqlite3_errmsg(database_);
        sqlite3_finalize(statement);

        logError(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << error_msg);
        throw utils::InconsistencyException(error_msg);
    }

    // Finalize the SQL statement
    sqlite3_finalize(statement);
}

void SqlWriter::create_sql_table_(
        const std::string& table_name,
        const std::string& table_definition)
{
    const auto ret = sqlite3_exec(database_, table_definition.c_str(), nullptr, nullptr, nullptr);

    if (ret != SQLITE_OK)
    {
        const std::string error_msg = utils::Formatter() << "Failed to create " << table_name << " table: "
                                                         << sqlite3_errmsg(database_);
        close_current_file_nts_();

        logError(DDSRECORDER_SQL_WRITER, "FAIL_SQL_OPEN | " << error_msg);
        throw utils::InitializationException(error_msg);
    }
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
