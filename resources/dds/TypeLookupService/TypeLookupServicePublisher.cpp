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
 * @file TypeLookupServicePublisher.cpp
 *
 */

#include <csignal>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicDataHelper.hpp>
#include <fastrtps/types/DynamicDataPtr.h>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/TypeObjectFactory.h>
#include <fastrtps/types/TypesBase.h>

#include "TypeLookupServicePublisher.h"
#include "types/complete/CompleteTypeObject.h"
#include "types/hello_world/HelloWorldTypeObject.h"

using namespace eprosima::fastdds::dds;

std::atomic<bool> TypeLookupServicePublisher::stop_(false);

TypeLookupServicePublisher::TypeLookupServicePublisher(
        const std::string& topic_name,
        const uint32_t domain,
        DataTypeKind data_type_kind)
    : participant_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , datawriter_(nullptr)
    , topic_name_(topic_name)
    , data_type_kind_(data_type_kind)
{
    ///////////////////////////////
    // Create the DomainParticipant
    DomainParticipantQos pqos;
    pqos.name("TypeLookupService_Participant_Publisher");

    pqos.wire_protocol().builtin.typelookup_config.use_client = false;
    pqos.wire_protocol().builtin.typelookup_config.use_server = true;

    participant_ = DomainParticipantFactory::get_instance()->create_participant(domain, pqos);

    if (participant_ == nullptr)
    {
        throw std::runtime_error("Error creating participant");
    }

    ////////////////////
    // Register the type
    switch (data_type_kind_)
    {
        case DataTypeKind::HELLO_WORLD:
            dynamic_type_ = generate_helloworld_type_();
            data_type_name_ = HELLO_WORLD_DATA_TYPE_NAME;
            break;
        case DataTypeKind::COMPLETE:
            dynamic_type_ = generate_complete_type_();
            data_type_name_ = COMPLETE_DATA_TYPE_NAME;
            break;
        default:
            throw std::runtime_error("Not recognized DynamicType kind");
            break;
    }

    TypeSupport type(new eprosima::fastrtps::types::DynamicPubSubType(dynamic_type_));

    // Send type information so the type can be discovered
    type->auto_fill_type_information(true);
    type->auto_fill_type_object(false);

    // Register the type in the Participant
    participant_->register_type(type);

    ///////////////////////
    // Create the Publisher
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, this);

    if (publisher_ == nullptr)
    {
        throw std::runtime_error("Error creating publisher");
    }

    ///////////////////////
    // Create the DDS Topic
    topic_ = participant_->create_topic(topic_name_, data_type_name_, TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        throw std::runtime_error("Error creating topic");
    }

    ////////////////////////////
    // Create the DDS DataWriter
    datawriter_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, this);

    if (datawriter_ == nullptr)
    {
        throw std::runtime_error("Error creating datawriter");
    }

    std::cout <<
        "Participant < " << participant_->guid() << "> created...\n" <<
        "\t- DDS Domain: " << participant_->get_domain_id() << "\n" <<
        "\t- DataWriter: " << datawriter_->guid() << "\n" <<
        "\t- Topic name: " << topic_name_ << "\n" <<
        "\t- Topic data type: " << data_type_name_ <<
        std::endl;
}

TypeLookupServicePublisher::~TypeLookupServicePublisher()
{
    if (participant_ != nullptr)
    {
        if (publisher_ != nullptr)
        {
            if (datawriter_ != nullptr)
            {
                publisher_->delete_datawriter(datawriter_);
            }
            participant_->delete_publisher(publisher_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

bool TypeLookupServicePublisher::is_stopped()
{
    return stop_;
}

void TypeLookupServicePublisher::stop()
{
    stop_ = true;
}

void TypeLookupServicePublisher::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "DataWriter matched with DataReader: " << info.last_subscription_handle << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "DataWriter unmatched with DataReader: " << info.last_subscription_handle << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void TypeLookupServicePublisher::publisher_thread(
        uint32_t samples,
        uint32_t sleep)
{
    // Publish samples until the sample limit is reached or the user stops the application (Ctrl+C)
    unsigned int samples_sent = 0;
    while (!is_stopped() && (samples == 0 || samples_sent < samples))
    {
        publish(samples_sent);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        ++samples_sent;
    }
}

void TypeLookupServicePublisher::run(
        uint32_t samples,
        uint32_t sleep)
{
    stop_ = false;

    // Run the publisher on another thread so the example can be stopped with Ctrl+C signal
    std::thread publish_thread(&TypeLookupServicePublisher::publisher_thread, this, samples, sleep);

    if (samples == 0)
    {
        std::cout << "Publisher running. Please press CTRL+C to stop the Publisher at any time." << std::endl;
    }
    else
    {
        std::cout << "Publisher running for " << samples <<
            " samples. Press CTRL+C to stop the Publisher at any time..." << std::endl;
    }



    // Ctrl+C (SIGINT) termination signal handler
    signal(SIGINT, [](int signum)
            {
                std::cout << "\nSIGINT received, stopping Publisher execution." << std::endl;
                static_cast<void>(signum);
                TypeLookupServicePublisher::stop();
            });

    // Join the publisher running thread
    publish_thread.join();
}

void TypeLookupServicePublisher::publish(unsigned int msg_index)
{
    // Get the dynamic data depending on the data type
    eprosima::fastrtps::types::DynamicData_ptr dynamic_data_;
    switch (data_type_kind_)
    {
    case DataTypeKind::HELLO_WORLD:
        dynamic_data_ = fill_helloworld_data_(msg_index);
        break;
    case DataTypeKind::COMPLETE:
        dynamic_data_ = fill_complete_data_(msg_index);
        break;

    default:
        throw std::runtime_error("Not recognized DynamicType kind");
        break;
    }

    // Publish data
    datawriter_->write(dynamic_data_.get());

    // Print the message published
    std::cout << "Message published: " << std::endl;
    eprosima::fastrtps::types::DynamicDataHelper::print(dynamic_data_);
    std::cout << "-----------------------------------------------------" << std::endl;
}

eprosima::fastrtps::types::DynamicType_ptr
    TypeLookupServicePublisher::generate_helloworld_type_() const
{
    // Generate HelloWorld type using methods from Fast DDS Gen autogenerated code
    registerHelloWorldTypes();

    // Get the complete type object and type identifier of the dynamic type
    auto type_object = GetHelloWorldObject(true);
    auto type_id = GetHelloWorldIdentifier(true);

    // Use data type name, type identifier and type object to build the dynamic type
    return eprosima::fastrtps::types::TypeObjectFactory::get_instance()->build_dynamic_type(
            HELLO_WORLD_DATA_TYPE_NAME,
            type_id,
            type_object);
}

eprosima::fastrtps::types::DynamicType_ptr
    TypeLookupServicePublisher::generate_complete_type_() const
{
    // Generate Complete type using methods from Fast DDS Gen autogenerated code
    registerCompleteTypes();

    // For complex types with recursive structures and dependent of another types,
    // we need to build the higher level type, as the rest of dependent types will be built recursively

    // Get the complete type object and type identifier of the dynamic type
    auto type_object = GetCompleteDataObject(true);
    auto type_id = GetCompleteDataIdentifier(true);

    // Use data type name, type identifier and type object to build the dynamic type
    return eprosima::fastrtps::types::TypeObjectFactory::get_instance()->build_dynamic_type(
            COMPLETE_DATA_TYPE_NAME,
            type_id,
            type_object);
}

eprosima::fastrtps::types::DynamicData_ptr
    TypeLookupServicePublisher::fill_helloworld_data_(
        const unsigned int& index)
{
    // Create and initialize new dynamic data
    eprosima::fastrtps::types::DynamicData_ptr new_data;
    new_data = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(dynamic_type_);

    // Set index
    new_data->set_uint32_value(index, 0);
    // Set message
    new_data->set_string_value("Hello World", 1);

    return new_data;
}

eprosima::fastrtps::types::DynamicData_ptr
    TypeLookupServicePublisher::fill_complete_data_(
        const unsigned int& index)
{
    // Create and initialize new dynamic data
    eprosima::fastrtps::types::DynamicData_ptr new_data;
    new_data = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(dynamic_type_);

    // Set index
    new_data->set_uint32_value(index, 0);

    // Set main_point
    eprosima::fastrtps::types::DynamicData* main_point = new_data->loan_value(1);
    main_point->set_int32_value(50, 0);
    main_point->set_int32_value(100, 1);
    main_point->set_int32_value(200, 2);
    new_data->return_loaned_value(main_point);

    // Set internal_data
    eprosima::fastrtps::types::DynamicData* points_sequence = new_data->loan_value(2);
    eprosima::fastrtps::types::DynamicType_ptr seq_elem_type =
            points_sequence->get_type()->get_descriptor()->get_element_type();
    eprosima::fastrtps::types::DynamicData_ptr seq_elem;
    eprosima::fastrtps::types::MemberId id;

    // internal_data sequence element 1
    seq_elem = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(seq_elem_type);
    seq_elem->set_int32_value(0, 0);
    seq_elem->set_int32_value(1, 1);
    seq_elem->set_int32_value(2, 2);
    points_sequence->insert_complex_value(seq_elem, id);

    // internal_data sequence element 2
    seq_elem = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(seq_elem_type);
    seq_elem->set_int32_value(3, 0);
    seq_elem->set_int32_value(4, 1);
    seq_elem->set_int32_value(5, 2);
    points_sequence->insert_complex_value(seq_elem, id);

    // internal_data sequence element 3
    seq_elem = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(seq_elem_type);
    seq_elem->set_int32_value(6, 0);
    seq_elem->set_int32_value(7, 1);
    seq_elem->set_int32_value(8, 2);
    points_sequence->insert_complex_value(seq_elem, id);

    new_data->return_loaned_value(points_sequence);

    // Set messages
    eprosima::fastrtps::types::DynamicData* messages_array = new_data->loan_value(3);
    eprosima::fastrtps::types::DynamicData* array_elem;
    eprosima::fastrtps::types::DynamicData* sub_elem;
    eprosima::fastrtps::types::DynamicData* timestamp;
    for (int i=0; i<2; i++)
    {
        array_elem = messages_array->loan_value(i);

        // descriptor
        sub_elem = array_elem->loan_value(0);
        // descriptor id
        sub_elem->set_uint32_value(i, 0);
        // descriptor topic
        sub_elem->set_string_value("Valuable information", 1);
        // descriptor timestamp
        timestamp = sub_elem->loan_value(2);
        timestamp->set_int32_value(index, 0);
        timestamp->set_int32_value(1000 * index, 1);
        sub_elem->return_loaned_value(timestamp);
        array_elem->return_loaned_value(sub_elem);

        // message
        array_elem->set_string_value("message #" + std::to_string(i), 1);
        messages_array->return_loaned_value(array_elem);
    }

    new_data->return_loaned_value(messages_array);

    return new_data;
}
