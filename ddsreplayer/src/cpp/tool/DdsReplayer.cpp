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

#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>

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

#include <cpp_utils/exception/InconsistencyException.hpp>

#include <ddspipe_core/types/dynamic_types/types.hpp>

#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollection.hpp>
#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollectionPubSubTypes.hpp>

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
        discovery_database_,
        configuration.replay_types);
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
                EPROSIMA_LOG_WARNING(DDSREPLAYER_REPLAYER,
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
        EPROSIMA_LOG_WARNING(DDSREPLAYER_REPLAYER,
                "MCAP file generated with a different DDS Record & Replay version (" << recording_version <<
                ", current is " << DDSRECORDER_PARTICIPANTS_VERSION_STRING << "), incompatibilities might arise...");
    }

    // Fetch dynamic types attachment
    auto attachments = mcap_reader.attachments();
    mcap::Attachment dynamic_attachment = attachments[DYNAMIC_TYPES_ATTACHMENT_NAME];

    // Deserialize dynamic types collection using CDR
    DynamicTypesCollection dynamic_types;
    eprosima::fastdds::dds::TypeSupport type_support(new DynamicTypesCollectionPubSubType());
    eprosima::fastdds::rtps::SerializedPayload_t serialized_payload =
            eprosima::fastdds::rtps::SerializedPayload_t(dynamic_attachment.dataSize);
    serialized_payload.length = dynamic_attachment.dataSize;
    std::memcpy(
        serialized_payload.data,
        reinterpret_cast<const unsigned char*>(dynamic_attachment.data),
        dynamic_attachment.dataSize);
    type_support.deserialize(serialized_payload, &dynamic_types);

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
        channel_topic->type_identifiers = registered_types_[type_name];

        // Apply the QoS stored in the MCAP file as if they were the discovered QoS.
        channel_topic->topic_qos.set_qos(
            deserialize_qos_(it->second->metadata[QOS_SERIALIZATION_QOS]),
            utils::FuzzyLevelValues::fuzzy_level_fuzzy);

        // Insert channel topic in builtin topics list
        builtin_topics.insert(channel_topic);
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
    fastdds::dds::xtypes::TypeIdentifier type_identifier;
    fastdds::dds::xtypes::TypeObject type_object;

    try
    {
        // Deserialize type identifer and object strings
        type_identifier = deserialize_type_identifier_(typeid_str);
        type_object = deserialize_type_object_(typeobj_str);
    }
    catch (const utils::InconsistencyException& e)
    {
        EPROSIMA_LOG_WARNING(DDSREPLAYER_REPLAYER,
                "Failed to deserialize " << dynamic_type.type_name() << " DynamicType: " << e.what());
        return;
    }


    // Create a TypeIdentifierPair to use in register_type_identifier
    fastdds::dds::xtypes::TypeIdentifierPair type_identifiers;
    type_identifiers.type_identifier1(type_identifier);

    // Register in factory
    if (fastdds::dds::RETCODE_OK !=
            fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().register_type_object(
                type_object, type_identifiers))
    {
        EPROSIMA_LOG_WARNING(DDSREPLAYER_REPLAYER,
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

template<class DynamicTypeData>
DynamicTypeData DdsReplayer::deserialize_type_data_(
        const std::string& typedata_str)
{
    // Create CDR message from string
    // NOTE: Use 0 length to avoid allocation
    fastdds::rtps::CDRMessage_t* cdr_message = new fastdds::rtps::CDRMessage_t(0);
    cdr_message->buffer = (unsigned char*)reinterpret_cast<const unsigned char*>(typedata_str.c_str());
    cdr_message->length = typedata_str.length();
#if __BIG_ENDIAN__
    cdr_message->msg_endian = fastdds::rtps::BIGEND;
#else
    cdr_message->msg_endian = fastdds::rtps::LITTLEEND;
#endif // if __BIG_ENDIAN__

    // Reserve payload and create buffer
    const auto parameter_length = cdr_message->length;
    fastdds::rtps::SerializedPayload_t payload(parameter_length);
    fastcdr::FastBuffer fastbuffer((char*)payload.data, parameter_length);

    // Check cdr message is valid
    if (!cdr_message)
    {
        throw utils::InconsistencyException(
                  "Error reading data -> cdr_message is null.");
    }

    // Check enough space in buffer
    if (!(cdr_message->length >= cdr_message->pos + parameter_length))
    {
        throw utils::InconsistencyException(
                  "Error reading data -> not enough space in cdr_message buffer.");
    }

    // Check length is consistent
    if (!(parameter_length > 0))
    {
        throw utils::InconsistencyException(
                  "Error reading data -> payload length is greater than 0.");
    }

    // Check payload is valid
    if (!payload.data)
    {
        throw utils::InconsistencyException(
                  "Error reading data -> payload data is null.");
    }

    // Copy data
    memcpy(payload.data, &cdr_message->buffer[cdr_message->pos], parameter_length);
    cdr_message->pos += parameter_length;

    // Create CDR deserializer
    fastcdr::Cdr deser(fastbuffer, fastcdr::Cdr::DEFAULT_ENDIAN, fastcdr::CdrVersion::XCDRv2);
    payload.encapsulation = deser.endianness() == fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    // Deserialize
    DynamicTypeData type_data;
    fastcdr::deserialize(deser, type_data);

    // Delete CDR message
    // NOTE: set wraps attribute to avoid double free (buffer released by string on destruction)
    cdr_message->wraps = true;
    delete cdr_message;

    return type_data;
}

fastdds::dds::xtypes::TypeIdentifier DdsReplayer::deserialize_type_identifier_(
        const std::string& typeid_str)
{
    fastdds::dds::xtypes::TypeIdentifier type_id;
    try
    {
        type_id = deserialize_type_data_<fastdds::dds::xtypes::TypeIdentifier>(typeid_str);
    }
    catch (const utils::InconsistencyException& e)
    {
        throw utils::InconsistencyException(std::string("Failed to deserialize TypeIdentifier: ") + e.what());
    }

    return type_id;
}

fastdds::dds::xtypes::TypeObject DdsReplayer::deserialize_type_object_(
        const std::string& typeobj_str)
{
    fastdds::dds::xtypes::TypeObject type_obj;
    try
    {
        type_obj = deserialize_type_data_<fastdds::dds::xtypes::TypeObject>(typeobj_str);
    }
    catch (const utils::InconsistencyException& e)
    {
        throw utils::InconsistencyException(std::string("Failed to deserialize TypeObject: ") + e.what());
    }

    return type_obj;
}

} /* namespace replayer */
} /* namespace ddsrecorder */
} /* namespace eprosima */
