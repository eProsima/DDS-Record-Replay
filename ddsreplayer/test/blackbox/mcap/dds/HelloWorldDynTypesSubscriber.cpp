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

#include <fastdds/dds/core/detail/DDSReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/xtypes/utils.hpp>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>

#include "HelloWorldDynTypesSubscriber.h"

using namespace eprosima;
using namespace fastdds::dds;

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

    // pqos.wire_protocol().builtin.typelookup_config.use_client = true;
    // pqos.wire_protocol().builtin.typelookup_config.use_server = false;

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
    fastdds::dds::traits<fastdds::dds::DynamicData>::ref_type new_dynamic_data;
    new_dynamic_data = fastdds::dds::DynamicDataFactory::get_instance()->create_data(dynamic_type_);

    SampleInfo info;

    // Take next sample
    while ((reader->take_next_sample(new_dynamic_data.get(),
            &info) == fastdds::dds::RETCODE_OK))
    {
        if (info.instance_state == ALIVE_INSTANCE_STATE)
        {
            uint64_t current_time = std::chrono::duration_cast<std::chrono::milliseconds>
                        (std::chrono::system_clock::now().time_since_epoch()).count();

            samples_++;

            if (dynamic_type_->get_name() == "std_msgs::msg::dds_::String_")
            {
                std::string message;
                new_dynamic_data->get_string_value(message, 0);

                fill_info(static_cast<int>(0), message, current_time);
            }
            else if (dynamic_type_->get_name() == "HelloWorld")
            {
                uint32_t index;
                new_dynamic_data->get_uint32_value(index, 0);
                std::string message;
                new_dynamic_data->get_string_value(message, 1);

                fill_info(static_cast<int>(index), message, current_time);
            }

            std::cout << "Message " << samples_ << " received:\n" << std::endl;
            std::stringstream ss;
            ss << std::setw(4);
            auto ret = fastdds::dds::json_serialize(new_dynamic_data, ss, fastdds::dds::DynamicDataJsonFormat::EPROSIMA);
            std::cout << ss.str() << std::endl;
            std::cout << "-----------------------------------------------------" << std::endl;
        }
    }
}

void HelloWorldDynTypesSubscriber::on_data_writer_discovery(
        fastdds::dds::DomainParticipant*,
        fastrtps::rtps::WriterDiscoveryInfo&& info,
        bool&)
{
    fastrtps::rtps::WriterProxyData proxy_copy(info.info);

    // Get type information
    const auto type_info = proxy_copy.type_information().type_information;
    const auto type_name = proxy_copy.typeName();
    const auto topic_name = proxy_copy.topicName();

    notify_type_discovered_(type_info, type_name, topic_name);
}

void HelloWorldDynTypesSubscriber::notify_type_discovered_(
            const fastdds::dds::xtypes::TypeInformation& type_info,
            const fastcdr::string_255& type_name,
            const fastcdr::string_255& topic_name)
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

    const auto type_identifier = type_info.complete().typeid_with_size().type_id();
    fastdds::dds::xtypes::TypeObject dyn_type_object;
    if (fastdds::dds::RETCODE_OK != fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
            type_identifier,
            dyn_type_object))
    {
        return;
    }

    // Create Dynamic Type
    fastdds::dds::DynamicType::_ref_type dyn_type = fastdds::dds::DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
                dyn_type_object)->build();
    if (!dyn_type)
    {
        return;
    }

    // Notify type_identifier and its associated tyme_name.
    // NOTE: We assume each type_name corresponds to only one type_identifier
    // logInfo("Participant " << this->id() << " discovered type object " << dyn_type->get_name());

    // Register DynamicType
    register_remote_type_callback_(type_name_, dyn_type);
}

void HelloWorldDynTypesSubscriber::register_remote_type_callback_(
        const std::string&,
        const fastdds::dds::traits<fastdds::dds::DynamicType>::ref_type dynamic_type)
{
    ////////////////////
    // Register the type
    TypeSupport type(new fastdds::dds::DynamicPubSubType(dynamic_type));
    type.register_type(participant_);

    ///////////////////////
    // Create the DDS Topic
    topic_ = participant_->create_topic(
        topic_name_,
        dynamic_type->get_name().to_string(),
        TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return;
    }

    init_info(dynamic_type->get_name().to_string());

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

// void HelloWorldDynTypesSubscriber::on_type_information_received(
//         fastdds::dds::DomainParticipant*,
//         const fastcdr::string_255 topic_name,
//         const fastcdr::string_255 type_name,
//         const fastrtps::types::TypeInformation& type_information)
// {
//     // First check if the topic received is the one we are expecting
//     if (topic_name.to_string() != topic_name_)
//     {
//         std::cout <<
//             "Discovered type information from topic < " << topic_name.to_string() <<
//             " > while expecting < " << topic_name_ << " >. Skipping..." << std::endl;
//         return;
//     }

//     bool already_discovered = type_discovered_.exchange(true);
//     if (already_discovered)
//     {
//         return;
//     }

//     std::string type_name_ = type_name.to_string();
//     const fastrtps::types::TypeIdentifier* type_identifier = nullptr;
//     const fastrtps::types::TypeObject* type_object = nullptr;
//     fastrtps::types::DynamicType_ptr dynamic_type(nullptr);

//     // Check if complete identifier already present in factory
//     type_identifier =
//             fastrtps::types::TypeObjectFactory::get_instance()->get_type_identifier(type_name_, true);
//     if (type_identifier)
//     {
//         type_object = fastrtps::types::TypeObjectFactory::get_instance()->get_type_object(type_name_, true);
//     }

//     // If complete not found, try with minimal
//     if (!type_object)
//     {
//         type_identifier = fastrtps::types::TypeObjectFactory::get_instance()->get_type_identifier(type_name_,
//                         false);
//         if (type_identifier)
//         {
//             type_object = fastrtps::types::TypeObjectFactory::get_instance()->get_type_object(type_name_,
//                             false);
//         }
//     }

//     // Build dynamic type if type identifier and object found in factory
//     if (type_identifier && type_object)
//     {
//         dynamic_type = fastrtps::types::TypeObjectFactory::get_instance()->build_dynamic_type(type_name_,
//                         type_identifier,
//                         type_object);
//     }

//     if (!dynamic_type)
//     {
//         // Create the callback to register the remote dynamic type
//         std::function<void(const std::string&, const fastrtps::types::DynamicType_ptr)> callback(
//             [this]
//                 (const std::string& name, const fastrtps::types::DynamicType_ptr type)
//             {
//                 this->register_remote_type_callback_(name, type);
//             });

//         // Register the discovered type and create a DataReader on this topic
//         participant_->register_remote_type(
//             type_information,
//             type_name.to_string(),
//             callback);
//     }
//     else
//     {
//         register_remote_type_callback_(type_name_, dynamic_type);
//     }
// }


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
