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
 * @file ConfigurationSubscriber.h
 *
 */

#pragma once

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>

#include "types/configuration/ConfigurationPubSubTypes.h"

struct DataToCheck
{
    unsigned int n_received_msgs;
    std::string type_msg;
    int min_index_msg;
    int max_index_msg;
    double mean_ms_between_msgs;
    double cummulated_ms_between_msgs;
};

/**
 * @brief Class used to group into a single working unit a Subscriber with a DataReader and its listener.
 *
 */
class ConfigurationSubscriber : public eprosima::fastdds::dds::DomainParticipantListener
{
public:

    /**
     * @brief Construct a new Type Lookup Service Subscriber object
     *
     * @param topic_name Name of the DDS Topic
     * @param domain DDS Domain of the DomainParticipant
     */
    ConfigurationSubscriber(
            const std::string& topic_name,
            uint32_t domain,
            DataToCheck& data);

    /**
     * @brief Destroy the Type Lookup Service Publisher object
     *
     */
    virtual ~ConfigurationSubscriber();

    //! DataReader callback executed when a new sample is received
    void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override;

    //! DataReader callback to inform new matches/unmatches with other DataWriters
    void on_subscription_matched(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

    void init_info(
            const std::string& type_name);

    void fill_info(
            Configuration configuration_,
            uint64_t time_arrive_msg);

protected:

    // Fast DDS entities
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Subscriber* subscriber_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::DataReader* datareader_;
    eprosima::fastdds::dds::TypeSupport type_;

    DataToCheck* data_;

    Configuration configuration_;

    //! Number of samples received
    uint32_t samples_;
    //! The time in milliseconds when the previous message arrived
    double prev_time_;
};
