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

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>

#include "HelloWorldSubscriber.h"

using namespace eprosima::fastdds::dds;

void init_info(
        DataToCheck* data,
        const std::string& type_name);
void fill_info(
        DataToCheck* data,
        HelloWorld hello_,
        uint64_t time_arrive_msg);

std::atomic<bool> HelloWorldSubscriber::stop_(false);
std::mutex HelloWorldSubscriber::terminate_cv_mtx_;
std::condition_variable HelloWorldSubscriber::terminate_cv_;

HelloWorldSubscriber::HelloWorldSubscriber(
        const std::string& topic_name,
        uint32_t domain,
        uint32_t max_messages,
        DataToCheck& data)
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , datareader_(nullptr)
    , type_(new HelloWorldPubSubType())
{
    samples_ = 0;

    set_max_messages(max_messages);

    data_ = &data;

    ///////////////////////////////
    // Create the DomainParticipant
    DomainParticipantQos pqos;
    pqos.name("TypeIntrospection_Subscriber");

    pqos.wire_protocol().builtin.typelookup_config.use_client = true;
    pqos.wire_protocol().builtin.typelookup_config.use_server = false;

    participant_ = DomainParticipantFactory::get_instance()->create_participant(domain, pqos);

    if (participant_ == nullptr)
    {
        throw std::runtime_error("Error creating participant");
    }

    ////////////////////////
    // REGISTER THE TYPE
    type_.register_type(participant_);

    //////////////////////////////
    // INIT DATA TO CHECK STRUCT
    init_info(data_, type_.get_type_name());

    ////////////////////////
    // Create the Subscriber
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

    if (subscriber_ == nullptr)
    {
        throw std::runtime_error("Error creating subscriber");
    }

    ////////////////////////
    // CREATE THE TOPIC
    topic_ = participant_->create_topic(
        topic_name,
        type_.get_type_name(),
        TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        throw std::runtime_error("Error creating topic");
    }

    ////////////////////////
    // CREATE THE READER
    datareader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, this);
    if (datareader_ == nullptr)
    {
        throw std::runtime_error("Error creating reader");
    }

    std::cout <<
        "Participant < " << participant_->guid() << "> created...\n" <<
        "\t- DDS Domain: " << participant_->get_domain_id() << "\n" <<
        std::endl;
}

HelloWorldSubscriber::~HelloWorldSubscriber()
{
    if (participant_ != nullptr)
    {
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        if (subscriber_ != nullptr)
        {
            if (datareader_ != nullptr)
            {
                subscriber_->delete_datareader(datareader_);
            }
            participant_->delete_subscriber(subscriber_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

bool HelloWorldSubscriber::is_stopped()
{
    return stop_;
}

void HelloWorldSubscriber::stop()
{
    stop_ = true;

    terminate_cv_.notify_all();
}

void HelloWorldSubscriber::set_max_messages(
        uint32_t max_messages)
{
    max_messages_ = max_messages;
}

void HelloWorldSubscriber::on_subscription_matched(
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

void HelloWorldSubscriber::on_data_available(
        DataReader* reader)
{
    SampleInfo info;

    while ((reader->take_next_sample(&hello_, &info) == ReturnCode_t::RETCODE_OK) && !is_stopped())
    {
        if (info.instance_state == ALIVE_INSTANCE_STATE)
        {
            uint64_t current_time = std::chrono::duration_cast<std::chrono::milliseconds>
                        (std::chrono::system_clock::now().time_since_epoch()).count();

            samples_++;

            fill_info(data_, hello_, current_time);

            // Print your structure data here.
            std::cout << "Message " << " " << hello_.index() << " RECEIVED" << std::endl;
            std::cout << "-----------------------------------------------------" << std::endl;

            // Stop if all expecting messages has been received (max_messages number reached)
            if (max_messages_ > 0 && (samples_ >= max_messages_))
            {
                stop();
            }
        }
    }
}

void HelloWorldSubscriber::run()
{
    stop_ = false;

    std::cout << "Subscriber running. Please press CTRL+C to stop the Subscriber." << std::endl;

    signal(SIGINT, [](int signum)
            {
                std::cout << "SIGINT received, stopping Subscriber execution." << std::endl;
                static_cast<void>(signum); HelloWorldSubscriber::stop();
            });
    std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
    terminate_cv_.wait(lck, []
            {
                return is_stopped();
            });
}

void init_info(
        DataToCheck* data,
        const std::string& type_name)
{
    data->n_received_msgs = 0;
    data->type_msg = type_name;
    data->message_msg = "";
    data->min_index_msg = -1;
    data->max_index_msg = -1;
    data->hz_msgs = -1;
}

uint64_t prev_time = 0;
void fill_info(
        DataToCheck* data,
        HelloWorld hello_,
        uint64_t time_arrive_msg)
{
    data->n_received_msgs++;
    data->message_msg = hello_.message();
    if (data->min_index_msg == -1 || data->min_index_msg > hello_.index())
    {
        data->min_index_msg = hello_.index();
    }
    if (data->max_index_msg == -1 || data->max_index_msg < hello_.index())
    {
        data->max_index_msg = hello_.index();
    }

    if (prev_time == 0)
    {
        prev_time = time_arrive_msg;
    }
    else
    {
        uint64_t time_between_msgs = time_arrive_msg - prev_time;
        if (data->hz_msgs == -1)
        {
            data->hz_msgs = time_between_msgs;
        }
        else
        {
            data->max_index_msg == (data->max_index_msg + time_between_msgs) / 2.0;
        }
    }

}
