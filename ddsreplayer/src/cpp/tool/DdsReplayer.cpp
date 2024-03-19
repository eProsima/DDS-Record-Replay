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
#include <cpp_utils/ros2_mangling.hpp>

#include <fastdds/rtps/common/CDRMessage_t.h>
#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastrtps/types/TypeObjectFactory.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastrtps/attributes/ParticipantAttributes.h>

#include <ddspipe_core/types/dynamic_types/types.hpp>

#if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v1/DynamicTypesCollection.hpp>
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v1/DynamicTypesCollectionPubSubTypes.hpp>
#else
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v2/DynamicTypesCollection.hpp>
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v2/DynamicTypesCollectionPubSubTypes.hpp>
#endif // if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13

#include <ddsrecorder_participants/constants.hpp>

#if FASTRTPS_VERSION_MINOR < 13
    #include <fastcdr/Cdr.h>
    #include <fastcdr/FastBuffer.h>
    #include <fastcdr/FastCdr.h>
#else
    #include <fastdds/rtps/common/CdrSerialization.hpp>
#endif // if FASTRTPS_VERSION_MINOR < 13

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
        yaml::ReplayerConfiguration& configuration,
        std::string& input_file)
    : dyn_participant_(nullptr)
    , dyn_publisher_(nullptr)
{
    // Create Discovery Database
    discovery_database_ = std::make_shared<DiscoveryDatabase>();

    // Create Payload Pool
    payload_pool_ = std::make_shared<FastPayloadPool>();

    // Create Thread Pool
    thread_pool_ = std::make_shared<SlotThreadPool>(configuration.n_threads);

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

        // Set app properties
        pqos.properties().properties().emplace_back(
            "fastdds.application.id",
            configuration.replayer_configuration->app_id,
            "true");
        pqos.properties().properties().emplace_back(
            "fastdds.application.metadata",
            configuration.replayer_configuration->app_metadata,
            "true");

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

    // Generate builtin-topics from the topics in the MCAP file
    configuration.ddspipe_configuration.builtin_topics = generate_builtin_topics_(configuration, input_file);

    // Create DDS Pipe
    pipe_ = std::make_unique<DdsPipe>(
        configuration.ddspipe_configuration,
        discovery_database_,
        payload_pool_,
        participants_database_,
        thread_pool_);
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

utils::ReturnCode DdsReplayer::reload_configuration(
        const yaml::ReplayerConfiguration& new_configuration)
{
    return pipe_->reload_configuration(new_configuration.ddspipe_configuration);
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
    std::set<utils::Heritable<DistributedTopic>> builtin_topics;

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

    // Fetch version metadata
    auto metadatas = mcap_reader.metadata();
    std::string recording_version;
    if (metadatas.count(VERSION_METADATA_NAME) != 0)
    {
        mcap::KeyValueMap version_metadata = metadatas[VERSION_METADATA_NAME].metadata;
        recording_version = version_metadata[VERSION_METADATA_RELEASE];
    }
    else
    {
        recording_version = "UNKNOWN";
    }

    if (recording_version != DDSRECORDER_PARTICIPANTS_VERSION_STRING)
    {
        logWarning(DDSREPLAYER_REPLAYER,
                "MCAP file generated with a different DDS Record & Replay version (" << recording_version << ", current is " << DDSRECORDER_PARTICIPANTS_VERSION_STRING <<
                "), incompatibilities might arise...");
    }

    // Fetch dynamic types attachment
    auto attachments = mcap_reader.attachments();
    mcap::Attachment dynamic_attachment = attachments[DYNAMIC_TYPES_ATTACHMENT_NAME];

    // Deserialize dynamic types collection using CDR
    DynamicTypesCollection dynamic_types;
    eprosima::fastdds::dds::TypeSupport type_support(new DynamicTypesCollectionPubSubType());
    eprosima::fastrtps::rtps::SerializedPayload_t serialized_payload =
            eprosima::fastrtps::rtps::SerializedPayload_t(dynamic_attachment.dataSize);
    serialized_payload.length = dynamic_attachment.dataSize;
    std::memcpy(
        serialized_payload.data,
        reinterpret_cast<const unsigned char*>(dynamic_attachment.data),
        dynamic_attachment.dataSize);
    type_support.deserialize(&serialized_payload, &dynamic_types);

    std::set<std::string> registered_types{};
    if (configuration.replay_types)
    {
        // Register in factory dynamic types from attachment
        for (auto& dynamic_type: dynamic_types.dynamic_types())
        {
            register_dynamic_type_(dynamic_type);
            registered_types.insert(dynamic_type.type_name());
        }
    }

    auto channels = mcap_reader.channels();
    auto schemas = mcap_reader.schemas();

    for (auto it = channels.begin(); it != channels.end(); it++)
    {
        std::string topic_name = it->second->metadata[ROS2_TYPES] == "true" ? utils::mangle_if_ros_topic(
            it->second->topic) : it->second->topic;
        std::string type_name = it->second->metadata[ROS2_TYPES] == "true" ? utils::mangle_if_ros_type(
            schemas[it->second->schemaId]->name) : schemas[it->second->schemaId]->name;                                                                                                             // TODO: assert exists beforehand

        auto channel_topic = utils::Heritable<DdsTopic>::make_heritable();
        channel_topic->m_topic_name = topic_name;
        channel_topic->type_name = type_name;

        // Apply the QoS stored in the MCAP file as if they were the discovered QoS.
        channel_topic->topic_qos.set_qos(
            deserialize_qos_(it->second->metadata[QOS_SERIALIZATION_QOS]),
            utils::FuzzyLevelValues::fuzzy_level_fuzzy);

        // Insert channel topic in builtin topics list
        builtin_topics.insert(channel_topic);

        if (configuration.replay_types && registered_types.count(type_name) != 0)
        {
            // Make a copy of the Topic to customize it according to the Participant's configured QoS.
            utils::Heritable<DistributedTopic> topic = channel_topic->copy();

            // Apply the Manual Topics for this participant.
            for (const auto& manual_topic : configuration.ddspipe_configuration.get_manual_topics(*channel_topic))
            {
                topic->topic_qos.set_qos(manual_topic.first->topic_qos, utils::FuzzyLevelValues::fuzzy_level_hard);
            }

            // Create Datawriter in this topic so dynamic type can be shared in EDP
            // TODO: Avoid creating the dynamic writer when the topic is not allowed.
            create_dynamic_writer_(topic);
        }
    }

    mcap_reader.close();

    return builtin_topics;
}

void DdsReplayer::register_dynamic_type_(
        const ddsrecorder::participants::DynamicType& dynamic_type)
{
    // Decode type identifer and object strings
    std::string typeid_str = utils::base64_decode(dynamic_type.type_information());
    std::string typeobj_str = utils::base64_decode(dynamic_type.type_object());

    // Deserialize type identifer and object strings
    fastrtps::types::TypeIdentifier type_identifier = deserialize_type_identifier_(typeid_str);
    fastrtps::types::TypeObject type_object = deserialize_type_object_(typeobj_str);

    // Register in factory
    fastrtps::types::TypeObjectFactory::get_instance()->add_type_object(
        dynamic_type.type_name(), &type_identifier, &type_object);
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
    const auto parameter_length = cdr_message->length;
    fastrtps::rtps::SerializedPayload_t payload(parameter_length);
    fastcdr::FastBuffer fastbuffer((char*)payload.data, parameter_length);

    // Read data
    fastrtps::rtps::CDRMessage::readData(cdr_message, payload.data, parameter_length);

    // Create CDR deserializer
    #if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    fastcdr::Cdr deser(fastbuffer, fastcdr::Cdr::DEFAULT_ENDIAN, fastcdr::Cdr::DDS_CDR);
    #else
    fastcdr::Cdr deser(fastbuffer, fastcdr::Cdr::DEFAULT_ENDIAN, fastcdr::CdrVersion::XCDRv1);
    #endif // if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
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
    const auto parameter_length = cdr_message->length;
    fastrtps::rtps::SerializedPayload_t payload(parameter_length);
    fastcdr::FastBuffer fastbuffer((char*)payload.data, parameter_length);

    // Read data
    fastrtps::rtps::CDRMessage::readData(cdr_message, payload.data, parameter_length);

    // Create CDR deserializer
    #if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    fastcdr::Cdr deser(fastbuffer, fastcdr::Cdr::DEFAULT_ENDIAN, fastcdr::Cdr::DDS_CDR);
    #else
    fastcdr::Cdr deser(fastbuffer, fastcdr::Cdr::DEFAULT_ENDIAN, fastcdr::CdrVersion::XCDRv1);
    #endif // if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
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
