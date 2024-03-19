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


#include <ddsrecorder_participants/recorder/monitoring/producers/DdsRecorderStatusMonitorProducer.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

void DdsRecorderStatusMonitorProducer::register_consumer(
        std::unique_ptr<ddspipe::core::IMonitorConsumer<DdsRecorderMonitoringStatus>> consumer)
{
    if (!enabled_)
    {
        logWarning(DDSRECORDER_MONITOR, "MONITOR | Not registering consumer " << consumer->get_name() << " on "
                "DdsRecorderStatusMonitorProducer since the DdsRecorderStatusMonitorProducer is disabled.");

        return;
    }

    logInfo(DDSRECORDER_MONITOR, "MONITOR | Registering consumer " << consumer->get_name() << " on "
            "DdsRecorderStatusMonitorProducer.");

    consumers_.push_back(std::move(consumer));
}

void DdsRecorderStatusMonitorProducer::produce_and_consume()
{
    if (!enabled_)
    {
        // Don't produce and consume if the producer is not enabled
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    produce_nts_();
    consume_nts_();
}

void DdsRecorderStatusMonitorProducer::produce()
{
    if (!enabled_)
    {
        // Don't consume if the producer is not enabled
        return;
    }

    // Take the lock to prevent:
    //      1. Changing the data while it's being saved.
    //      2. Simultaneous calls to produce.
    std::lock_guard<std::mutex> lock(mutex_);

    produce_nts_();
}

void DdsRecorderStatusMonitorProducer::consume()
{
    if (!enabled_)
    {
        // Don't consume if the producer is not enabled
        return;
    }

    // Take the lock to prevent:
    //      1. Changing the data while it's being saved.
    //      2. Simultaneous calls to consume.
    std::lock_guard<std::mutex> lock(mutex_);

    consume_nts_();
}

void DdsRecorderStatusMonitorProducer::add_error_to_status(
        const std::string& error)
{
    if (!enabled_)
    {
        // Don't save the data if the producer is not enabled
        return;
    }

    // Take the lock to prevent:
    //      1. Changing the data while it's being saved.
    //      2. Simultaneous calls to add_error_to_status.
    std::lock_guard<std::mutex> lock(mutex_);

    logInfo(DDSPIPE_MONITOR, "MONITOR | Adding error " << error << " to status.");

    if (error == "TYPE_MISMATCH")
    {
        error_status_.type_mismatch(true);
    }
    else if (error == "QOS_MISMATCH")
    {
        error_status_.qos_mismatch(true);
    }
    else if (error == "MCAP_FILE_CREATION_FAILURE")
    {
        ddsrecorder_error_status_.mcap_file_creation_failure(true);
    }
    else if (error == "DISK_FULL")
    {
        ddsrecorder_error_status_.disk_full(true);
    }

    has_errors_  = true;
}

void DdsRecorderStatusMonitorProducer::produce_nts_()
{
    logInfo(DDSPIPE_MONITOR, "MONITOR | Producing DdsRecorderMonitoringStatus.");

    data_.error_status(error_status_);
    data_.ddsrecorder_error_status(ddsrecorder_error_status_);
    data_.has_errors(has_errors_);
}

void DdsRecorderStatusMonitorProducer::consume_nts_()
{
    logInfo(DDSPIPE_MONITOR, "MONITOR | Consuming DdsRecorderMonitoringStatus.");

    for (auto& consumer : consumers_)
    {
        consumer->consume(data_);
    }
}

} //namespace participants
} //namespace ddsrecorder
} //namespace eprosima

namespace std
{

std::ostream& operator <<(
        std::ostream& os,
        const DdsRecorderMonitoringStatus& data)
{
    os << "DdsRecorder Monitoring Status: [";

    bool is_first_error = true;

    auto print_error = [&](const std::string& error)
            {
                if (!is_first_error)
                {
                    os << ", ";
                }

                os << error;
                is_first_error = false;
            };

    const auto& status = data.ddsrecorder_error_status();

    if (status.mcap_file_creation_failure())
    {
        print_error("MCAP_FILE_CREATION_FAILURE");
    }

    if (status.disk_full())
    {
        print_error("DISK_FULL");
    }

    if (data.error_status().type_mismatch())
    {
        print_error("TYPE_MISMATCH");
    }

    if (data.error_status().qos_mismatch())
    {
        print_error("QOS_MISMATCH");
    }

    os << "]";

    return os;
}

} // namespace std
