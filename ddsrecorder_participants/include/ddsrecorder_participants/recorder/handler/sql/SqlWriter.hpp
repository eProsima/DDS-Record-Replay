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
 * @file SqlWriter.hpp
 */

#pragma once

#include <sqlite/sqlite3.h>
#include <string>
#include <vector>

#include <cstdint>
#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollection.hpp>
#include <ddsrecorder_participants/library/library_dll.h>
#include <ddsrecorder_participants/recorder/handler/BaseWriter.hpp>
#include <ddsrecorder_participants/recorder/handler/sql/SqlHandlerConfiguration.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

class DDSRECORDER_PARTICIPANTS_DllAPI SqlWriter : public BaseWriter
{
public:

    SqlWriter(
            const OutputSettings& configuration,
            std::shared_ptr<FileTracker>& file_tracker,
            const bool record_types = true,
            const bool ros2_types = false,
            const DataFormat data_format = DataFormat::both);

    /**
     * @brief Writes data to the output file.
     *
     * @param data Pointer to the data to be written.
     *
     * @throws \c InconsistencyException if there is a database error
     */
    template <typename T>
    void write(
            const T& data);

    /**
     * TODO. danip. MCAP
     * @brief Writes partition to the output file.
     *
     * @param data Pointer to the data to be written.
     *
     * @throws \c InconsistencyException if there is a database error
     */
    void write_partition(
            const std::string& topic_name,
            const std::string& topic_type,
            const std::string& topic_partition);

    /**
     * @brief Updates the dynamic types.
     *
     * The dynamic types are written down when the MCAP file is being closed.
     * This is done so that dynamic types can be updated even when the writer is disabled.
     *
     * @param dynamic_type The dynamic type to add to \c dynamic_types_.
     */
    void update_dynamic_types(
            const DynamicType& dynamic_type);

protected:

    /**
     * @brief Opens a new file.
     *
     * @param min_file_size The minimum size of the file.
     * @throws \c FullDiskException if the disk is full.
     * @throws \c InconsistencyException if \c min_file_size is not enough to open a new empty file.
     * @throws \c InitializationException if the SQL library fails to open the new file.
     */
    void open_new_file_nts_(
            const std::uint64_t min_file_size) override;

    /**
     * @brief Closes the current file.
     *
     * Writes the dynamic types to the SQL file.
     *
     * @throws \c InconsistencyException if closing the current file fails.
     */
    void close_current_file_nts_() override;

    /**
     * @brief Writes data to the SQL file.
     *
     * @param data The data to be written.
     * @throws \c FullFileException if the SQL file is full.
     *
     * @throws \c InconsistencyException if there is a database error
     */
    template <typename T>
    void write_nts_(
            const T& data);

    /**
     * @brief Writes partition data to the SQL file.
     *
     * @param topic_name The name of the topic.
     * @param topic_type The type of the topic.
     * @param topic_partition The type of the partition.
     * @throws \c FullFileException if the SQL file is full.
     *
     * @throws \c InconsistencyException if there is a database error
     */
    void write_nts_(
            const std::string& topic_name,
            const std::string& topic_type,
            const std::string& topic_partition);

    /**
     * @brief Creates a new SQL table.
     *
     * @param table_name The name of the table.
     * @param table_definition The definition of the table.
     *
     * @throws \c InitializationException if the table creation fails.
     */
    void create_sql_table_(
            const std::string& table_name,
            const std::string& table_definition);

    /**
     * @brief Removes oldest entries (publish time wise) from the Messages table.
     *
     * @param size_required The size required to be freed.
     *
     * @throws \c FullDiskException if there are no enough entries to be removed.
     *
     * @throws \c InconsistencyException if it fails to prepare select statement
     *
     * @returns The size freed.
     */
    std::uint64_t remove_oldest_entries_(
            const std::uint64_t size_required);

    /**
     * @brief calculates the storage required (bytes) in an sql database for an integer value
     *
     * @param value The int value to be evaluated
     *
     * @returns The size freed.
     */
    size_t calculate_int_storage_size(
            std::int64_t value) const noexcept;

    /**
     * @brief Checks for free space remaining in the SQL file, if there is not and file rotation is enabled, it will remove the oldest entries.
     *
     * @param entry_size The size of the entry to be written.
     * @param force Whether to force the entry (only used with the dynamic types entry).
     *
     * @throws \c FullFileException if the SQL file is full or there is no space to remove entries.
     *
     * @throws \c InconsistencyException if there is a database error when removing entries.
     */
    void size_control_(
            size_t entry_size,
            bool force);

    /**
     * @brief Checks the actual file size in memory and updates the written_sql_size_ variable.
     */
    void check_file_size_();

    // The SQLite database
    sqlite3* database_;

    // The received dynamic types
    std::vector<DynamicType> dynamic_types_;

    // Whether to format types for ROS 2
    const bool ros2_types_;

    // Whether to record the data in cdr, in json, or in both formats
    const DataFormat data_format_;

    // The size of an empty SQL file
    static constexpr std::uint64_t MIN_SQL_SIZE{33672};

    // The maximum size of the wal file (in bytes) before being checkpointed to the actual database file. This value is set to quarter size_tolerance in constructor
    std::uint64_t size_checkpoint_{500 * 1024};

    // Written (estimated) file size, that takes into account written objects
    std::uint64_t written_sql_size_{MIN_SQL_SIZE};

    /* To have a size checker of the SQL file, we need to check the size of the file every X bytes but the file may not be written yet
     * so we need a variable to "time" the check (variation between written_sql_size_ and checked_written_sql_size_) and another variable
     * to store the size of the file the last time it was checked to know if it has been updated (checked_actual_sql_size_)
     */
    // The value written_sql_size_ had when doing the last check (it is signed to allow negative when freeing space)
    std::int64_t checked_written_sql_size_{0};

    // The actual size of the sql file the last time it was checked
    std::uint64_t checked_actual_sql_size_{0};

    // Threshold of bytes stimated between checks before checking the size of the file again. This value is set to half size_tolerance in constructor
    std::uint64_t check_interval_{500 * 1024}; // 300 KB

    // The size of each page in the SQL file (useful for vacuuming in order to defragment the file)
    std::uint64_t page_size_{0};
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */

#include <ddsrecorder_participants/recorder/handler/sql/impl/SqlWriter.ipp>
