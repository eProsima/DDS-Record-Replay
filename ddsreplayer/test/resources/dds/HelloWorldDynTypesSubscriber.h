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
 * @file HelloWorldDynTypesSubscriber.h
 *
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>

#include "DataToCheck.hpp"

/**
 * @brief Class used to group into a single working unit a Subscriber with a DataReader and its listener.
 *
 */
class HelloWorldDynTypesSubscriber : public eprosima::fastdds::dds::DomainParticipantListener
{
public:

    /**
     * @brief Construct a new Type Lookup Service Subscriber object
     *
     * @param topic_name Name of the DDS Topic
     * @param domain DDS Domain of the DomainParticipant
     */
    HelloWorldDynTypesSubscriber(
            const std::string& topic_name,
            uint32_t domain,
            DataToCheck& data);

    /**
     * @brief Destroy the Type Lookup Service Publisher object
     *
     */
    virtual ~HelloWorldDynTypesSubscriber();

    //! DataReader callback executed when a new sample is received
    void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override;

    //! DataReader callback to inform new matches/unmatches with other DataWriters
    void on_subscription_matched(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

    //! Callback to receive the remote data type information
    virtual void on_type_information_received(
            eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastrtps::string_255 topic_name,
            const eprosima::fastrtps::string_255 type_name,
            const eprosima::fastrtps::types::TypeInformation& type_information) override;

    void init_info(
            const std::string& type_name);

    void fill_info(
            int index,
            const std::string& message,
            uint64_t time_arrive_msg);

protected:

    /**
     * @brief Custom callback to register the type, create the topic and create the DataReader once the data
     * type information is received.
     *
     */
    void register_remote_type_callback_(
            const std::string& name,
            const eprosima::fastrtps::types::DynamicType_ptr dynamic_type);

    // Fast DDS entities
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Subscriber* subscriber_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::DataReader* datareader_;

    DataToCheck* data_;

    //! Name of the DDS Topic
    std::string topic_name_;
    //! Name of the received DDS Topic type
    std::string type_name_;
    //! DynamicType generated with the received type information
    eprosima::fastrtps::types::DynamicType_ptr dynamic_type_;

    //! Number of samples received
    uint32_t samples_;
    //! The time in milliseconds when the previous message arrived
    double prev_time_;

    //! Atomic variables to check whether the type has been discovered and registered
    static std::atomic<bool> type_discovered_;
    static std::atomic<bool> type_registered_;

    //! Protects type_discovered condition variable
    static std::mutex type_discovered_cv_mtx_;
    //! Waits until a type has been discovered
    static std::condition_variable type_discovered_cv_;

};
