// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DynTypesParticipant.cpp
 */

#include <memory>

#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/types/DynamicTypePtr.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/TypeObjectFactory.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include <participant/implementations/rtps/DynTypesParticipant.hpp>
#include <writer/implementations/auxiliar/BlankWriter.hpp>
#include <reader/implementations/auxiliar/BaseReader.hpp>
#include <reader/implementations/auxiliar/BlankReader.hpp>
#include <reader/implementations/rtps/SpecificQoSReader.hpp>
#include <reader/implementations/rtps/SimpleReader.hpp>
#include <recorder/dynamic_types/types.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {
namespace rtps {

using namespace eprosima::ddsrecorder::core::types;

DynTypesParticipant::DynTypesParticipant(
        std::shared_ptr<configuration::SimpleParticipantConfiguration> participant_configuration,
        std::shared_ptr<PayloadPool> payload_pool,
        std::shared_ptr<DiscoveryDatabase> discovery_database)
    : SimpleParticipant(
        participant_configuration,
        payload_pool,
        discovery_database)
    , type_object_reader_(std::make_shared<InternalReader>(
        this->id(),
        recorder::type_object_topic(),
        this->payload_pool_))
{
    initialize_internal_dds_participant_();
}

DynTypesParticipant::~DynTypesParticipant()
{
    dds_participant_->set_listener(nullptr);
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(dds_participant_);

}

void DynTypesParticipant::on_type_discovery(
        eprosima::fastdds::dds::DomainParticipant* /* participant */,
        const fastrtps::rtps::SampleIdentity& /* request_sample_id */,
        const fastrtps::string_255& /* topic */,
        const fastrtps::types::TypeIdentifier* identifier,
        const fastrtps::types::TypeObject* object,
        fastrtps::types::DynamicType_ptr dyn_type)
{
    if (nullptr != dyn_type)
    {
        // Register type obj in singleton factory
        eprosima::fastrtps::types::TypeObjectFactory::get_instance()->add_type_object(dyn_type->get_name(), identifier, object);
        internal_notify_type_object_(dyn_type->get_name());
    }
}

void DynTypesParticipant::on_type_information_received(
        eprosima::fastdds::dds::DomainParticipant* participant,
        const fastrtps::string_255 /* topic_name */,
        const fastrtps::string_255 type_name,
        const fastrtps::types::TypeInformation& type_information)
{
    std::function<void(const std::string&, const eprosima::fastrtps::types::DynamicType_ptr)> callback(
        [this]
        (const std::string& type_name, const eprosima::fastrtps::types::DynamicType_ptr /* type */)
        {
            this->internal_notify_type_object_(type_name);
        });

    // Registering type and creating reader
    participant->register_remote_type(
        type_information,
        type_name.to_string(),
        callback);
}

std::shared_ptr<IWriter> DynTypesParticipant::create_writer_(
        types::DdsTopic topic)
{
    // NOTE: The Participant in the Recorder does not require writers
    return std::make_shared<BlankWriter>();
}

std::shared_ptr<IReader> DynTypesParticipant::create_reader_(
        types::DdsTopic topic)
{
    if (recorder::is_type_object_topic(topic))
    {
        return this->type_object_reader_;
    }
    else
    {
        if (topic.topic_qos.get_reference().has_partitions() || topic.topic_qos.get_reference().has_ownership())
        {
            auto reader = std::make_shared<SpecificQoSReader>(
                this->id(),
                topic,
                this->payload_pool_,
                rtps_participant_,
                discovery_database_);
            reader->init();

            return reader;
        }
        else
        {
            auto reader = std::make_shared<SimpleReader>(
                this->id(),
                topic,
                this->payload_pool_,
                rtps_participant_);
            reader->init();

            return reader;
        }
    }
}

void DynTypesParticipant::internal_notify_type_object_(const std::string& type_name)
{
    logInfo(DDSRECORDER_RTPS_PARTICIPANT,
        "Participant " << this->id_nts_() << " discovered type object " << type_name);

    type_object_reader_->simulate_data_reception(
        std::move(recorder::string_serialization(payload_pool_, type_name))
    );
}

void DynTypesParticipant::initialize_internal_dds_participant_()
{
    eprosima::fastdds::dds::DomainParticipantQos pqos;
    pqos.name(this->configuration_->id.id_name());

    // Set Type LookUp to ON
    pqos.wire_protocol().builtin.typelookup_config.use_server = false;
    pqos.wire_protocol().builtin.typelookup_config.use_client = true;

    // CREATE THE PARTICIPANT
    dds_participant_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        std::dynamic_pointer_cast<configuration::SimpleParticipantConfiguration>(this->configuration_)->domain,
        pqos,
        this);

    if (dds_participant_ == nullptr)
    {
        throw utils::InitializationException("Error creating DDS Participant.");
    }
}

} /* namespace rtps */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
