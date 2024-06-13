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
 * @file HelloWorldDynTypesSubscriber.cpp
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

#include "HelloWorldDynTypesSubscriber.h"

using namespace eprosima::fastdds::dds;

std::atomic<bool> HelloWorldDynTypesSubscriber::type_discovered_(false);
std::atomic<bool> HelloWorldDynTypesSubscriber::type_registered_(false);
std::mutex HelloWorldDynTypesSubscriber::type_discovered_cv_mtx_;
std::condition_variable HelloWorldDynTypesSubscriber::type_discovered_cv_;

HelloWorldDynTypesSubscriber::HelloWorldDynTypesSubscriber(
        const std::string& topic_name,
        uint32_t domain,
        DataToCheck& data)
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , datareader_(nullptr)
    , data_(&data)
    , topic_name_(topic_name)
    , samples_(0)
    , prev_time_(0)
{
    ///////////////////////////////
    // Create the DomainParticipant
    DomainParticipantQos pqos;
    pqos.name("HelloWorldDynTypes_Subscriber");

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
}

HelloWorldDynTypesSubscriber::~HelloWorldDynTypesSubscriber()
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

void HelloWorldDynTypesSubscriber::on_subscription_matched(
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

void HelloWorldDynTypesSubscriber::on_data_available(
        DataReader* reader)
{
    // Create a new DynamicData to read the sample
    eprosima::fastrtps::types::DynamicData_ptr new_dynamic_data;
    new_dynamic_data = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(dynamic_type_);

    SampleInfo info;

    // Take next sample
    while ((reader->take_next_sample(new_dynamic_data.get(),
            &info) == ReturnCode_t::RETCODE_OK))
    {
        if (info.instance_state == ALIVE_INSTANCE_STATE)
        {
            uint64_t current_time = std::chrono::duration_cast<std::chrono::milliseconds>
                        (std::chrono::system_clock::now().time_since_epoch()).count();

            samples_++;

            if (new_dynamic_data->get_name() == "std_msgs::msg::dds_::String_")
            {
                std::string message = new_dynamic_data->get_string_value(0);

                fill_info(static_cast<int>(0), message, current_time);
            }
            else if (new_dynamic_data->get_name() == "HelloWorld")
            {
                int32_t index = new_dynamic_data->get_uint32_value(0);
                std::string message = new_dynamic_data->get_string_value(1);

                fill_info(static_cast<int>(index), message, current_time);
            }

            std::cout << "Message " << samples_ << " received:\n" << std::endl;
            eprosima::fastrtps::types::DynamicDataHelper::print(new_dynamic_data);
            std::cout << "-----------------------------------------------------" << std::endl;
        }
    }
}

void HelloWorldDynTypesSubscriber::on_type_information_received(
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

    bool already_discovered = type_discovered_.exchange(true);
    if (already_discovered)
    {
        return;
    }

    std::string type_name_ = type_name.to_string();
    const eprosima::fastrtps::types::TypeIdentifier* type_identifier = nullptr;
    const eprosima::fastrtps::types::TypeObject* type_object = nullptr;
    eprosima::fastrtps::types::DynamicType_ptr dynamic_type(nullptr);

    // Check if complete identifier already present in factory
    type_identifier =
            eprosima::fastrtps::types::TypeObjectFactory::get_instance()->get_type_identifier(type_name_, true);
    if (type_identifier)
    {
        type_object = eprosima::fastrtps::types::TypeObjectFactory::get_instance()->get_type_object(type_name_, true);
    }

    // If complete not found, try with minimal
    if (!type_object)
    {
        type_identifier = eprosima::fastrtps::types::TypeObjectFactory::get_instance()->get_type_identifier(type_name_,
                        false);
        if (type_identifier)
        {
            type_object = eprosima::fastrtps::types::TypeObjectFactory::get_instance()->get_type_object(type_name_,
                            false);
        }
    }

    // Build dynamic type if type identifier and object found in factory
    if (type_identifier && type_object)
    {
        dynamic_type = eprosima::fastrtps::types::TypeObjectFactory::get_instance()->build_dynamic_type(type_name_,
                        type_identifier,
                        type_object);
    }

    if (!dynamic_type)
    {
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
    else
    {
        register_remote_type_callback_(type_name_, dynamic_type);
    }
}

void HelloWorldDynTypesSubscriber::register_remote_type_callback_(
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

    init_info(dynamic_type->get_name());

    ////////////////////////
    // Create the DataReader
    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    rqos.history().kind = KEEP_ALL_HISTORY_QOS;

    // WARNING: subscriber should already have been created (in object's constructor)
    datareader_ = subscriber_->create_datareader(
        topic_,
        rqos,
        this);

    std::cout <<
        "Participant < " << participant_->guid() <<
        " > in domain < " << participant_->get_domain_id() <<
        " > created reader < " << datareader_->guid() <<
        " > in topic < " << topic_name_ <<
        " > with data type < " << dynamic_type->get_name() << " > " <<
        std::endl;

    // Update HelloWorldDynTypesSubscriber members
    dynamic_type_ = dynamic_type;
    type_discovered_.store(true);
    type_registered_.store(true);
    // Notify that the type has been discovered and registered
    type_discovered_cv_.notify_all();
}

void HelloWorldDynTypesSubscriber::init_info(
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

void HelloWorldDynTypesSubscriber::fill_info(
        int index,
        const std::string& message,
        uint64_t time_arrive_msg)
{
    data_->n_received_msgs++;
    data_->message_msg = message;
    if (data_->min_index_msg == -1 || data_->min_index_msg > index)
    {
        data_->min_index_msg = index;
    }
    if (data_->max_index_msg == -1 || data_->max_index_msg < index)
    {
        data_->max_index_msg = index;
    }

    if (prev_time_ == 0u)
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
