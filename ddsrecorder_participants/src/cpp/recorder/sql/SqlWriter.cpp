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

#include <sqlite/sqlite3.h>

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

#include <filesystem>

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
    , check_interval_(configuration.resource_limits.size_tolerance_/2)
    , size_checkpoint_(configuration.resource_limits.size_tolerance_/4)
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
                    "size (" + utils::from_bytes(configuration_.resource_limits.max_file_size_) + ").");
    }

    const auto filename = file_tracker_->get_current_filename();

    // Create SQLite database
    const auto ret = sqlite3_open(filename.c_str(), &database_);

    if (ret != SQLITE_OK)
    {
        const std::string error_msg = utils::Formatter() << "Failed to open SQL file " << filename
                                                         << " for writing: " << sqlite3_errmsg(database_);
        sqlite3_close(database_);

        EPROSIMA_LOG_ERROR(DDSRECORDER_SQL_WRITER, "FAIL_SQL_OPEN | " << error_msg);
        throw utils::InitializationException(error_msg);
    }

    // Enable WAL mode: appends changes to a separate file before applying them to the main database, reducing the risk of corruption in the event of a crash
    sqlite3_exec(database_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

    // Get the page size for later vacuuming and auto checkpointing
    sqlite3_stmt* stmt;
    const char* query = "PRAGMA page_size;";

    if (sqlite3_prepare_v2(database_, query, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            page_size_ = sqlite3_column_int(stmt, 0);
        }
    } else {
        const std::string error_msg = utils::Formatter() << "Failed to calculate SQL page size: " << sqlite3_errmsg(database_);
        sqlite3_close(database_);

        EPROSIMA_LOG_ERROR(DDSRECORDER_SQL_WRITER, "FAIL_SQL_OPEN | " << error_msg);
        throw utils::InitializationException(error_msg);
    }

    sqlite3_finalize(stmt);

    // Set autocheckpoint every size_checkpoint_ bytes
    const int checkpoint_pages = size_checkpoint_ / page_size_;
    std::string pragma_cmd = "PRAGMA wal_autocheckpoint = " + std::to_string(checkpoint_pages) + ";";
    sqlite3_exec(database_, pragma_cmd.c_str(), nullptr, nullptr, nullptr);

    // Enable Incremental Auto-Vacuum mode (for antifragmentation memory management)
    sqlite3_exec(database_, "PRAGMA auto_vacuum = INCREMENTAL;", nullptr, nullptr, nullptr);

    // Perform an initial VACUUM if needed (only on new databases, as it can be costly)
    sqlite3_exec(database_, "VACUUM;", nullptr, nullptr, nullptr);

    

    // Create Types table
    // NOTE: These tables creation should never fail since the minimum size accounts for them.
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

    written_sql_size_ = MIN_SQL_SIZE;
}

template <>
void SqlWriter::write_nts_(
        const DynamicType& dynamic_type)
{
    if (!enabled_)
    {
        EPROSIMA_LOG_WARNING(DDSRECORDER_SQL_WRITER, "Attempting to write a dynamic type in a disabled writer.");
        return;
    }

    EPROSIMA_LOG_INFO(DDSRECORDER_SQL_WRITER, "Writing dynamic type " << dynamic_type.type_name() << ".");

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

        EPROSIMA_LOG_ERROR(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << error_msg);
        throw utils::InconsistencyException(error_msg);
    }

    // Bind the DynamicType to the SQL statement
    const auto type_name = ros2_types_ ? utils::demangle_if_ros_type(dynamic_type.type_name()) : dynamic_type.type_name();
    const auto is_type_ros2_type = ros2_types_ && type_name != dynamic_type.type_name();

    sqlite3_bind_text(statement, 1, type_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, dynamic_type.type_information().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, dynamic_type.type_object().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 4, is_type_ros2_type ? "true" : "false", -1, SQLITE_TRANSIENT);

    // Calculate the estimated size of this entry
    size_t entry_size = 0;

    entry_size += type_name.size();
    entry_size += dynamic_type.type_information().size();
    entry_size += dynamic_type.type_object().size();
    entry_size += sizeof("false");

    try{
        size_control_(entry_size, true);
    }
    catch (const FullFileException& e)
    {
        sqlite3_finalize(statement);

        EPROSIMA_LOG_INFO(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << e.what());
        throw e;
    }
    catch (const utils::InconsistencyException& e)
    {
        sqlite3_finalize(statement);

        EPROSIMA_LOG_ERROR(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << e.what());
        throw e;
    }

    // Execute the SQL statement
    const auto step_ret = sqlite3_step(statement);

    if (step_ret != SQLITE_DONE)
    {
        const std::string error_msg = utils::Formatter() << "Failed to write dynamic type to SQL database: "
                                                         << sqlite3_errmsg(database_);
        sqlite3_finalize(statement);

        EPROSIMA_LOG_ERROR(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << error_msg);
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
        EPROSIMA_LOG_WARNING(DDSRECORDER_SQL_WRITER, "Attempting to write messages in a disabled writer.");
        return;
    }

    EPROSIMA_LOG_INFO(DDSRECORDER_SQL_WRITER, "Writing << " << messages.size() << " messages.");

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

        EPROSIMA_LOG_ERROR(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << error_msg);
        throw utils::InconsistencyException(error_msg);
    }

    // Begin transaction
    if (sqlite3_exec(database_, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) != SQLITE_OK)
    {
        const std::string error_msg = utils::Formatter() << "Failed to begin transaction: "
                                                         << sqlite3_errmsg(database_);
        sqlite3_finalize(statement);

        EPROSIMA_LOG_ERROR(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << error_msg);
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

        // Calculate the estimated size of this entry
        size_t entry_size = 0;

        entry_size += writer_guid_ss.str().size();
        entry_size += calculate_int_storage_size(message.sequence_number.to64long());
        entry_size += data_json.size();
        entry_size += data_cdr_size;
        entry_size += calculate_int_storage_size(data_cdr_size);
        entry_size += message.topic.topic_name().size();
        entry_size += message.topic.type_name.size();
        entry_size += message.key.size();
        entry_size += to_sql_timestamp(message.log_time).size();
        entry_size += to_sql_timestamp(message.publish_time).size();

        try{
            size_control_(entry_size, false);
        }
        catch (const FullFileException& e)
        {
            sqlite3_finalize(statement);

            EPROSIMA_LOG_INFO(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << e.what());
            throw e;
        }
        catch (const utils::InconsistencyException& e)
        {
            sqlite3_finalize(statement);

            EPROSIMA_LOG_ERROR(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << e.what());
            throw e;
        }

        
        // Execute the SQL statement
        const auto step_ret = sqlite3_step(statement);

        if (step_ret != SQLITE_DONE)
        {
            const std::string error_msg = utils::Formatter() << "Failed to write message to SQL database: "
                                                             << sqlite3_errmsg(database_);
            sqlite3_finalize(statement);
            sqlite3_exec(database_, "ROLLBACK;", nullptr, nullptr, nullptr);

            EPROSIMA_LOG_ERROR(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << error_msg);
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

        EPROSIMA_LOG_ERROR(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << error_msg);
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
        EPROSIMA_LOG_WARNING(DDSRECORDER_SQL_WRITER, "Attempting to write a topic in a disabled writer.");
        return;
    }

    EPROSIMA_LOG_INFO(DDSRECORDER_SQL_WRITER, "Writing topic " << topic.topic_name() << ".");

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

        EPROSIMA_LOG_ERROR(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << error_msg);
        throw utils::InconsistencyException(error_msg);
    }

    // Bind the Topic to the SQL statement
    const auto topic_name = ros2_types_ ? utils::demangle_if_ros_topic(topic.topic_name()) : topic.topic_name();
    const auto topic_qos_serialized = Serializer::serialize(topic.topic_qos);
    const auto is_topic_ros2_type = ros2_types_ && topic_name != topic.topic_name();

    sqlite3_bind_text(statement, 1, topic_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, topic.type_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, topic_qos_serialized.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 4, is_topic_ros2_type ? "true" : "false", -1, SQLITE_TRANSIENT);

    // Calculate the estimated size of this entry
    size_t entry_size = 0;

    entry_size += topic_name.size();
    entry_size += topic.type_name.size();
    entry_size += topic_qos_serialized.size();
    entry_size += sizeof("false");

    try{
        size_control_(entry_size, false);
    }
    catch (const FullFileException& e)
    {
        sqlite3_finalize(statement);

        EPROSIMA_LOG_INFO(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << e.what());
        throw e;
    }
    catch (const utils::InconsistencyException& e)
    {
        sqlite3_finalize(statement);

        EPROSIMA_LOG_ERROR(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << e.what());
        throw e;
    }

    // Execute the SQL statement
    const auto step_ret = sqlite3_step(statement);

    if (step_ret != SQLITE_DONE)
    {
        const std::string error_msg = utils::Formatter() << "Failed to write topic to SQL database: "
                                                        << sqlite3_errmsg(database_);
        sqlite3_finalize(statement);

        EPROSIMA_LOG_ERROR(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << error_msg);
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

    // Checkpoint any remaining data in the WAL file
    sqlite3_wal_checkpoint_v2(database_, nullptr, SQLITE_CHECKPOINT_FULL, nullptr, nullptr);

    file_tracker_->set_current_file_size(written_sql_size_);

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

        EPROSIMA_LOG_ERROR(DDSRECORDER_SQL_WRITER, "FAIL_SQL_OPEN | " << error_msg);
        throw utils::InitializationException(error_msg);
    }
}

std::uint64_t SqlWriter::remove_oldest_entries_(
        const std::uint64_t size_required)
{
    std::uint64_t freed_size = 0;

    while (freed_size < size_required)
    {
        // SQL query to select the oldest message based on publish_time
        const char* select_oldest_statement = R"(
            SELECT rowid, LENGTH(writer_guid), LENGTH(sequence_number), LENGTH(data_json), 
                   LENGTH(data_cdr), data_cdr_size, LENGTH(topic), LENGTH(type), 
                   LENGTH(key), LENGTH(log_time), LENGTH(publish_time)
            FROM Messages
            ORDER BY publish_time ASC
            LIMIT 1;
        )";

        sqlite3_stmt* select_stmt;
        if (sqlite3_prepare_v2(database_, select_oldest_statement, -1, &select_stmt, nullptr) != SQLITE_OK)
        {
            // Failed to prepare select statement
            const std::string error_msg = utils::Formatter() << "Failed to prepare SQL select statement to free space: "
                                                         << sqlite3_errmsg(database_);
            sqlite3_finalize(select_stmt);

            EPROSIMA_LOG_ERROR(DDSRECORDER_SQL_WRITER, "FAIL_SQL_REMOVE | " << error_msg);
            throw utils::InconsistencyException(error_msg);
        }

        // Fetch the oldest entry's size
        if (sqlite3_step(select_stmt) == SQLITE_ROW)
        {
            // Calculate the size of the row data in bytes
            size_t entry_size = 0;
            for (int i = 1; i <= 10; ++i) // Skipping rowid (index 0) and summing lengths of columns
            {
                entry_size += sqlite3_column_int(select_stmt, i);
            }

            // Get the rowid of the entry to delete
            int rowid = sqlite3_column_int(select_stmt, 0);

            // Prepare delete statement
            const char* delete_statement = "DELETE FROM Messages WHERE rowid = ?;";
            sqlite3_stmt* delete_stmt;
            if (sqlite3_prepare_v2(database_, delete_statement, -1, &delete_stmt, nullptr) != SQLITE_OK)
            {
                sqlite3_finalize(delete_stmt);
                sqlite3_finalize(select_stmt);
                return 0; // Failed to prepare delete statement
            }

            // Bind the rowid to the delete statement
            sqlite3_bind_int(delete_stmt, 1, rowid);

            // Execute delete statement
            if (sqlite3_step(delete_stmt) == SQLITE_DONE)
            {
                freed_size += static_cast<std::uint64_t>(entry_size); // Update the freed size
            }

            sqlite3_finalize(delete_stmt);
        }
        else
        {
            sqlite3_finalize(select_stmt);

            // No more rows to delete, unable to free enough space
            EPROSIMA_LOG_ERROR(DDSRECORDER_SQL_WRITER, "FAIL_SQL_REMOVE | No more rows to delete.");
            throw FullFileException("SQL file is full and not removable.", size_required);
        }

        // Reclaim 10 pages after freeing space, adjustable based on observation
        sqlite3_exec(database_, "PRAGMA incremental_vacuum = 10;", nullptr, nullptr, nullptr);

        sqlite3_finalize(select_stmt);
    }

    // Vacuum as many pages as the freed size in bytes
    int pages_to_reclaim = size_required / page_size_;
    if (pages_to_reclaim > 0) {
        sqlite3_exec(database_, ("PRAGMA incremental_vacuum = " + std::to_string(pages_to_reclaim) + ";").c_str(), nullptr, nullptr, nullptr);
    }

    return(freed_size);
}

size_t SqlWriter::calculate_int_storage_size(std::int64_t value) const noexcept
{
    if (value >= 0 && value <= 127) {
        return 1;
    } else if (value >= -32768 && value <= 32767) {
        return 2;
    } else if (value >= -8388608 && value <= 8388607) {
        return 3;
    } else if (value >= -2147483648LL && value <= 2147483647LL) {
        return 4;
    } else if (value >= -1099511627776LL && value <= 1099511627775LL) {
        return 6;
    } else {
        return 8;
    }
}

void SqlWriter::size_control_(size_t entry_size, bool force)
{
    // Add a fixed overhead per row for SQLite storage (headers, etc.)
    constexpr size_t SQLITE_ROW_OVERHEAD = 67;
    entry_size += SQLITE_ROW_OVERHEAD;

    // Check if the entry fits in the current file or it has been forced
    if(written_sql_size_ + entry_size > configuration_.resource_limits.max_file_size_ && !force)
    {
        bool free_space = false;
        // Free space in case of file rotation
        if(configuration_.resource_limits.file_rotation_)
        {
            try
            {
                // To avoid removing entries in every write, we will try to free space for entities_multiplier*entries, in a range [pages_multiplier pages, file_percentage]
                constexpr float file_percentage = 0.05;
                std::uint64_t desired_space = configuration_.resource_limits.max_file_size_*file_percentage;

                std::uint64_t remove_size = remove_oldest_entries_(desired_space);
                written_sql_size_ -= remove_size;
                checked_written_sql_size_ -= remove_size;
                check_file_size_();
                free_space = true;
            }
            catch (const FullFileException& e)
            {
                EPROSIMA_LOG_ERROR(DDSRECORDER_SQL_WRITER, "FAIL_SQL_REMOVE | " << e.what());
                throw e;
            }
            catch (const utils::InconsistencyException& e)
            {
                EPROSIMA_LOG_ERROR(DDSRECORDER_SQL_WRITER, "FAIL_SQL_REMOVE | " << e.what());
                throw e;
            }
        }

        // If there is no free space, close the current file
        if(! free_space)
        {
            EPROSIMA_LOG_INFO(DDSRECORDER_SQL_WRITER, "FAIL_SQL_WRITE | " << "SQL file is full.");
            throw FullFileException(
                STR_ENTRY << "Attempted to write " << utils::from_bytes(entry_size) << " on a SQL of "
                            << utils::from_bytes(written_sql_size_) << " but there is not enough space available: "
                            << utils::from_bytes(configuration_.resource_limits.max_file_size_ - written_sql_size_) << "."
                    , entry_size);
        }
    }

    // Update the written size
    written_sql_size_ += entry_size;

    // Check the actual size of the file if check_interval_ has passed
    if(written_sql_size_ - checked_written_sql_size_ > check_interval_){
        check_file_size_();
    }
}

void SqlWriter::check_file_size_()
{
    const auto filename = file_tracker_->get_current_filename();
    auto file_size = std::filesystem::file_size(filename);
    if(checked_actual_sql_size_ != file_size){
        checked_actual_sql_size_ = file_size;
        written_sql_size_ = file_size;
    }
    checked_written_sql_size_ = written_sql_size_;
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
