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
 * @file DynTypesParticipant.hpp
 */

#pragma once

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>

#include <ddsrouter_core/participants/participant/configuration/SimpleParticipantConfiguration.hpp>
#include <ddsrouter_core/participants/participant/rtps/SimpleParticipant.hpp>
#include <ddsrouter_core/participants/reader/auxiliar/InternalReader.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/**
 * This is an abomination Participant that is a Simple RTPS Participant with a built-in DDS Participant.
 * The DDS Part that is only used to read type objects and type lookup services.
 */
class DynTypesParticipant : public ddsrouter::participants::rtps::SimpleParticipant, public eprosima::fastdds::dds::DomainParticipantListener
{
public:

    /**
     * @brief Construct a new Dummy Participant object
     *
     * It uses the \c BaseParticipant constructor.
     * Apart from BaseParticipant, it creates a new RTPSParticipant with default Attributes and domain given
     * by configuration.
     *
     * @throw \c InitializationException in case any internal error has ocurred while creating RTPSParticipant
     * @throw \c IConfigurationException in case configuration was incorrectly set
     */
    DynTypesParticipant(
            std::shared_ptr<ddsrouter::participants::SimpleParticipantConfiguration> participant_configuration,
            std::shared_ptr<ddsrouter::core::PayloadPool> payload_pool,
            std::shared_ptr<ddsrouter::core::DiscoveryDatabase> discovery_database);

    ~DynTypesParticipant();

    void on_type_discovery(
            eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastrtps::rtps::SampleIdentity& request_sample_id,
            const eprosima::fastrtps::string_255& topic,
            const eprosima::fastrtps::types::TypeIdentifier* identifier,
            const eprosima::fastrtps::types::TypeObject* object,
            eprosima::fastrtps::types::DynamicType_ptr dyn_type) override;

    virtual void on_type_information_received(
            eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastrtps::string_255 topic_name,
            const eprosima::fastrtps::string_255 type_name,
            const eprosima::fastrtps::types::TypeInformation& type_information) override;

protected:

    /**
     * @brief Create a writer object
     *
     * Depending on the Topic QoS creates a Basic or Specific Writer.
     */
    std::shared_ptr<ddsrouter::core::IWriter> create_writer_(
            ddsrouter::core::types::DdsTopic topic) override;

    /**
     * @brief Create a reader object
     *
     * Depending on the Topic QoS creates a Basic or Specific Reader.
     */
    std::shared_ptr<ddsrouter::core::IReader> create_reader_(
            ddsrouter::core::types::DdsTopic topic) override;

    void internal_notify_type_object_(const std::string& type_name);

    void initialize_internal_dds_participant_();

    eprosima::fastdds::dds::DomainParticipant* dds_participant_;

    //! Type Object Internal Reader
    std::shared_ptr<ddsrouter::participants::InternalReader> type_object_reader_;
};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
