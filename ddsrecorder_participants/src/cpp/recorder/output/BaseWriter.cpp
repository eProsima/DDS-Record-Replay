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
 * @file BaseWriter.cpp
 */

#include <filesystem>
#include <stdexcept>

#include <cpp_utils/Formatter.hpp>
#include <cpp_utils/Log.hpp>
#include <cpp_utils/utils.hpp>

#include <ddsrecorder_participants/recorder/exceptions/FullDiskException.hpp>
#include <ddsrecorder_participants/recorder/exceptions/FullFileException.hpp>
#include <ddsrecorder_participants/recorder/monitoring/producers/DdsRecorderStatusMonitorProducer.hpp>
#include <ddsrecorder_participants/recorder/output/BaseWriter.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

BaseWriter::BaseWriter(
        const OutputSettings& configuration,
        std::shared_ptr<FileTracker>& file_tracker,
        const bool record_types /* = true */,
        const std::uint64_t min_file_size /* = 0 */)
    : configuration_(configuration)
    , file_tracker_(file_tracker)
    , record_types_(record_types)
    , MIN_FILE_SIZE(min_file_size)
{
}

BaseWriter::~BaseWriter()
{
    disable();
}

void BaseWriter::enable()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (enabled_)
    {
        return;
    }

    logInfo(DDSRECORDER_BASE_WRITER,
            "WRITE | Enabling writer.");

    try
    {
        open_new_file_nts_(MIN_FILE_SIZE);
    }
    catch (const FullDiskException& e)
    {
        logError(DDSRECORDER_BASE_WRITER,
                "WRITE | Error opening a new output file: " << e.what());
        on_disk_full_();
    }

    enabled_ = true;
}

void BaseWriter::disable()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!enabled_)
    {
        return;
    }

    logInfo(DDSRECORDER_BASE_WRITER,
            "WRITE | Disabling writer.");

    close_current_file_nts_();

    enabled_ = false;
}

void BaseWriter::set_on_disk_full_callback(
        std::function<void()> on_disk_full_lambda) noexcept
{
    on_disk_full_lambda_ = on_disk_full_lambda;
}

void BaseWriter::on_file_full_nts_(
        const FullFileException& e,
        const std::uint64_t min_file_size)
{
    close_current_file_nts_();

    // Disable the writer in case opening a new file fails
    enabled_ = false;

    if (configuration_.max_file_size == configuration_.max_size)
    {
        // There can only be one file and it's full
        throw FullDiskException(e.what());
    }

    // Open a new file to write the remaining data.
    // Throw an exception if the file cannot be opened.
    open_new_file_nts_(min_file_size + e.data_size_to_write());

    // The file has been opened correctly. Enable the writer.
    enabled_ = true;
}

void BaseWriter::on_disk_full_() const noexcept
{
    monitor_error("DISK_FULL");

    if (on_disk_full_lambda_ != nullptr)
    {
        on_disk_full_lambda_();
    }
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
