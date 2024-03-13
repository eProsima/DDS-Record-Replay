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


#include <ddspipe_core/monitoring/consumers/DdsMonitorConsumer.hpp>
#include <ddspipe_core/monitoring/consumers/StdoutMonitorConsumer.hpp>

#include <ddsrecorder_participants/recorder/monitoring/producers/DdsRecorderStatusMonitorProducer.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

void DdsRecorderStatusMonitorProducer::init(
        const ddspipe::core::MonitorProducerConfiguration& configuration)
{
    // Store whether the producer is enabled
    enabled_ = configuration.enabled;

    if (!enabled_)
    {
        // Don't register the consumers if the producer is not enabled
        return;
    }

    // Store the period so it can be used by the Monitor
    period = configuration.period;

    // Register the type
    fastdds::dds::TypeSupport type(new DdsRecorderMonitoringStatusPubSubType());

    // Create the consumers
    consumers_.push_back(new ddspipe::core::DdsMonitorConsumer<DdsRecorderMonitoringStatus>(configuration.domain.
                    get_value(), configuration.topic_name, type));
    consumers_.push_back(new ddspipe::core::StdoutMonitorConsumer<DdsRecorderMonitoringStatus>());
}

void DdsRecorderStatusMonitorProducer::consume()
{
    if (!enabled_)
    {
        // Don't consume if the producer is not enabled
        return;
    }

    const auto data = save_data_();

    for (auto consumer : consumers_)
    {
        consumer->consume(data);
    }
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

    auto error_status = data_->error_status();
    auto ddsrecorder_error_status = data_->ddsrecorder_error_status();

    if (error == "TYPE_MISMATCH")
    {
        error_status.type_mismatch(true);
    }
    else if (error == "QOS_MISMATCH")
    {
        error_status.qos_mismatch(true);
    }
    else if (error == "MCAP_FILE_CREATION_FAILURE")
    {
        ddsrecorder_error_status.mcap_file_creation_failure(true);
    }
    else if (error == "DISK_FULL")
    {
        ddsrecorder_error_status.disk_full(true);
    }

    data_->error_status(error_status);
    data_->ddsrecorder_error_status(ddsrecorder_error_status);
    data_->has_errors(true);
}

DdsRecorderMonitoringStatus* DdsRecorderStatusMonitorProducer::save_data_() const
{
    return data_;
}

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

} //namespace participants
} //namespace ddsrecorder
} //namespace eprosima
