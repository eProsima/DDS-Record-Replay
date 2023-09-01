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

#include <mcap/reader.hpp>
#include <yaml-cpp/yaml.h>

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/utils.hpp>

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>
#include <fastcdr/FastCdr.h>
#include <fastdds/rtps/common/CDRMessage_t.h>
#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastrtps/types/TypeObjectFactory.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastrtps/attributes/ParticipantAttributes.h>

#include <ddsrecorder_participants/constants.hpp>

#include "DdsReplayer.hpp"

namespace eprosima {
namespace ddsrecorder {
namespace replayer {

using namespace eprosima::ddspipe::core;
using namespace eprosima::ddspipe::core::types;
using namespace eprosima::ddspipe::participants;
using namespace eprosima::ddspipe::participants::rtps;
using namespace eprosima::ddsrecorder::participants;
using namespace eprosima::utils;

DdsReplayer::DdsReplayer(
        const yaml::ReplayerConfiguration& configuration,
        std::string& input_file)
    : dyn_participant_(nullptr)
    , dyn_publisher_(nullptr)
{
    // Create allowed topics list
    auto allowed_topics = std::make_shared<AllowedTopicList>(
        configuration.allowlist,
        configuration.blocklist);

    // Create Discovery Database
    discovery_database_ =
            std::make_shared<DiscoveryDatabase>();

    // Create Payload Pool
    payload_pool_ =
            std::make_shared<FastPayloadPool>();

    // Create Thread Pool
    thread_pool_ =
            std::make_shared<SlotThreadPool>(configuration.n_threads);

    // Create MCAP Reader Participant
    mcap_reader_participant_ = std::make_shared<McapReaderParticipant>(
        configuration.mcap_reader_configuration,
        payload_pool_,
        input_file);

    // Create Replayer Participant
    replayer_participant_ = std::make_shared<ReplayerParticipant>(
        configuration.replayer_configuration,
        payload_pool_,
        discovery_database_);
    replayer_participant_->init();

    // Create and populate Participants Database
    participants_database_ =
            std::make_shared<ParticipantsDatabase>();

    // Populate Participants Database
    participants_database_->add_participant(
        mcap_reader_participant_->id(),
        mcap_reader_participant_
        );
    participants_database_->add_participant(
        replayer_participant_->id(),
        replayer_participant_
        );

    if (configuration.replay_types)
    {
        // Create participant sending dynamic types
        fastdds::dds::DomainParticipantQos pqos;
        pqos.name("DdsReplayer_dynTypesPublisher");

        // Set as server in TypeLookup service
        pqos.wire_protocol().builtin.typelookup_config.use_client = false;
        pqos.wire_protocol().builtin.typelookup_config.use_server = true;

        // Participant creation via factory
        dyn_participant_ = fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
            configuration.replayer_configuration->domain, pqos);
        if (nullptr == dyn_participant_)
        {
            throw utils::InitializationException(
                      STR_ENTRY << "Failed to create dynamic types participant."
                      );
        }

        // Create publisher
        dyn_publisher_ = dyn_participant_->create_publisher(fastdds::dds::PUBLISHER_QOS_DEFAULT);
        if (nullptr == dyn_publisher_)
        {
            throw utils::InitializationException(
                      STR_ENTRY << "Failed to create dynamic types publisher."
                      );
        }
    }

    // Generate builtin-topics list by combining information from YAML and MCAP files
    auto builtin_topics = generate_builtin_topics_(configuration, input_file);

    // Create DDS Pipe
    pipe_ = std::make_unique<DdsPipe>(
        allowed_topics,
        discovery_database_,
        payload_pool_,
        participants_database_,
        thread_pool_,
        builtin_topics,
        true);
}

DdsReplayer::~DdsReplayer()
{
    if (dyn_participant_ != nullptr)
    {
        if (dyn_publisher_ != nullptr)
        {
            for (auto writer : dyn_writers_)
            {
                if (writer.second != nullptr)
                {
                    dyn_publisher_->delete_datawriter(writer.second);
                }
            }
            dyn_participant_->delete_publisher(dyn_publisher_);
        }
        for (auto topic : dyn_topics_)
        {
            if (topic.second != nullptr)
            {
                dyn_participant_->delete_topic(topic.second);
            }
        }
        fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(dyn_participant_);
    }
}

utils::ReturnCode DdsReplayer::reload_allowed_topics(
        const std::shared_ptr<AllowedTopicList>& allowed_topics)
{
    return pipe_->reload_allowed_topics(allowed_topics);
}

void DdsReplayer::process_mcap()
{
    mcap_reader_participant_->process_mcap();

    // Wait until all tasks have been consumed
    thread_pool_->wait_all_consumed();

    // Even if all tasks are consumed, they may still be in the process of being executed. Disabling the thread pool
    // blocks this thread until all ThreadPool threads are joined (which occurs when consumed tasks are completed).
    thread_pool_->disable();
}

void DdsReplayer::stop()
{
    mcap_reader_participant_->stop();
    pipe_->disable();
}

std::set<utils::Heritable<DistributedTopic>> DdsReplayer::generate_builtin_topics_(
        const yaml::ReplayerConfiguration& configuration,
        std::string& input_file)
{
    std::set<utils::Heritable<DistributedTopic>> builtin_topics = configuration.builtin_topics;

    mcap::McapReader mcap_reader;

    auto status = mcap_reader.open(input_file);
    if (status.code != mcap::StatusCode::Success)
    {
        throw utils::InitializationException(
                  STR_ENTRY << "Failed MCAP read."
                  );
    }

    // Scan and parse channels and schemas
    const auto onProblem = [](const mcap::Status& status)
            {
                logWarning(DDSREPLAYER_REPLAYER,
                        "An error occurred while reading summary: " << status.message << ".");
            };
    // Read mcap summary: ForceScan method required for parsing metadata and attachments
    status = mcap_reader.readSummary(mcap::ReadSummaryMethod::ForceScan, onProblem);
    if (status.code != mcap::StatusCode::Success)
    {
        throw utils::InitializationException(
                  STR_ENTRY << "Failed to read summary."
                  );
    }

    // auto metadatas = mcap_reader.metadata();
    // mcap::KeyValueMap dynamic_metadata = metadatas[METADATA_DYNAMIC_TYPES].metadata;

    // Fetch dynamic types attachment
    auto attachments = mcap_reader.attachments();
    mcap::Attachment dynamic_attachment = attachments[DYNAMIC_TYPES_ATTACHMENT_NAME];
    std::string dynamic_types_str(reinterpret_cast<char const*>(dynamic_attachment.data), dynamic_attachment.dataSize);
    dynamic_types_str = utils::base64_decode(dynamic_types_str);

    // Deserialize into dynamic types map
    mcap::KeyValueMap dynamic_types;

    std::string intertypes_delimiter(INTERTYPES_SERIALIZATION_DELIMITER);
    std::string namevalue_delimiter(TYPE_NAME_VALUE_SERIALIZATION_DELIMITER);
    size_t intertypes_del_pos = 0;
    size_t current_pos = 0;
    while (intertypes_del_pos != std::string::npos)
    {
        intertypes_del_pos = dynamic_types_str.find(intertypes_delimiter, current_pos);
        std::string dynamic_type_str = dynamic_types_str.substr(current_pos, intertypes_del_pos - current_pos);

        auto namevalue_del_pos = dynamic_type_str.find(namevalue_delimiter);
        std::string dynamic_type_name = dynamic_type_str.substr(0, namevalue_del_pos);
        std::string dynamic_type_value = dynamic_type_str.substr(namevalue_del_pos + namevalue_delimiter.length(), std::string::npos);

        dynamic_types[dynamic_type_name] = dynamic_type_value;

        current_pos = intertypes_del_pos + intertypes_delimiter.length();
    }


    std::set<std::string> registered_types{};
    if (configuration.replay_types)
    {
        // Register in factory dynamic types from metadata
        for (auto& dynamic_type: dynamic_types)
        {
            register_dynamic_type_(dynamic_type.first, dynamic_type.second);
        }
        registered_types = get_keys(dynamic_types);
    }

    auto channels = mcap_reader.channels();
    auto schemas = mcap_reader.schemas();
    for (auto it = channels.begin(); it != channels.end(); it++)
    {
        std::string topic_name = it->second->topic;
        std::string type_name = schemas[it->second->schemaId]->name;  // TODO: assert exists beforehand

        auto channel_topic = utils::Heritable<DdsTopic>::make_heritable();
        channel_topic->m_topic_name = topic_name;
        channel_topic->type_name = type_name;

        if (builtin_topics.count(channel_topic) == 1)
        {
            // Already present in builtin_topics list, using qos provided through configuration
            // NOTE: also covers situation where there are channels for same topic with and without (blank) schema
            continue;
        }

        // Use QoS stored in MCAP file (discovered when recording, or given to recorder's builtin topics list)
        channel_topic->topic_qos = deserialize_qos_(it->second->metadata[QOS_SERIALIZATION_QOS]);

        // Insert channel topic in builtin topics list
        builtin_topics.insert(channel_topic);

        if (configuration.replay_types && registered_types.count(type_name) != 0)
        {
            // Create Datawriter in this topic so dynamic type can be shared in EDP
            create_dynamic_writer_(channel_topic);
        }
    }
    mcap_reader.close();

    return builtin_topics;
}

void DdsReplayer::register_dynamic_type_(
        const std::string& type_name,
        const std::string& dynamic_type)
{
    std::string delimiter(TYPE_ID_OBJECT_SERIALIZATION_DELIMITER);
    auto del_pos = dynamic_type.find(delimiter);

    // Split string (concatenation of serialized type identifer and object)
    std::string typeid_str = dynamic_type.substr(0, del_pos);
    std::string typeobj_str = dynamic_type.substr(del_pos + delimiter.length(), std::string::npos);

    // Deserialize type identifer and object strings
    fastrtps::types::TypeIdentifier type_identifier = deserialize_type_identifier_(typeid_str);
    fastrtps::types::TypeObject type_object = deserialize_type_object_(typeobj_str);

    // Register in factory
    fastrtps::types::TypeObjectFactory::get_instance()->add_type_object(type_name, &type_identifier, &type_object);
}

void DdsReplayer::create_dynamic_writer_(
        utils::Heritable<DdsTopic> topic)
{
    auto type_identifier = fastrtps::types::TypeObjectFactory::get_instance()->get_type_identifier(topic->type_name,
                    true);
    auto type_object = fastrtps::types::TypeObjectFactory::get_instance()->get_type_object(topic->type_name, true);
    fastrtps::types::DynamicType_ptr dyn_type = fastrtps::types::TypeObjectFactory::get_instance()->build_dynamic_type(
        topic->type_name,
        type_identifier,
        type_object);

    if (nullptr == dyn_type)
    {
        logWarning(DDSREPLAYER_REPLAYER,
                "Failed to create " << topic->type_name << " DynamicType, aborting dynamic writer creation...");
        return;
    }

    fastdds::dds::TypeSupport type(new fastrtps::types::DynamicPubSubType(dyn_type));

    if (nullptr == type)
    {
        logWarning(DDSREPLAYER_REPLAYER,
                "Failed to create " << topic->type_name << " TypeSupport, aborting dynamic writer creation...");
        return;
    }

    // Only enable sharing dynamic types through TypeLookup Service
    type->auto_fill_type_information(true);
    type->auto_fill_type_object(false);

    // Register type
    if (ReturnCode_t::RETCODE_OK != dyn_participant_->register_type(type))
    {
        logWarning(DDSREPLAYER_REPLAYER,
                "Failed to register " << topic->type_name << " type, aborting dynamic writer creation...");
        return;
    }

    // Create DDS topic
    fastdds::dds::Topic* dyn_topic = dyn_participant_->create_topic(topic->m_topic_name, topic->type_name,
                    fastdds::dds::TOPIC_QOS_DEFAULT);
    if (nullptr == dyn_topic)
    {
        logWarning(DDSREPLAYER_REPLAYER,
                "Failed to create {" << topic->m_topic_name << ";" << topic->type_name <<
                "} DDS topic, aborting dynamic writer creation...");
        return;
    }
    // Store pointer to be freed on destruction
    dyn_topics_[topic] = dyn_topic;

    // Create DDS writer QoS
    fastdds::dds::DataWriterQos wqos = fastdds::dds::DATAWRITER_QOS_DEFAULT;
    wqos.durability().kind =
            ( topic->topic_qos.is_transient_local() ?
            fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS :
            fastdds::dds::DurabilityQosPolicyKind::VOLATILE_DURABILITY_QOS
            );
    wqos.reliability().kind =
            ( topic->topic_qos.is_reliable() ?
            fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS :
            fastdds::dds::ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS
            );
    wqos.ownership().kind =
            ( topic->topic_qos.has_ownership() ?
            fastdds::dds::OwnershipQosPolicyKind::EXCLUSIVE_OWNERSHIP_QOS :
            fastdds::dds::OwnershipQosPolicyKind::SHARED_OWNERSHIP_QOS
            );

    // Create DDS writer
    fastdds::dds::DataWriter* dyn_writer = dyn_publisher_->create_datawriter(dyn_topic, wqos);
    if (nullptr == dyn_writer)
    {
        logWarning(DDSREPLAYER_REPLAYER,
                "Failed to create {" << topic->m_topic_name << ";" << topic->type_name <<
                "} DDS writer, aborting dynamic writer creation...");
        return;
    }
    // Store pointer to be freed on destruction
    dyn_writers_[topic] = dyn_writer;
}

TopicQoS DdsReplayer::deserialize_qos_(
        const std::string& qos_str)
{
    // TODO: Reuse code from ddspipe_yaml

    TopicQoS qos{};

    YAML::Node qos_yaml = YAML::Load(qos_str);
    bool reliable = qos_yaml[QOS_SERIALIZATION_RELIABILITY].as<bool>();
    bool transient_local = qos_yaml[QOS_SERIALIZATION_DURABILITY].as<bool>();
    bool exclusive_ownership = qos_yaml[QOS_SERIALIZATION_OWNERSHIP].as<bool>();
    bool keyed = qos_yaml[QOS_SERIALIZATION_KEYED].as<bool>();

    // Parse reliability
    if (reliable)
    {
        qos.reliability_qos = ddspipe::core::types::ReliabilityKind::RELIABLE;
    }
    else
    {
        qos.reliability_qos = ddspipe::core::types::ReliabilityKind::BEST_EFFORT;
    }

    // Parse durability
    if (transient_local)
    {
        qos.durability_qos = ddspipe::core::types::DurabilityKind::TRANSIENT_LOCAL;
    }
    else
    {
        qos.durability_qos = ddspipe::core::types::DurabilityKind::VOLATILE;
    }

    // Parse ownership
    if (exclusive_ownership)
    {
        qos.ownership_qos = ddspipe::core::types::OwnershipQosPolicyKind::EXCLUSIVE_OWNERSHIP_QOS;
    }
    else
    {
        qos.ownership_qos = ddspipe::core::types::OwnershipQosPolicyKind::SHARED_OWNERSHIP_QOS;
    }

    // Parse keyed
    qos.keyed = keyed;

    return qos;
}

fastrtps::types::TypeIdentifier DdsReplayer::deserialize_type_identifier_(
        const std::string& typeid_str)
{
    // Create CDR message from string
    // NOTE: Use 0 length to avoid allocation
    fastrtps::rtps::CDRMessage_t* cdr_message = new fastrtps::rtps::CDRMessage_t(0);
    cdr_message->buffer = (unsigned char*)reinterpret_cast<const unsigned char*>(typeid_str.c_str());
    cdr_message->length = typeid_str.length();
#if __BIG_ENDIAN__
    cdr_message->msg_endian = fastrtps::rtps::BIGEND;
#else
    cdr_message->msg_endian = fastrtps::rtps::LITTLEEND;
#endif // if __BIG_ENDIAN__

    // Reserve payload and create buffer
    uint16_t parameter_length = cdr_message->length;
    fastrtps::rtps::SerializedPayload_t payload(parameter_length);
    fastcdr::FastBuffer fastbuffer((char*)payload.data, parameter_length);

    // Read data
    fastrtps::rtps::CDRMessage::readData(cdr_message, payload.data, parameter_length);

    // Create CDR deserializer
    fastcdr::Cdr deser(fastbuffer, fastcdr::Cdr::DEFAULT_ENDIAN, fastcdr::Cdr::DDS_CDR);
    payload.encapsulation = deser.endianness() == fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    // Deserialize
    fastrtps::types::TypeIdentifier type_identifier;
    type_identifier.deserialize(deser);

    // Delete CDR message
    // NOTE: set wraps attribute to avoid double free (buffer released by string on destruction)
    cdr_message->wraps = true;
    delete cdr_message;

    return type_identifier;
}

fastrtps::types::TypeObject DdsReplayer::deserialize_type_object_(
        const std::string& typeobj_str)
{
    // Create CDR message from string
    // NOTE: Use 0 length to avoid allocation
    fastrtps::rtps::CDRMessage_t* cdr_message = new fastrtps::rtps::CDRMessage_t(0);
    cdr_message->buffer = (unsigned char*)reinterpret_cast<const unsigned char*>(typeobj_str.c_str());
    cdr_message->length = typeobj_str.length();
#if __BIG_ENDIAN__
    cdr_message->msg_endian = fastrtps::rtps::BIGEND;
#else
    cdr_message->msg_endian = fastrtps::rtps::LITTLEEND;
#endif // if __BIG_ENDIAN__

    // Reserve payload and create buffer
    uint16_t parameter_length = cdr_message->length;
    fastrtps::rtps::SerializedPayload_t payload(parameter_length);
    fastcdr::FastBuffer fastbuffer((char*)payload.data, parameter_length);

    // Read data
    fastrtps::rtps::CDRMessage::readData(cdr_message, payload.data, parameter_length);

    // Create CDR deserializer
    fastcdr::Cdr deser(fastbuffer, fastcdr::Cdr::DEFAULT_ENDIAN, fastcdr::Cdr::DDS_CDR);
    payload.encapsulation = deser.endianness() == fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    // Deserialize
    fastrtps::types::TypeObject type_object;
    type_object.deserialize(deser);

    // Delete CDR message
    // NOTE: set wraps attribute to avoid double free (buffer released by string on destruction)
    cdr_message->wraps = true;
    delete cdr_message;

    return type_object;
}

} /* namespace replayer */
} /* namespace ddsrecorder */
} /* namespace eprosima */
