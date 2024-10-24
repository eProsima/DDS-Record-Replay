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

#include <sstream>
#include <stdexcept>
#include <vector>

#include <sqlite3.h>

#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/Formatter.hpp>
#include <cpp_utils/Log.hpp>
#include <cpp_utils/ros2_mangling.hpp>

#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddsrecorder_participants/common/serialize/Serializer.hpp>
#include <ddsrecorder_participants/common/time_utils.hpp>
#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollection.hpp>
#include <ddsrecorder_participants/recorder/message/SqlMessage.hpp>
#include <ddsrecorder_participants/recorder/monitoring/producers/DdsRecorderStatusMonitorProducer.hpp>
#include <ddsrecorder_participants/recorder/sql/SqlHandlerConfiguration.hpp>
#include <ddsrecorder_participants/recorder/sql/SqlWriter.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

SqlWriter::SqlWriter(
        const OutputSettings& configuration,
        std::shared_ptr<FileTracker>& file_tracker,
        const bool record_types,
        const bool ros2_types,
        const DataFormat data_format)
    : BaseWriter(configuration, file_tracker, record_types, MIN_SQL_SIZE)
    , ros2_types_(ros2_types)
    , data_format_(data_format)
{
}

void SqlWriter::update_dynamic_types(
        const DynamicType& dynamic_type)
{
    std::lock_guard<std::mutex> lock(mutex_);

    dynamic_types_.push_back(dynamic_type);
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
            object TEXT NOT NULL,
            is_ros2_type TEXT NOT NULL
        );
    )"};

    create_sql_table_("Types", create_types_table);

    // Create Topics table
    const std::string create_topics_table{
    R"(
        CREATE TABLE IF NOT EXISTS Topics (
            name TEXT NOT NULL,
            type TEXT NOT NULL,
            qos TEXT NOT NULL,
            is_ros2_topic TEXT NOT NULL,
            PRIMARY KEY(name, type),
            FOREIGN KEY(type) REFERENCES Types(name)
        );
    )"};

    create_sql_table_("Topics", create_topics_table);

    // Create Messages table
    const std::string create_messages_table{
    R"(
        CREATE TABLE IF NOT EXISTS Messages (
            writer_guid TEXT NOT NULL,
            sequence_number INTEGER NOT NULL,
            data_json TEXT,
            data_cdr BLOB,
            data_cdr_size INTEGER,
            topic TEXT NOT NULL,
            type TEXT NOT NULL,
            key TEXT NOT NULL,
            log_time DATETIME NOT NULL,
            publish_time DATETIME NOT NULL,
            PRIMARY KEY(writer_guid, sequence_number),
            FOREIGN KEY(topic, type) REFERENCES Topics(name, type)
        );
    )"};

    create_sql_table_("Messages", create_messages_table);
}

template <>
void SqlWriter::write_nts_(
        const DynamicType& dynamic_type)
{
    if (!enabled_)
    {
        logWarning(DDSRECORDER_SQL_WRITER, "Attempting to write a dynamic type in a disabled writer.");
        return;
    }

    logInfo(DDSRECORDER_SQL_WRITER, "Writing dynamic type " << dynamic_type.type_name() << ".");

    // Define the SQL statement
    const char* insert_statement = R"(
        INSERT INTO Types (name, information, object, is_ros2_type)
        VALUES (?, ?, ?, ?);
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
    const auto type_name = ros2_types_ ? utils::demangle_if_ros_type(dynamic_type.type_name()) : dynamic_type.type_name();
    const auto is_type_ros2_type = ros2_types_ && type_name != dynamic_type.type_name();

    sqlite3_bind_text(statement, 1, type_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, dynamic_type.type_information().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, dynamic_type.type_object().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 4, is_type_ros2_type ? "true" : "false", -1, SQLITE_TRANSIENT);

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
        const std::vector<SqlMessage>& messages)
{
    if (!enabled_)
    {
        logWarning(DDSRECORDER_SQL_WRITER, "Attempting to write messages in a disabled writer.");
        return;
    }

    logInfo(DDSRECORDER_SQL_WRITER, "Writing << " << messages.size() << " messages.");

    // Define the SQL statement for batch insert
    const char* insert_statement = R"(
        INSERT INTO Messages (writer_guid, sequence_number, data_json, data_cdr, data_cdr_size, topic, type, key, log_time, publish_time)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";

    // Prepare the SQL statement
    sqlite3_stmt* statement;
    const auto prep_ret = sqlite3_prepare_v2(database_, insert_statement, -1, &statement, nullptr);

    if (prep_ret != SQLITE_OK)
    {
        const std::string error_msg = utils::Formatter() << "Failed to prepare SQL statement to write messages: "
                                                         << sqlite3_errmsg(database_);
        sqlite3_finalize(statement);

        logError(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << error_msg);
        throw utils::InconsistencyException(error_msg);
    }

    // Begin transaction
    if (sqlite3_exec(database_, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) != SQLITE_OK)
    {
        const std::string error_msg = utils::Formatter() << "Failed to begin transaction: "
                                                         << sqlite3_errmsg(database_);
        sqlite3_finalize(statement);

        logError(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << error_msg);
        throw utils::InconsistencyException(error_msg);
    }

    for (const auto& message : messages)
    {
        // Bind the SqlMessage to the SQL statement

        // Bind the sample identity
        std::ostringstream writer_guid_ss;
        writer_guid_ss << message.writer_guid;

        sqlite3_bind_text(statement, 1, writer_guid_ss.str().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(statement, 2, message.sequence_number.to64long());

        // Bind the sample data
        std::string data_json = "";
        std::byte* data_cdr = new std::byte{0};
        std::uint32_t data_cdr_size = 0;

        if (data_format_ == DataFormat::both || data_format_ == DataFormat::json)
        {
            data_json = message.data_json;
        }

        if (data_format_ == DataFormat::both || data_format_ == DataFormat::cdr)
        {
            data_cdr = message.get_data_cdr();
            data_cdr_size = message.get_data_cdr_size();
        }

        sqlite3_bind_text(statement, 3, data_json.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_blob(statement, 4, data_cdr, data_cdr_size, SQLITE_TRANSIENT);
        sqlite3_bind_int64(statement, 5, data_cdr_size);

        // Bind the topic data
        sqlite3_bind_text(statement, 6, message.topic.topic_name().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(statement, 7, message.topic.type_name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(statement, 8, message.key.c_str(), -1, SQLITE_TRANSIENT);

        // Bind the time data
        sqlite3_bind_text(statement, 9, to_sql_timestamp(message.log_time).c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(statement, 10, to_sql_timestamp(message.publish_time).c_str(), -1, SQLITE_TRANSIENT);

        // Execute the SQL statement
        const auto step_ret = sqlite3_step(statement);

        if (step_ret != SQLITE_DONE)
        {
            const std::string error_msg = utils::Formatter() << "Failed to write message to SQL database: "
                                                             << sqlite3_errmsg(database_);
            sqlite3_finalize(statement);
            sqlite3_exec(database_, "ROLLBACK;", nullptr, nullptr, nullptr);

            logError(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << error_msg);
            throw utils::InconsistencyException(error_msg);
        }

        // Reset the statement for the next execution
        sqlite3_reset(statement);
    }

    // Commit transaction
    if (sqlite3_exec(database_, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK)
    {
        const std::string error_msg = utils::Formatter() << "Failed to commit transaction: "
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
        INSERT INTO Topics (name, type, qos, is_ros2_topic)
        VALUES (?, ?, ?, ?);
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
    const auto topic_name = ros2_types_ ? utils::demangle_if_ros_topic(topic.topic_name()) : topic.topic_name();
    const auto is_topic_ros2_type = ros2_types_ && topic_name != topic.topic_name();

    sqlite3_bind_text(statement, 1, topic_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, topic.type_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, Serializer::serialize(topic.topic_qos).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 4, is_topic_ros2_type ? "true" : "false", -1, SQLITE_TRANSIENT);

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

// NOTE: The method has to be defined after the definition of write_nts_ for DynamicType
void SqlWriter::close_current_file_nts_()
{
    if (record_types_ && dynamic_types_.size() > 0)
    {
        // Write the dynamic types
        for (const auto& dynamic_type : dynamic_types_)
        {
            write_nts_(dynamic_type);
        }
    }

    sqlite3_close(database_);
    file_tracker_->close_file();
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
