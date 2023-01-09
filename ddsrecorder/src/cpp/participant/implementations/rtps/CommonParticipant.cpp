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
 * @file CommonParticipant.cpp
 */

#include <memory>

#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/types/DynamicTypePtr.h>
#include <fastrtps/types/DynamicType.h>

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/utils.hpp>
#include <cpp_utils/Log.hpp>

#include <ddsrecorder/types/dds/DomainId.hpp>
#include <ddsrecorder/types/topic/rpc/RPCTopic.hpp>

#include <reader/implementations/auxiliar/InternalReader.hpp>
#include <reader/implementations/rtps/SpecificQoSReader.hpp>
#include <reader/implementations/rtps/SimpleReader.hpp>
#include <writer/implementations/auxiliar/BlankWriter.hpp>
#include <writer/implementations/rtps/MultiWriter.hpp>
#include <writer/implementations/rtps/SimpleWriter.hpp>
#include <writer/implementations/rtps/QoSSpecificWriter.hpp>
#include <participant/implementations/auxiliar/BaseParticipant.hpp>
#include <participant/implementations/rtps/CommonParticipant.hpp>
#include <recorder/types.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {
namespace rtps {

CommonParticipant::CommonParticipant(
        std::shared_ptr<configuration::ParticipantConfiguration> participant_configuration,
        std::shared_ptr<PayloadPool> payload_pool,
        std::shared_ptr<DiscoveryDatabase> discovery_database,
        const types::DomainId& domain_id,
        const fastrtps::rtps::RTPSParticipantAttributes& participant_attributes)
    : BaseParticipant(participant_configuration, payload_pool, discovery_database)
    , domain_id_(domain_id)
    , participant_attributes_(participant_attributes)
    , type_object_reader_(std::make_shared<InternalReader>(
        this->id(),
        recorder::type_object_topic(),
        this->payload_pool_))
{
    // Do nothing
}

CommonParticipant::~CommonParticipant()
{
    if (rtps_participant_)
    {
        fastrtps::rtps::RTPSDomain::removeRTPSParticipant(rtps_participant_);
    }
}

void CommonParticipant::init()
{
    create_participant_(
        domain_id_,
        participant_attributes_);
}

void CommonParticipant::onParticipantDiscovery(
        fastrtps::rtps::RTPSParticipant* participant,
        fastrtps::rtps::ParticipantDiscoveryInfo&& info)
{
    if (info.info.m_guid.guidPrefix != participant->getGuid().guidPrefix)
    {
        if (info.status == fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
        {
            logInfo(DDSRECORDER_DISCOVERY,
                    "Found in Participant " << this->id_nts_() << " new Participant " << info.info.m_guid << ".");
        }
        else if (info.status == fastrtps::rtps::ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT)
        {
            logInfo(DDSRECORDER_DISCOVERY, "Participant " << info.info.m_guid << " changed QoS.");
        }
        else if (info.status == fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT)
        {
            logInfo(DDSRECORDER_DISCOVERY, "Participant " << info.info.m_guid << " removed.");
        }
        else
        {
            logInfo(DDSRECORDER_DISCOVERY, "Participant " << info.info.m_guid << " dropped.");
        }
    }
}

template<class DiscoveryInfoKind>
types::Endpoint CommonParticipant::create_common_endpoint_from_info_(
        DiscoveryInfoKind& info)
{
    // Parse GUID
    types::Guid info_guid;
    info_guid = info.info.guid();

    // Parse TopicQoS
    types::TopicQoS discovered_topic_qos;
    // Durability
    discovered_topic_qos.durability_qos = info.info.m_qos.m_durability.durabilityKind();
    // Reliability
    if (info.info.m_qos.m_reliability.kind == fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
    {
        discovered_topic_qos.reliability_qos = fastrtps::rtps::BEST_EFFORT;
    }
    else if (info.info.m_qos.m_reliability.kind == fastdds::dds::RELIABLE_RELIABILITY_QOS)
    {
        discovered_topic_qos.reliability_qos = fastrtps::rtps::RELIABLE;
    }
    else
    {
        utils::tsnh(
            utils::Formatter() <<
                "Invalid ReliabilityQoS value found while parsing DiscoveryInfo for Endpoint creation.");
    }
    // Set Topic with Partitions
    discovered_topic_qos.use_partitions = !info.info.m_qos.m_partition.empty();
    // Set Topic with ownership
    discovered_topic_qos.ownership_qos = info.info.m_qos.m_ownership.kind;

    // Parse Topic
    types::DdsTopic info_topic(std::string(info.info.topicName()), std::string(info.info.typeName()));
    info_topic.keyed = info.info.topicKind() == eprosima::fastrtps::rtps::TopicKind_t::WITH_KEY;
    // Set qos as set, but fuzzy
    info_topic.topic_qos = discovered_topic_qos;
    info_topic.topic_qos.set_level(utils::FuzzyLevelValues::fuzzy_level_fuzzy);

    types::Endpoint endpoint = types::Endpoint(
        types::EndpointKind::invalid,
        info_guid,
        info_topic,
        this->id_nts_());

    // Parse specific QoS of the entity
    types::SpecificEndpointQoS specific_qos;
    if (discovered_topic_qos.has_partitions())
    {
        specific_qos.partitions = info.info.m_qos.m_partition;
    }
    endpoint.specific_qos(specific_qos);

    // NOTE: ownership is only for Writer
    return endpoint;
}

template<>
types::Endpoint CommonParticipant::create_endpoint_from_info_<fastrtps::rtps::WriterDiscoveryInfo>(
        fastrtps::rtps::WriterDiscoveryInfo& info)
{
    // Create Endpoint from common info
    types::Endpoint endpoint = create_common_endpoint_from_info_(info);

    if (endpoint.topic_qos().has_ownership())
    {
        // Only for writers (TODO: this could be done much better if Endpoint is not a class but a struct)
        auto specific_qos = endpoint.specific_qos();
        specific_qos.ownership_strength = info.info.m_qos.m_ownershipStrength;
        endpoint.specific_qos(specific_qos);
    }

    // Set type
    endpoint.kind(types::EndpointKind::writer);

    return endpoint;
}

template<>
types::Endpoint CommonParticipant::create_endpoint_from_info_<fastrtps::rtps::ReaderDiscoveryInfo>(
        fastrtps::rtps::ReaderDiscoveryInfo& info)
{
    // Create Endpoint from common info
    types::Endpoint endpoint = create_common_endpoint_from_info_(info);

    // Set type
    endpoint.kind(types::EndpointKind::reader);

    return endpoint;
}

void CommonParticipant::onReaderDiscovery(
        fastrtps::rtps::RTPSParticipant* participant,
        fastrtps::rtps::ReaderDiscoveryInfo&& info)
{
    if (info.info.guid().guidPrefix != participant->getGuid().guidPrefix)
    {
        types::Endpoint info_reader = create_endpoint_from_info_<fastrtps::rtps::ReaderDiscoveryInfo>(info);

        if (info.status == fastrtps::rtps::ReaderDiscoveryInfo::DISCOVERED_READER)
        {
            logInfo(DDSRECORDER_DISCOVERY,
                    "Found in Participant " << this->id_nts_() << " new Reader " << info.info.guid() << ".");

            this->discovery_database_->add_endpoint(info_reader);
        }
        else if (info.status == fastrtps::rtps::ReaderDiscoveryInfo::CHANGED_QOS_READER)
        {
            logInfo(DDSRECORDER_DISCOVERY, "Reader " << info.info.guid() << " changed TopicQoS.");

            this->discovery_database_->update_endpoint(info_reader);
        }
        else if (info.status == fastrtps::rtps::ReaderDiscoveryInfo::REMOVED_READER)
        {
            logInfo(DDSRECORDER_DISCOVERY, "Reader " << info.info.guid() << " removed.");

            info_reader.active(false);
            this->discovery_database_->erase_endpoint(info_reader);
        }
        else
        {
            logInfo(DDSRECORDER_DISCOVERY, "Reader " << info.info.guid() << " dropped.");

            info_reader.active(false);
            this->discovery_database_->erase_endpoint(info_reader);
        }
    }
}

void CommonParticipant::onWriterDiscovery(
        fastrtps::rtps::RTPSParticipant* participant,
        fastrtps::rtps::WriterDiscoveryInfo&& info)
{
    if (info.info.guid().guidPrefix != participant->getGuid().guidPrefix)
    {
        types::Endpoint info_writer = create_endpoint_from_info_<fastrtps::rtps::WriterDiscoveryInfo>(info);

        if (info.status == fastrtps::rtps::WriterDiscoveryInfo::DISCOVERED_WRITER)
        {
            logInfo(DDSRECORDER_DISCOVERY,
                    "Found in Participant " << this->id_nts_() << " new Writer " << info.info.guid() << ".");

            this->discovery_database_->add_endpoint(info_writer);
        }
        else if (info.status == fastrtps::rtps::WriterDiscoveryInfo::CHANGED_QOS_WRITER)
        {
            logInfo(DDSRECORDER_DISCOVERY, "Writer " << info.info.guid() << " changed TopicQoS.");

            this->discovery_database_->update_endpoint(info_writer);
        }
        else if (info.status == fastrtps::rtps::WriterDiscoveryInfo::REMOVED_WRITER)
        {
            logInfo(DDSRECORDER_DISCOVERY, "Writer " << info.info.guid() << " removed.");

            info_writer.active(false);
            this->discovery_database_->erase_endpoint(info_writer);
        }
        else
        {
            logInfo(DDSRECORDER_DISCOVERY, "Writer " << info.info.guid() << " dropped.");

            info_writer.active(false);
            this->discovery_database_->erase_endpoint(info_writer);
        }
    }
}

void CommonParticipant::on_type_discovery(
        fastrtps::rtps::RTPSParticipant* /* participant */,
        const fastrtps::rtps::SampleIdentity& /* request_sample_id */,
        const fastrtps::string_255& /* topic */,
        const fastrtps::types::TypeIdentifier* /* identifier */,
        const fastrtps::types::TypeObject* object,
        fastrtps::types::DynamicType_ptr /* dyn_type */)
{
    // internal_notify_type_object_(dyn_type);
    internal_notify_actual_type_object_(object);
}

void CommonParticipant::create_participant_(
        const types::DomainId& domain,
        const fastrtps::rtps::RTPSParticipantAttributes& participant_attributes)
{
    logInfo(DDSRECORDER_RTPS_PARTICIPANT,
            "Creating Participant in domain " << domain);

    // Listener must be set in creation as no callbacks should be missed
    // It is safe to do so here as object is already created and callbacks do not require anything set in this method
    rtps_participant_ = fastrtps::rtps::RTPSDomain::createParticipant(
        domain,
        participant_attributes,
        this);

    if (!rtps_participant_)
    {
        throw utils::InitializationException(
                  utils::Formatter() << "Error creating RTPS Participant " << this->id());
    }

    logInfo(DDSRECORDER_RTPS_PARTICIPANT,
            "New Participant " << this->configuration_->kind <<
            " created with id " << this->id() <<
            " in domain " << domain << " with guid " << rtps_participant_->getGuid() <<
            (this->is_repeater() ? " (repeater)" : " (non repeater)"));
}

std::shared_ptr<IWriter> CommonParticipant::create_writer_(
        types::DdsTopic topic)
{
    if (recorder::is_type_object_topic(topic))
    {
        // RTPS Participants has no type object writers
        return std::make_shared<BlankWriter>();
    }
    else
    {
        if (topic.topic_qos.get_reference().has_partitions() || topic.topic_qos.get_reference().has_ownership())
        {
            // Notice that MultiWriter does not require an init call
            return std::make_shared<MultiWriter>(
                this->id(),
                topic,
                this->payload_pool_,
                rtps_participant_,
                this->configuration_->is_repeater);
        }
        else
        {
            auto writer = std::make_shared<SimpleWriter>(
                this->id(),
                topic,
                this->payload_pool_,
                rtps_participant_,
                this->configuration_->is_repeater);
            writer->init();

            return writer;
        }
    }
}

std::shared_ptr<IReader> CommonParticipant::create_reader_(
        types::DdsTopic topic)
{
    if (recorder::is_type_object_topic(topic))
    {
        // If type object reader requested, send the one already created
        return type_object_reader_;
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

void CommonParticipant::internal_notify_type_object_(fastrtps::types::DynamicType_ptr dyn_type)
{
    logInfo(DDSRECORDER_RTPS_PARTICIPANT,
        "Participant " << this->id_nts_() << " sending internally type object " << dyn_type->get_name()
    );
    logError(DDSRECORDER_RTPS_PARTICIPANT,
        "Participant " << this->id_nts_() << " sending internally type object " << dyn_type->get_name()
    ); // TODO(recorder) remove

    // Store it so it is not destroyed
    dyn_types_.push_back(dyn_type);

    type_object_reader_->simulate_data_reception(
        std::move(recorder::type_object_data_serialization(payload_pool_, dyn_type))
    );
}

void CommonParticipant::internal_notify_actual_type_object_(const fastrtps::types::TypeObject* type_obj)
{
    logInfo(DDSRECORDER_RTPS_PARTICIPANT,
        "Participant " << this->id_nts_() << " sending internally type object.");
    logError(DDSRECORDER_RTPS_PARTICIPANT,
        "Participant " << this->id_nts_() << " sending internally type object.");

    type_object_reader_->simulate_data_reception(
        std::move(recorder::actual_type_object_data_serialization(payload_pool_, type_obj))
    );
}

fastrtps::rtps::RTPSParticipantAttributes
CommonParticipant::get_participant_attributes_(
        const configuration::ParticipantConfiguration* participant_configuration)
{
    fastrtps::rtps::RTPSParticipantAttributes params;

    // Add Participant name
    params.setName(participant_configuration->id.id_name().c_str());

    return params;
}

} /* namespace rtps */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
