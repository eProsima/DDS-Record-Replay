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


#include <memory>

#include <cpp_utils/event/PeriodicEventHandler.hpp>

#include <ddspipe_core/configuration/MonitorConfiguration.hpp>
#include <ddspipe_core/monitoring/consumers/DdsMonitorParticipantRegistry.hpp>
#include <ddspipe_core/monitoring/Monitor.hpp>
#include <ddspipe_core/monitoring/producers/IMonitorProducer.hpp>

#include <ddsrecorder_participants/library/library_dll.h>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * The \c DdsRecorderMonitor is the entity that reports internal data of the \c DdsRecorder.
 * It manages the \c MonitorProducer instances to produce and consume their data.
 */
class DdsRecorderMonitor : public ddspipe::core::Monitor
{
public:

    /**
     * @brief Construct a \c DdsRecorderMonitor.
     *
     * @param configuration The \c MonitorConfiguration to initialize the \c DdsRecorderMonitor.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    DdsRecorderMonitor(
            const ddspipe::core::MonitorConfiguration& configuration);

    /**
     * @brief Destruct the \c DdsRecorderMonitor.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    virtual ~DdsRecorderMonitor() = default;

    /**
     * @brief Monitorize the DdsRecorder status.
     *
     * The DdsRecorder status is monitored by the \c DdsRecorderStatusMonitorProducer, which is a
     * \c StatusMonitorProducer that produces the \c DdsRecorderMonitoringStatus.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void monitor_status() override;
};

} // namespace participants
} // namespace ddsrecorder
} // namespace eprosima
