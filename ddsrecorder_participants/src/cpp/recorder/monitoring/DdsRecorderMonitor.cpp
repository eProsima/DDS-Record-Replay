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

#include <fastdds/dds/topic/TypeSupport.hpp>

#include <ddspipe_core/monitoring/consumers/DdsMonitorConsumer.hpp>
#include <ddspipe_core/monitoring/consumers/LogMonitorConsumer.hpp>

#include <ddsrecorder_participants/recorder/monitoring/producers/DdsRecorderStatusMonitorProducer.hpp>

#include <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/DdsRecorderMonitoringStatus.hpp>
#include \
    <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/DdsRecorderMonitoringStatusPubSubTypes.hpp>

#include <ddsrecorder_participants/recorder/monitoring/DdsRecorderMonitor.hpp>


namespace eprosima {
namespace ddsrecorder {
namespace participants {

DdsRecorderMonitor::DdsRecorderMonitor(
        const ddspipe::core::MonitorConfiguration& configuration)
    : ddspipe::core::Monitor(configuration)
{
}

void DdsRecorderMonitor::monitor_status()
{
    EPROSIMA_LOG_INFO(DDSRECORDER_MONITOR, "MONITOR | Registering DdsRecorder Status Monitor Producer.");

    // Initialize the Status Monitor Producer with the DDS Recorder Status
    static auto ddsrecorder_status_producer =
            std::make_unique<ddsrecorder::participants::DdsRecorderStatusMonitorProducer>();

    // Register the type
    fastdds::dds::TypeSupport type(new DdsRecorderMonitoringStatusPubSubType());

    // Initialize the DdsRecorder Status Producer
    ddsrecorder_status_producer->init(configuration_.producers.at(ddspipe::core::STATUS_MONITOR_PRODUCER_ID));

    // Register the consumers
    ddsrecorder_status_producer->register_consumer(
        std::make_unique<ddspipe::core::LogMonitorConsumer<DdsRecorderMonitoringStatus>>());

    if (configuration_.consumers.count(ddspipe::core::STATUS_MONITOR_PRODUCER_ID) > 0)
    {
        ddsrecorder_status_producer->register_consumer(std::make_unique<ddspipe::core::DdsMonitorConsumer<DdsRecorderMonitoringStatus>>(
                    configuration_.consumers[ddspipe::core::STATUS_MONITOR_PRODUCER_ID], registry_,
                    type));
    }

    ddspipe::core::StatusMonitorProducer::init_instance(std::move(ddsrecorder_status_producer));

    // Register the Status Monitor Producer
    auto status_producer = ddspipe::core::StatusMonitorProducer::get_instance();

    register_producer_(status_producer);
}

} //namespace participants
} //namespace ddsrecorder
} //namespace eprosima
