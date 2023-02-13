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
 * @file TypeLookupServicePublisher.h
 *
 */

#ifndef _EPROSIMA_DDSRECORDER_RESOURCES_DDS_TYPELOOKUPSERVICE_TYPELOOKUPSERVICEPUBLISHER_H_
#define _EPROSIMA_DDSRECORDER_RESOURCES_DDS_TYPELOOKUPSERVICE_TYPELOOKUPSERVICEPUBLISHER_H_

#include <atomic>
#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

/**
 * @brief Data types that can be used in this example
 *
 */
enum class DataTypeKind
{
    HELLO_WORLD,
    COMPLETE
};

// Name of the data types that can be used
constexpr const char* HELLO_WORLD_DATA_TYPE_NAME = "HelloWorld";
constexpr const char* COMPLETE_DATA_TYPE_NAME = "Complete";

/**
 * @brief Class used to group into a single working unit a Publisher with a DataWriter and its listener.
 *
 */
class TypeLookupServicePublisher : public eprosima::fastdds::dds::DomainParticipantListener
{
public:

    /**
     * @brief Construct a new Type Lookup Service Publisher object
     *
     * @param topic_name Name of the DDS Topic
     * @param domain DDS Domain of the DomainParticipant
     * @param data_type_kind Data type to be used
     */
    TypeLookupServicePublisher(
            const std::string& topic_name,
            const uint32_t domain,
            DataTypeKind data_type_kind);

    /**
     * @brief Destroy the Type Lookup Service Publisher object
     *
     */
    virtual ~TypeLookupServicePublisher();

    /**
     * @brief Run the publisher until "number" samples are published every "sleep" seconds
     *
     * @param number Number of samples to be published
     * @param sleep Period for publishing
     */
    void run(
            uint32_t number,
            uint32_t sleep);

    //! DataWriter callback to inform new matches/unmatches with other DataReaders
    void on_publication_matched(
            eprosima::fastdds::dds::DataWriter* writer,
            const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

    //! Return the current state of execution
    static bool is_stopped();

    //! Trigger the end of execution
    static void stop();

protected:

    //! Publish a sample with given index
    void publish(unsigned int msg_index);

    /**
     * Publish a "number" of samples every "sleep" period.
     * This function is called internally from another thread so the publisher can
     * continue publishing samples while the main thread (application) is waiting
     * for a Ctrl+C (SIGNINT) interruption to exit the application.
     */
    void publisher_thread(
            uint32_t number,
            uint32_t sleep);

    //! Generate the HelloWorld DynamicType
    eprosima::fastrtps::types::DynamicType_ptr generate_helloworld_type_() const;

    //! Generate the Complete DynamicType
    eprosima::fastrtps::types::DynamicType_ptr generate_complete_type_() const;

    /**
     * @brief Fill HelloWold message with actual data
     *
     * @param index Index of the sample to be published. This parameter is used to modify the content of the message
     *              depending on the number of the sample to be sent.
     * @return eprosima::fastrtps::types::DynamicData_ptr The generated data to be published
     */
    eprosima::fastrtps::types::DynamicData_ptr fill_helloworld_data_(
        const unsigned int& index);

    /**
     * @brief Fill Complete message with actual data
     *
     * @param index Index of the sample to be published. This parameter is used to modify the content of the message
     *              depending on the number of the sample to be sent.
     * @return eprosima::fastrtps::types::DynamicData_ptr The generated data to be published
     */
    eprosima::fastrtps::types::DynamicData_ptr fill_complete_data_(
        const unsigned int& index);

    // Fast DDS entities
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Publisher* publisher_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::DataWriter* datawriter_;

    //! Name of the DDS Topic
    std::string topic_name_;
    //! The user can choose between HelloWorld and Complete types so this defines the chosen type
    DataTypeKind data_type_kind_;
    //! Name of the DDS Topic type according to the DataTypeKind
    std::string data_type_name_;
    //! Actual DynamicType generated according to the DataTypeKind
    eprosima::fastrtps::types::DynamicType_ptr dynamic_type_;


    //! Indicates if the application is still running
    static std::atomic<bool> stop_;
};

#endif /* _EPROSIMA_DDSRECORDER_RESOURCES_DDS_TYPELOOKUPSERVICE_TYPELOOKUPSERVICEPUBLISHER_H_ */
