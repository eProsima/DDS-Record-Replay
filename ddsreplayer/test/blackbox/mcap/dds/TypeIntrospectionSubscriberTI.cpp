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
#include <functional>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicDataHelper.hpp>
#include <fastrtps/types/TypeObjectFactory.h>
#include <fastrtps/types/TypesBase.h>

#include "TypeIntrospectionSubscriberTI.h"

using namespace eprosima::fastdds::dds;

std::atomic<bool> TypeIntrospectionSubscriber::type_discovered_(false);
std::atomic<bool> TypeIntrospectionSubscriber::type_registered_(false);
std::mutex TypeIntrospectionSubscriber::type_discovered_cv_mtx_;
std::condition_variable TypeIntrospectionSubscriber::type_discovered_cv_;
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
    , topic_name_(topic_name)
    , samples_(0)
{
    ///////////////////////////////
    // Create the DomainParticipant
    DomainParticipantQos pqos;
    pqos.name("TypeLookupService_Participant_Subscriber");

    pqos.wire_protocol().builtin.typelookup_config.use_client = true;
    pqos.wire_protocol().builtin.typelookup_config.use_server = false;

    // Create listener mask so the data do not go to on_data_on_readers from subscriber
    StatusMask mask;
    mask << StatusMask::data_available();
    mask << StatusMask::subscription_matched();
    // No mask for type_information_received

    participant_ = DomainParticipantFactory::get_instance()->create_participant(domain, pqos, this, mask);

    if (participant_ == nullptr)
    {
        throw std::runtime_error("Error creating participant");
    }

    ////////////////////////
    // Create the Subscriber
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

    if (subscriber_ == nullptr)
    {
        throw std::runtime_error("Error creating subscriber");
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

    type_discovered_cv_.notify_all();
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
    // Create a new DynamicData to read the sample
    eprosima::fastrtps::types::DynamicData_ptr new_dynamic_data;
    new_dynamic_data = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(dynamic_type_);

    SampleInfo info;

    // Take next sample until we've read all samples or the application stopped
    while ((reader->take_next_sample(new_dynamic_data.get(), &info) == ReturnCode_t::RETCODE_OK) && !is_stopped())
    {
        if (info.instance_state == ALIVE_INSTANCE_STATE)
        {
            samples_++;

            std::cout << "Message " << samples_ << " received:\n" << std::endl;
            eprosima::fastrtps::types::DynamicDataHelper::print(new_dynamic_data);
            std::cout << "-----------------------------------------------------" << std::endl;

            // Stop if all expecting messages has been received (max_messages number reached)
            if (max_messages_ > 0 && (samples_ >= max_messages_))
            {
                stop();
            }
        }
    }
}

void TypeIntrospectionSubscriber::on_type_information_received(
        eprosima::fastdds::dds::DomainParticipant*,
        const eprosima::fastrtps::string_255 topic_name,
        const eprosima::fastrtps::string_255 type_name,
        const eprosima::fastrtps::types::TypeInformation& type_information)
{
    // First check if the topic received is the one we are expecting
    if (topic_name.to_string() != topic_name_)
    {
        std::cout <<
            "Discovered type information from topic < " << topic_name.to_string() <<
            " > while expecting < " << topic_name_ << " >. Skipping..." << std::endl;
        return;
    }

    // Set the topic type as discovered
    bool already_discovered = type_discovered_.exchange(true);
    if (already_discovered)
    {
        return;
    }

    std::cout <<
        "Found type in topic < " << topic_name_ <<
        " > with name < " << type_name.to_string() <<
        " > by lookup service. Registering..." << std::endl;

    // Create the callback to register the remote dynamic type
    std::function<void(const std::string&, const eprosima::fastrtps::types::DynamicType_ptr)> callback(
            [this]
            (const std::string& name, const eprosima::fastrtps::types::DynamicType_ptr type)
            {
                this->register_remote_type_callback_(name, type);
            });

    // Register the discovered type and create a DataReader on this topic
    participant_->register_remote_type(
        type_information,
        type_name.to_string(),
        callback);
}

void TypeIntrospectionSubscriber::run(
        uint32_t samples)
{
    stop_ = false;
    max_messages_ = samples;

    // Ctrl+C (SIGINT) termination signal handler
    signal(SIGINT, [](int signum)
            {
                std::cout << "\nSIGINT received, stopping Subscriber execution." << std::endl;
                static_cast<void>(signum);
                TypeIntrospectionSubscriber::stop();
            });

    // Wait for type discovery
    std::cout << "Subscriber waiting to discover type for topic < " << topic_name_
        << " >. Press CTRL+C to stop the Subscriber..." << std::endl;

    // Wait until the type is discovered and registered
    {
        std::unique_lock<std::mutex> lck(type_discovered_cv_mtx_);
        type_discovered_cv_.wait(lck, []
                {
                    return is_stopped() || (type_discovered_.load() && type_registered_.load());
                });
    }

    // Check if the application has already been stopped
    if (is_stopped())
    {
        return;
    }

    std::cout <<
        "Subscriber < " << datareader_->guid() <<
        " > listening for data in topic < " << topic_name_ <<
        " > found data type < " << dynamic_type_->get_name() <<
        " >" << std::endl;

    // Wait for expected samples or the user stops the execution
    if (samples > 0)
    {
        std::cout << "Running until " << samples <<
            " samples have been received. Press CTRL+C to stop the Subscriber at any time." << std::endl;
    }
    else
    {
        std::cout << "Press CTRL+C to stop the Subscriber." << std::endl;
    }

    {
        std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
        terminate_cv_.wait(lck, []
                {
                    return is_stopped();
                });
    }

    // Print number of data received
    std::cout <<
        "Subscriber received " << samples_ <<
        " samples." << std::endl;
}

void TypeIntrospectionSubscriber::register_remote_type_callback_(
        const std::string&,
        const eprosima::fastrtps::types::DynamicType_ptr dynamic_type)
{
    ////////////////////
    // Register the type
    TypeSupport type(new eprosima::fastrtps::types::DynamicPubSubType(dynamic_type));
    type.register_type(participant_);

    ///////////////////////
    // Create the DDS Topic
    topic_ = participant_->create_topic(
            topic_name_,
            dynamic_type->get_name(),
            TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return;
    }

    ////////////////////////
    // Create the DataReader
    datareader_ = subscriber_->create_datareader(
            topic_,
            DATAREADER_QOS_DEFAULT,
            this);

    std::cout <<
        "Participant < " << participant_->guid() <<
        " > in domain < " << participant_->get_domain_id() <<
        " > created reader < " << datareader_->guid() <<
        " > in topic < " << topic_name_ <<
        " > with data type < " << dynamic_type->get_name() << " > " <<
        std::endl;

    // Update TypeIntrospectionSubscriber members
    dynamic_type_ = dynamic_type;
    type_discovered_.store(true);
    type_registered_.store(true);
    // Notify that the type has been discovered and registered
    type_discovered_cv_.notify_all();
}
