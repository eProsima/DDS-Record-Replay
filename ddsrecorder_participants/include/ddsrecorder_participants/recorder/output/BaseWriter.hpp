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
 * @file BaseWriter.hpp
 */

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>

#include <fastdds/rtps/common/SerializedPayload.hpp>

#include <ddsrecorder_participants/constants.hpp>
#include <ddsrecorder_participants/library/library_dll.h>
#include <ddsrecorder_participants/recorder/exceptions/FullFileException.hpp>
#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>
#include <ddsrecorder_participants/recorder/output/OutputSettings.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

class DDSRECORDER_PARTICIPANTS_DllAPI BaseWriter
{
public:

    BaseWriter(
            const OutputSettings& configuration,
            std::shared_ptr<FileTracker>& file_tracker,
            const bool record_types = true,
            const std::uint64_t min_file_size = 0);

    virtual ~BaseWriter();

    /**
     * @brief Enable the writer.
     *
     * After a \c FullFileException :
     * - @throws \c InconsistencyException if the allocated space is not enough to close the current file or to open a
     * new one.
     * - @throws \c InitializationException if the output library fails to open a new file.
     */
    virtual void enable();

    /**
     * @brief Disable the writer.
     *
     * After a \c FullFileException :
     * - @throws \c InconsistencyException if the allocated space is not enough to close the current file or to open a
     * new one.
     */
    virtual void disable();

    /**
     * @brief Sets the callback to be called when the disk is full.
     *
     * Sets \c on_disk_full_lambda_ to \c on_disk_full_lambda.
     */
    void set_on_disk_full_callback(
            std::function<void()> on_disk_full_lambda) noexcept;

protected:

    /**
     * @brief Opens a new file.
     *
     * @param min_file_size The minimum size of the file.
     */
    virtual void open_new_file_nts_(
            const std::uint64_t min_file_size) = 0;

    /**
     * @brief Closes the current file.
     */
    virtual void close_current_file_nts_() = 0;

    /**
     * @brief Function called when the output file is full.
     *
     * The function closes the current file and tries to open a new one if allowed.
     *
     * @throws \c FullDiskException if the disk is full and the \c BaseWriter can't open any more files.
     * @throws \c InconsistencyException if \c min_file_size is not enough to write the file's minimum information.
     * @throws \c InitializationException if the output library fails to open the new file.
     *
     * @param e The exception thrown.
     * @param min_file_size The minimum size of an empty file.
     */
    void on_file_full_nts_(
            const FullFileException& e,
            const std::uint64_t min_file_size);

    /**
     * @brief Function called when the disk is full.
     */
    void on_disk_full_() const noexcept;

    // The configuration for the class
    const OutputSettings configuration_;

    // Track the files written by the output library
    std::shared_ptr<FileTracker> file_tracker_;

    // Whether to record the types
    const bool record_types_{false};

    // The mutex to protect the calls to the public methods
    std::mutex mutex_;

    // Whether the writer can write to the output library
    bool enabled_{false};

    //! Lambda to call when the disk is full
    std::function<void()> on_disk_full_lambda_;

    // The size of an empty output file
    const std::uint64_t MIN_FILE_SIZE{0};
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
