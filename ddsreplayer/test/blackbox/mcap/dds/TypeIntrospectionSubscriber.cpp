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

#include "TypeIntrospectionSubscriber.h"

using namespace eprosima::fastdds::dds;

std::atomic<bool> TypeIntrospectionSubscriber::stop_(false);
std::mutex TypeIntrospectionSubscriber::terminate_cv_mtx_;
std::condition_variable TypeIntrospectionSubscriber::terminate_cv_;

TypeIntrospectionSubscriber::TypeIntrospectionSubscriber(
        const std::string& topic_name,
        uint32_t domain)
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , datareader_(nullptr)
    , type_(new HelloWorldPubSubType())
{
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
        "HelloWorld",
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

TypeIntrospectionSubscriber::~TypeIntrospectionSubscriber()
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

bool TypeIntrospectionSubscriber::is_stopped()
{
    return stop_;
}

void TypeIntrospectionSubscriber::stop()
{
    stop_ = true;

    terminate_cv_.notify_all();
}

void TypeIntrospectionSubscriber::on_subscription_matched(
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

void TypeIntrospectionSubscriber::on_data_available(
        DataReader* reader)
{
    SampleInfo info;

    // Take next sample until we've read all samples or the application stopped
    while ((reader->take_next_sample(&hello_, &info) == ReturnCode_t::RETCODE_OK) && !is_stopped())
    {
        if (info.instance_state == ALIVE_INSTANCE_STATE)
        {
            samples_++;

            // Print your structure data here.
            std::cout << "Message " << hello_.message().data() << " " << hello_.index() << " RECEIVED" << std::endl;
            std::cout << "Message " << samples_ << " received:\n" << std::endl;
            std::cout << "-----------------------------------------------------" << std::endl;

            // Stop if all expecting messages has been received (max_messages number reached)
            if (max_messages_ > 0 && (samples_ >= max_messages_))
            {
                stop();
            }
        }
    }
}

void TypeIntrospectionSubscriber::run(
        uint32_t samples)
{
    stop_ = false;
    if (samples > 0)
    {
        std::cout << "Subscriber running until " << samples <<
            " samples have been received. Please press CTRL+C to stop the Subscriber at any time." << std::endl;
    }
    else
    {
        std::cout << "Subscriber running. Please press CTRL+C to stop the Subscriber." << std::endl;
    }
    signal(SIGINT, [](int signum)
            {
                std::cout << "SIGINT received, stopping Subscriber execution." << std::endl;
                static_cast<void>(signum); TypeIntrospectionSubscriber::stop();
            });
    std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
    terminate_cv_.wait(lck, []
            {
                return is_stopped();
            });
}
