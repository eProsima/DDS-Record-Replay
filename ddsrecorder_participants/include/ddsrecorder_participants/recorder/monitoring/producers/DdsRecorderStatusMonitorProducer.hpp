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
//
#pragma once

#include <mutex>

#include <ddspipe_core/configuration/MonitorProducerConfiguration.hpp>
#include <ddspipe_core/monitoring/consumers/IMonitorConsumer.hpp>
#include <ddspipe_core/monitoring/producers/IMonitorProducer.hpp>
#include <ddspipe_core/monitoring/producers/StatusMonitorProducer.hpp>

#if FASTRTPS_VERSION_MAJOR < 2 || (FASTRTPS_VERSION_MAJOR == 2 && FASTRTPS_VERSION_MINOR < 13)
    #include <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v1/DdsRecorderMonitoringStatus.h>
    #include <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v1/DdsRecorderMonitoringStatusPubSubTypes.h>
#else
    #include <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v2/DdsRecorderMonitoringStatus.h>
    #include <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v2/DdsRecorderMonitoringStatusPubSubTypes.h>
#endif // if FASTRTPS_VERSION_MAJOR < 2 || (FASTRTPS_VERSION_MAJOR == 2 && FASTRTPS_VERSION_MINOR < 13)

// Monitoring API:

// DDSPIPE MONITOR MACROS

namespace eprosima {
namespace ddspipe {
namespace core {

/**
 * TODO
 */
class DdsRecorderStatusMonitorProducer : public StatusMonitorProducer
{
public:

    // TODO
    void init(const MonitorProducerConfiguration& configuration) override;

    // TODO
    void consume() override;

    // TODO
    virtual void add_error_to_status(
            const std::string& error) override;

protected:

    // TODO
    DdsRecorderMonitoringStatus* save_data_() const;

    // TODO
    DdsRecorderMonitoringStatus* data_ = new DdsRecorderMonitoringStatus();

    // TODO
    std::vector<IMonitorConsumer<DdsRecorderMonitoringStatus>*> consumers_;
};

std::ostream& operator<<(std::ostream& os, const DdsRecorderMonitoringStatus& data);

} // namespace core
} // namespace ddspipe
} // namespace eprosima
