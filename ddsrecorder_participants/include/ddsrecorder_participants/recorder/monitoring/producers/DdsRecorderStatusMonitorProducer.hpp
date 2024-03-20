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

#include <ddsrecorder_participants/library/library_dll.h>

#if FASTRTPS_VERSION_MAJOR < 2 || (FASTRTPS_VERSION_MAJOR == 2 && FASTRTPS_VERSION_MINOR < 13)
    #include <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v1/DdsRecorderMonitoringStatus.h>
    #include \
    <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v1/DdsRecorderMonitoringStatusPubSubTypes.h>
    #include \
    <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v1/DdsRecorderMonitoringStatusTypeObject.h>
#else
    #include <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v2/DdsRecorderMonitoringStatus.h>
    #include \
    <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v2/DdsRecorderMonitoringStatusPubSubTypes.h>
    #include \
    <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v2/DdsRecorderMonitoringStatusTypeObject.h>
#endif // if FASTRTPS_VERSION_MAJOR < 2 || (FASTRTPS_VERSION_MAJOR == 2 && FASTRTPS_VERSION_MINOR < 13)


namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * @brief Producer of the \c DdsRecorderMonitoringStatus.
 *
 * The \c DdsRecorderStatusMonitorProducer produces the \c DdsRecorderMonitoringStatus by gathering data with the
 * \c StatusMonitorProducer's macros:
 * - \c monitor_error
 *
 * The \c DdsRecorderStatusMonitorProducer consumes the \c DdsRecorderMonitoringStatus by using its consumers.
 */
class DdsRecorderStatusMonitorProducer : public ddspipe::core::StatusMonitorProducer
{
public:

    /**
     * @brief Destroy the \c DdsRecorderStatusMonitorProducer.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    virtual ~DdsRecorderStatusMonitorProducer() = default;

    /**
     * @brief Register a consumer.
     *
     * The consumer can be any class that implements the \c IMonitorConsumer interface as long as it is a template class
     * that accepts the \c DdsRecorderMonitoringStatus as a template parameter.
     *
     * @param consumer Consumer to be registered.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void register_consumer(
            std::unique_ptr<ddspipe::core::IMonitorConsumer<DdsRecorderMonitoringStatus>> consumer);

    /**
     * @brief Remove all consumers.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void clear_consumers() override;

    /**
     * @brief Produce and consume the \c DdsRecorderMonitoringStatus.
     *
     * Produces a \c DdsRecorderMonitoringStatus with the data gathered and consumes it.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void produce_and_consume() override;

    /**
     * @brief Produce the \c DdsRecorderMonitoringStatus.
     *
     * Generates a \c DdsRecorderMonitoringStatus with the data gathered by the producer.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void produce() override;

    /**
     * @brief Consume the \c DdsRecorderMonitoringStatus.
     *
     * Calls the consume method of its consumers.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    void consume() override;

    /**
     * @brief Add an error to the \c DdsRecorderMonitoringStatus.
     *
     * Method called by the \c monitor_error macro to.
     *
     * @param error String identifying the error to be added to the \c DdsRecorderMonitoringStatus.
     */
    DDSRECORDER_PARTICIPANTS_DllAPI
    virtual void add_error_to_status(
            const std::string& error) override;

protected:

    // Produce data_.
    void produce_nts_();

    // Consume data_.
    void consume_nts_();

    // The produced data.
    DdsRecorderMonitoringStatus data_;

    // DDS Recorder specific errors gathered by the producer.
    DdsRecorderMonitoringErrorStatus ddsrecorder_error_status_;

    // Vector of consumers of the DdsRecorderMonitoringStatus.
    std::vector<std::unique_ptr<ddspipe::core::IMonitorConsumer<DdsRecorderMonitoringStatus>>> consumers_;
};

} // namespace participants
} // namespace ddsrecorder
} // namespace eprosima

namespace std {

std::ostream& operator <<(
        std::ostream& os,
        const DdsRecorderMonitoringStatus& data);

} // namespace std
