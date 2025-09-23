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
 * @file TypeLookupServiceSubscriber.cpp
 *
 */

#include <csignal>
#include <chrono>

#include <fastdds/dds/core/detail/DDSReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>

#include "../types/configuration/ConfigurationPubSubTypes.hpp"
#include "TestSubscriber.h"

using namespace eprosima::fastdds::dds;

TestSubscriber::TestSubscriber(
        const std::string& topic_name,
        const std::string& type_name,
        uint32_t domain,
        DataToCheck& data)
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , datareader_(nullptr)
    , data_(&data)
    , samples_(0)
    , prev_time_(0)
{
    //TODO Change this to the desired type (ROS2 or DDS)
    eprosima::fastdds::dds::TypeSupport type(new ConfigurationPubSubType());

    ///////////////////////////////
    // Create the DomainParticipant
    DomainParticipantQos pqos;
    pqos.name("Test_Subscriber");

    participant_ = DomainParticipantFactory::get_instance()->create_participant(domain, pqos);

    if (participant_ == nullptr)
    {
        throw std::runtime_error("Error creating participant");
    }

    ////////////////////////
    // REGISTER THE TYPE
    type.register_type(participant_);

    //////////////////////////////
    // INIT DATA TO CHECK STRUCT
    init_info(type.get_type_name());

    ////////////////////////
    // Create the Subscriber

    SubscriberQos subs_qos = SUBSCRIBER_QOS_DEFAULT;
    subs_qos.partition().push_back("*");

    subscriber_ = participant_->create_subscriber(subs_qos, nullptr);

    if (subscriber_ == nullptr)
    {
        throw std::runtime_error("Error creating subscriber");
    }

    ////////////////////////
    // CREATE THE TOPIC
    std::cout << "\t\tNAME: "<< topic_name << "\tTYPE: " << type_name << "\n";

    topic_ = participant_->create_topic(
        topic_name,
        type.get_type_name(),
        TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        throw std::runtime_error("Error creating topic");
    }

    ////////////////////////
    // CREATE THE READER
    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    rqos.history().kind = KEEP_ALL_HISTORY_QOS;

    datareader_ = subscriber_->create_datareader(topic_, rqos, this);
    if (datareader_ == nullptr)
    {
        throw std::runtime_error("Error creating reader");
    }

    std::cout <<
        "Participant < " << participant_->guid() << "> created...\n" <<
        "\t- DDS Domain: " << participant_->get_domain_id() << "\n" <<
        std::endl;
}

TestSubscriber::~TestSubscriber()
{
    if (participant_ != nullptr)
    {
        if (subscriber_ != nullptr)
        {
            if (datareader_ != nullptr)
            {
                subscriber_->delete_datareader(datareader_);
            }
            participant_->delete_subscriber(subscriber_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void TestSubscriber::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "DataReader matched with DataWriter: " << info.last_publication_handle << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "DataReader unmatched with DataWriter: " << info.last_publication_handle << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void TestSubscriber::on_data_available(
        DataReader* reader)
{
    SampleInfo info;

    std::cout << "------------------ WAITING FOR A MESSAGE\n";

    while ((reader->take_next_sample(&configuration_,
            &info) == RETCODE_OK))
    {
        if (info.instance_state == ALIVE_INSTANCE_STATE)
        {
            uint64_t current_time = std::chrono::duration_cast<std::chrono::milliseconds>
                        (std::chrono::system_clock::now().time_since_epoch()).count();

            samples_++;

            fill_info(configuration_, current_time);

            // Print your structure data here.
            std::cout << "Message " << " " << configuration_.index() << " RECEIVED" << std::endl;
            std::cout << "-----------------------------------------------------" << std::endl;
        }
    }

}

void TestSubscriber::init_info(
        const std::string& type_name)
{
    data_->n_received_msgs = 0;
    data_->type_msg = type_name;
    data_->message_msg = "";
    data_->min_index_msg = -1;
    data_->max_index_msg = -1;
    data_->cummulated_ms_between_msgs = -1;
    data_->mean_ms_between_msgs = -1;
}

void TestSubscriber::fill_info(
        Configuration configuration_,
        uint64_t time_arrive_msg)
{
    data_->n_received_msgs++;
    data_->message_msg = configuration_.message().data();

    if (data_->min_index_msg == -1 || data_->min_index_msg > static_cast<int>(configuration_.index()))
    {
        data_->min_index_msg = configuration_.index();
    }
    if (data_->max_index_msg == -1 || data_->max_index_msg < static_cast<int>(configuration_.index()))
    {
        data_->max_index_msg = configuration_.index();
    }

    if (prev_time_ == 0)
    {
        prev_time_ = time_arrive_msg;
    }
    else
    {
        double time_between_msgs = time_arrive_msg - prev_time_;
        prev_time_ = time_arrive_msg;
        if (data_->cummulated_ms_between_msgs == -1)
        {
            data_->cummulated_ms_between_msgs = time_between_msgs;
            data_->mean_ms_between_msgs = time_between_msgs;
        }
        else
        {
            data_->cummulated_ms_between_msgs = data_->cummulated_ms_between_msgs + time_between_msgs;
            data_->mean_ms_between_msgs = data_->cummulated_ms_between_msgs / (data_->n_received_msgs - 1);
        }
    }
}
