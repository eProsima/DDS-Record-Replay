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

#include <sqlite3.h>
#include <string>
#include <vector>

#include <cstdint>
#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollection.hpp>
#include <ddsrecorder_participants/library/library_dll.h>
#include <ddsrecorder_participants/recorder/output/BaseWriter.hpp>
#include <ddsrecorder_participants/recorder/sql/SqlHandlerConfiguration.hpp>

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
     * After a \c FullFileException :
     * - @throws \c InconsistencyException if the allocated space is not enough to close the current file or to open a
     * new one.
     * - @throws \c InitializationException if the output library fails to open a new file.
     */
    template <typename T>
    void write(
            const T& data);

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
     */
    template <typename T>
    void write_nts_(
            const T& data);

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

    // The SQLite database
    sqlite3* database_;

    // The received dynamic types
    std::vector<DynamicType> dynamic_types_;

    // Whether to format types for ROS 2
    const bool ros2_types_;

    // Whether to record the data in cdr, in json, or in both formats
    const DataFormat data_format_;

    // The size of an empty SQL file
    static constexpr std::uint64_t MIN_SQL_SIZE{20480};
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */

#include <ddsrecorder_participants/recorder/sql/impl/SqlWriter.ipp>
