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
#include <ddspipe_core/monitoring/producers/StatusMonitorProducer.hpp>

#if FASTRTPS_VERSION_MAJOR < 2 || (FASTRTPS_VERSION_MAJOR == 2 && FASTRTPS_VERSION_MINOR < 13)
    #include <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v1/DdsRecorderMonitoringStatus.h>
    #include \
    <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v1/DdsRecorderMonitoringStatusPubSubTypes.h>
    #include <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v1/DdsRecorderMonitoringStatusTypeObject.h>
#else
    #include <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v2/DdsRecorderMonitoringStatus.h>
    #include \
    <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v2/DdsRecorderMonitoringStatusPubSubTypes.h>
    #include <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v2/DdsRecorderMonitoringStatusTypeObject.h>
#endif // if FASTRTPS_VERSION_MAJOR < 2 || (FASTRTPS_VERSION_MAJOR == 2 && FASTRTPS_VERSION_MINOR < 13)


namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * TODO
 */
class DdsRecorderStatusMonitorProducer : public ddspipe::core::StatusMonitorProducer
{
public:

    // TODO
    void register_consumer(
            std::unique_ptr<ddspipe::core::IMonitorConsumer<DdsRecorderMonitoringStatus>> consumer);

    // TODO
    void produce_and_consume() override;

    // TODO
    void produce() override;

    // TODO
    void consume() override;

    // TODO
    virtual void add_error_to_status(
            const std::string& error) override;

protected:

    // TODO
    void produce_nts_();

    // TODO
    void consume_nts_();

    // TODO
    DdsRecorderMonitoringStatus data_;

    // TODO
    DdsRecorderMonitoringErrorStatus ddsrecorder_error_status_;

    // TODO
    std::vector<std::unique_ptr<ddspipe::core::IMonitorConsumer<DdsRecorderMonitoringStatus>>> consumers_;
};

} // namespace participants
} // namespace ddsrecorder
} // namespace eprosima

namespace std
{

std::ostream& operator <<(
        std::ostream& os,
        const DdsRecorderMonitoringStatus& data);

} // namespace std
