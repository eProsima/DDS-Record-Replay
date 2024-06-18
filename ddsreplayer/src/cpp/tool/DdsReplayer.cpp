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

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

#include <ddspipe_core/types/dynamic_types/types.hpp>

#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollection.hpp>
#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollectionPubSubTypes.h>

#include <ddsrecorder_participants/constants.hpp>

#include <fastdds/rtps/common/CdrSerialization.hpp>

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
                "MCAP file generated with a different DDS Record & Replay version (" << recording_version <<
                ", current is " << DDSRECORDER_PARTICIPANTS_VERSION_STRING << "), incompatibilities might arise...");
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

    if (configuration.replay_types)
    {
        // Register in factory dynamic types from attachment
        for (auto& dynamic_type : dynamic_types.dynamic_types())
        {
            register_dynamic_type_(dynamic_type);
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
        channel_topic->type_ids = registered_types_[type_name];

        // Apply the QoS stored in the MCAP file as if they were the discovered QoS.
        channel_topic->topic_qos.set_qos(
            deserialize_qos_(it->second->metadata[QOS_SERIALIZATION_QOS]),
            utils::FuzzyLevelValues::fuzzy_level_fuzzy);

        // Insert channel topic in builtin topics list
        builtin_topics.insert(channel_topic);

        if (configuration.replay_types && registered_types_.count(type_name) != 0)
        {
            // Make a copy of the Topic to customize it according to the Participant's configured QoS.
            utils::Heritable<DistributedTopic> topic = channel_topic->copy();

            // Apply the Manual Topics for this participant.
            for (const auto& manual_topic : configuration.ddspipe_configuration.get_manual_topics(*channel_topic))
            {
                topic->topic_qos.set_qos(manual_topic.first->topic_qos, utils::FuzzyLevelValues::fuzzy_level_hard);
            }
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
    fastdds::dds::xtypes::TypeIdentifier type_identifier = deserialize_type_identifier_(typeid_str);
    fastdds::dds::xtypes::TypeObject type_object = deserialize_type_object_(typeobj_str);

    // Create a TypeIdentifierPair to use in register_type_identifier
    fastdds::dds::xtypes::TypeIdentifierPair type_identifiers;
    type_identifiers.type_identifier1(type_identifier);

    // // Register in factory
    if (fastdds::dds::RETCODE_OK != fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().register_type_object(
                type_object, type_identifiers))
    {
        logWarning(DDSREPLAYER_REPLAYER,
                "Failed to register " << dynamic_type.type_name() << " DynamicType.");
    }
    else
    {
        registered_types_.insert({dynamic_type.type_name(), type_identifiers});
    }
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

fastdds::dds::xtypes::TypeIdentifier DdsReplayer::deserialize_type_identifier_(
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
    if (cdr_message != nullptr)
    {
        if (cdr_message->length >= cdr_message->pos + parameter_length)
        {
            if (parameter_length > 0)
            {
                if (payload.data != nullptr)
                {
                    memcpy(payload.data, &cdr_message->buffer[cdr_message->pos], parameter_length);
                    cdr_message->pos += parameter_length;
                }
            }
        }
    }

    // Create CDR deserializer
    fastcdr::Cdr deser(fastbuffer, fastcdr::Cdr::DEFAULT_ENDIAN, fastcdr::CdrVersion::XCDRv2);
    payload.encapsulation = deser.endianness() == fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    // Deserialize
    fastdds::dds::xtypes::TypeIdentifier type_identifier;
    fastcdr::deserialize(deser, type_identifier);

    // Delete CDR message
    // NOTE: set wraps attribute to avoid double free (buffer released by string on destruction)
    cdr_message->wraps = true;
    delete cdr_message;

    return type_identifier;
}

fastdds::dds::xtypes::TypeObject DdsReplayer::deserialize_type_object_(
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
    if (cdr_message != nullptr)
    {
        if (cdr_message->length >= cdr_message->pos + parameter_length)
        {
            if (parameter_length > 0)
            {
                if (payload.data != nullptr)
                {
                    memcpy(payload.data, &cdr_message->buffer[cdr_message->pos], parameter_length);
                    cdr_message->pos += parameter_length;
                }
            }
        }
    }

    // Create CDR deserializer
    fastcdr::Cdr deser(fastbuffer, fastcdr::Cdr::DEFAULT_ENDIAN, fastcdr::CdrVersion::XCDRv2);
    payload.encapsulation = deser.endianness() == fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    // Deserialize
    fastdds::dds::xtypes::TypeObject type_object;
    fastcdr::deserialize(deser, type_object);

    // Delete CDR message
    // NOTE: set wraps attribute to avoid double free (buffer released by string on destruction)
    cdr_message->wraps = true;
    delete cdr_message;

    return type_object;
}

} /* namespace replayer */
} /* namespace ddsrecorder */
} /* namespace eprosima */
