// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file HelloWorldSubscriber.h
 *
 */

#ifndef _EPROSIMA_DDSRECORDER_RESOURCES_DDS_TYPEINTROSPECTION_TYPEINTROSPECTIONSUBSCRIBER_H_
#define _EPROSIMA_DDSRECORDER_RESOURCES_DDS_TYPEINTROSPECTION_TYPEINTROSPECTIONSUBSCRIBER_H_

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>

#include "types/hello_world/HelloWorldPubSubTypes.h"

/**
 * @brief Class used to group into a single working unit a Subscriber with a DataReader and its listener.
 *
 */
class HelloWorldSubscriber : public eprosima::fastdds::dds::DomainParticipantListener
{
public:

    /**
     * @brief Construct a new Type Lookup Service Subscriber object
     *
     * @param topic_name Name of the DDS Topic
     * @param domain DDS Domain of the DomainParticipant
     * @param max_messages Number of messages to be received before triggering termination of execution
     */
    HelloWorldSubscriber(
            const std::string& topic_name,
            uint32_t domain,
            uint32_t max_messages);

    /**
     * @brief Destroy the Type Lookup Service Publisher object
     *
     */
    virtual ~HelloWorldSubscriber();

    /**
     * @brief Run the subscriber until "number" samples are received
     *
     */void run();

    //! Set the maximum number of messages to receive before exiting
    void set_max_messages(
            uint32_t max_messages);

    //! DataReader callback executed when a new sample is received
    void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override;

    //! DataReader callback to inform new matches/unmatches with other DataWriters
    void on_subscription_matched(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

    //! Return the current state of execution
    static bool is_stopped();

    //! Trigger the end of execution
    static void stop();

protected:

    // Fast DDS entities
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Subscriber* subscriber_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::DataReader* datareader_;
    eprosima::fastdds::dds::TypeSupport type_;

    HelloWorld hello_;

    //! Number of DataWriters matched to the associated DataReader
    int matched_;
    //! Number of samples received
    uint32_t samples_;
    //! Number of messages to be received before triggering termination of execution
    uint32_t max_messages_;

    //! Member used for control flow purposes
    static std::atomic<bool> stop_;

    //! Protects terminate condition variable
    static std::mutex terminate_cv_mtx_;

    //! Waits during execution until SIGINT or max_messages_ samples are received
    static std::condition_variable terminate_cv_;
};

#endif /* _EPROSIMA_DDSRECORDER_RESOURCES_DDS_TYPEINTROSPECTION_TYPEINTROSPECTIONSUBSCRIBER_H_ */
