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
#include <ddspipe_core/monitoring/consumers/StdoutMonitorConsumer.hpp>

#include <ddsrecorder_participants/recorder/monitoring/producers/DdsRecorderStatusMonitorProducer.hpp>

#if FASTRTPS_VERSION_MAJOR < 2 || (FASTRTPS_VERSION_MAJOR == 2 && FASTRTPS_VERSION_MINOR < 13)
    #include <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v1/DdsRecorderMonitoringStatus.h>
    #include <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v1/DdsRecorderMonitoringStatusPubSubTypes.h>
#else
    #include <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v2/DdsRecorderMonitoringStatus.h>
    #include <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v2/DdsRecorderMonitoringStatusPubSubTypes.h>
#endif // if FASTRTPS_VERSION_MAJOR < 2 || (FASTRTPS_VERSION_MAJOR == 2 && FASTRTPS_VERSION_MINOR < 13)

#include "DdsRecorderMonitor.hpp"


namespace eprosima {
namespace ddsrecorder {
namespace recorder {

DdsRecorderMonitor::DdsRecorderMonitor(
        const ddspipe::core::MonitorConfiguration& configuration)
        : ddspipe::core::Monitor(configuration)
{
}

void DdsRecorderMonitor::monitorize_status()
{
    // Initialize the Status Monitor Producer with the DDS Recorder Status
    static auto ddsrecorder_status_producer =
            std::make_unique<ddsrecorder::participants::DdsRecorderStatusMonitorProducer>();

    // Register the type
    fastdds::dds::TypeSupport type(new DdsRecorderMonitoringStatusPubSubType());

    // Register the consumers
    ddsrecorder_status_producer->register_consumer(std::make_unique<ddspipe::core::StdoutMonitorConsumer<DdsRecorderMonitoringStatus>>());
    ddsrecorder_status_producer->register_consumer(std::make_unique<ddspipe::core::DdsMonitorConsumer<DdsRecorderMonitoringStatus>>(
            configuration_.consumers["status"], registry_, type));

    ddspipe::core::StatusMonitorProducer::init_instance(std::move(ddsrecorder_status_producer));

    // Register the Status Monitor Producer
    auto status_producer = ddspipe::core::StatusMonitorProducer::get_instance();
    status_producer->init(configuration_.producers["status"]);

    register_producer_(status_producer);
}

} //namespace recorder
} //namespace ddsrecorder
} //namespace eprosima
