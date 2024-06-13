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

#include <memory>
#include <set>
#include <string>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastrtps/types/TypeObjectFactory.h>

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/Log.hpp>
#include <cpp_utils/memory/Heritable.hpp>
#include <cpp_utils/ReturnCode.hpp>
#include <cpp_utils/thread_pool/pool/SlotThreadPool.hpp>
#include <cpp_utils/types/Fuzzy.hpp>

#include <ddspipe_core/core/DdsPipe.hpp>
#include <ddspipe_core/dynamic/DiscoveryDatabase.hpp>
#include <ddspipe_core/dynamic/ParticipantsDatabase.hpp>
#include <ddspipe_core/efficiency/payload/FastPayloadPool.hpp>

#include <ddsrecorder_participants/common/serialize/Serializer.hpp>
#include <ddsrecorder_participants/constants.hpp>
#include <ddsrecorder_participants/replayer/McapReaderParticipant.hpp>
#include <ddsrecorder_participants/replayer/ReplayerParticipant.hpp>
#include <ddsrecorder_participants/replayer/SqlReaderParticipant.hpp>

#if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v1/DynamicTypesCollection.hpp>
    // #include <ddsrecorder_participants/common/types/dynamic_types_collection/v1/DynamicTypesCollectionPubSubTypes.hpp>
#else
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v2/DynamicTypesCollection.hpp>
    // #include <ddsrecorder_participants/common/types/dynamic_types_collection/v2/DynamicTypesCollectionPubSubTypes.hpp>
#endif // if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13

#include "DdsReplayer.hpp"

namespace eprosima {
namespace ddsrecorder {
namespace replayer {

DdsReplayer::DdsReplayer(
        yaml::ReplayerConfiguration& configuration,
        const std::string& input_file)
    : configuration_(configuration)
{
    // Create Payload Pool
    auto payload_pool = std::make_shared<ddspipe::core::FastPayloadPool>();

    // Create Reader Participant
    if (input_file.size() >= 3 && input_file.substr(input_file.size() - 3) == ".db")
    {
        reader_participant_ = std::make_shared<participants::SqlReaderParticipant>(
            configuration.mcap_reader_configuration,
            payload_pool,
            input_file);
    }
    else
    {
        reader_participant_ = std::make_shared<participants::McapReaderParticipant>(
            configuration.mcap_reader_configuration,
            payload_pool,
            input_file);
    }

    // Create Discovery Database
    auto discovery_database = std::make_shared<ddspipe::core::DiscoveryDatabase>();

    // Create Replayer Participant
    auto replayer_participant = std::make_shared<participants::ReplayerParticipant>(
        configuration.replayer_configuration,
        payload_pool,
        discovery_database);

    replayer_participant->init();

    // Create and populate Participants Database
    auto participants_database = std::make_shared<ddspipe::core::ParticipantsDatabase>();

    // Populate Participants Database
    participants_database->add_participant(
        reader_participant_->id(),
        reader_participant_
        );

    participants_database->add_participant(
        replayer_participant->id(),
        replayer_participant
        );

    // Create Thread Pool
    thread_pool_ = std::make_shared<utils::SlotThreadPool>(configuration.n_threads);

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
            throw utils::InitializationException(STR_ENTRY << "Failed to create dynamic types participant.");
        }

        // Create publisher
        dyn_publisher_ = dyn_participant_->create_publisher(fastdds::dds::PUBLISHER_QOS_DEFAULT);
        if (nullptr == dyn_publisher_)
        {
            throw utils::InitializationException(STR_ENTRY << "Failed to create dynamic types publisher.");
        }
    }

    // Process the input file's summary
    std::set<utils::Heritable<ddspipe::core::types::DdsTopic>> topics;
    participants::DynamicTypesCollection types;

    reader_participant_->process_summary(topics, types);

    // Store the topics as built-in topics
    for (const auto& topic : topics)
    {
        configuration.ddspipe_configuration.builtin_topics.insert(topic);
    }

    // Register the dynamic types
    const auto& registered_types = register_dynamic_types_(types);

    // Create writers to publish the dynamic types
    create_dynamic_types_writers_(topics, registered_types);

    // Create DDS Pipe
    pipe_ = std::make_unique<ddspipe::core::DdsPipe>(
        configuration.ddspipe_configuration,
        discovery_database,
        payload_pool,
        participants_database,
        thread_pool_);
}

DdsReplayer::~DdsReplayer()
{
    if (dyn_participant_ == nullptr)
    {
        // The participant was not created
        return;
    }

    if (dyn_publisher_ != nullptr)
    {
        // Delete the writers
        for (const auto& [_, writer] : dyn_writers_)
        {
            if (writer != nullptr)
            {
                dyn_publisher_->delete_datawriter(writer);
            }
        }

        // Delete the publisher
        dyn_participant_->delete_publisher(dyn_publisher_);
    }

    // Delete the topics
    for (const auto& [_, topic] : dyn_topics_)
    {
        if (topic != nullptr)
        {
            dyn_participant_->delete_topic(topic);
        }
    }

    // Delete the participant
    fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(dyn_participant_);
}

utils::ReturnCode DdsReplayer::reload_configuration(
        const yaml::ReplayerConfiguration& new_configuration)
{
    return pipe_->reload_configuration(new_configuration.ddspipe_configuration);
}

void DdsReplayer::process_file()
{
    reader_participant_->process_messages();

    // Wait until all tasks have been consumed
    thread_pool_->wait_all_consumed();

    // Even if all tasks are consumed, they may still be in the process of being executed. Disabling the thread pool
    // blocks this thread until all ThreadPool threads are joined (which occurs when consumed tasks are completed).
    thread_pool_->disable();
}

void DdsReplayer::stop()
{
    reader_participant_->stop();
    pipe_->disable();
}

std::set<std::string> DdsReplayer::register_dynamic_types_(
        const participants::DynamicTypesCollection& dynamic_types)
{
    std::set<std::string> registered_types{};

    for (const auto& dynamic_type : dynamic_types.dynamic_types())
    {
        // Deserialize type identifier
        const auto type_identifier_str = utils::base64_decode(dynamic_type.type_information());
        const auto type_identifier = participants::Serializer::deserialize<fastrtps::types::TypeIdentifier>(type_identifier_str);

        // Deserialize type object
        const auto type_object_str = utils::base64_decode(dynamic_type.type_object());
        const auto type_object = participants::Serializer::deserialize<fastrtps::types::TypeObject>(type_object_str);

        // Register in factory
        fastrtps::types::TypeObjectFactory::get_instance()->add_type_object(
                dynamic_type.type_name(), &type_identifier, &type_object);

        registered_types.insert(dynamic_type.type_name());
    }

    return registered_types;
}

void DdsReplayer::create_dynamic_types_writers_(
        const std::set<utils::Heritable<ddspipe::core::types::DdsTopic>>& topics,
        const std::set<std::string>& registered_types)
{
    for (const auto& topic : topics)
    {
        if (registered_types.find(topic->type_name) == registered_types.end())
        {
            continue;
        }

        // Make a copy of the Topic to customize it according to the Participant's configured QoS.
        utils::Heritable<ddspipe::core::types::DdsTopic> topic_copy = topic->copy();

        // Apply the Manual Topics for this participant.
        for (const auto& [manual_topic, _] : configuration_.ddspipe_configuration.get_manual_topics(*topic))
        {
            topic_copy->topic_qos.set_qos(manual_topic->topic_qos, utils::FuzzyLevelValues::fuzzy_level_hard);
        }

        // Create Datawriter in this topic so dynamic type can be shared in EDP
        // TODO: Avoid creating the dynamic writer when the topic is not allowed.
        create_dynamic_type_writer_(topic_copy);
    }
}

void DdsReplayer::create_dynamic_type_writer_(
        utils::Heritable<ddspipe::core::types::DdsTopic> topic)
{
    const auto type_object_factory = fastrtps::types::TypeObjectFactory::get_instance();
    const auto type_identifier = type_object_factory->get_type_identifier(topic->type_name, true);
    const auto type_object = type_object_factory->get_type_object(topic->type_name, true);

    const auto dyn_type = type_object_factory->build_dynamic_type(
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
    const auto dyn_topic = dyn_participant_->create_topic(
            topic->m_topic_name, topic->type_name, fastdds::dds::TOPIC_QOS_DEFAULT);

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
    auto wqos = fastdds::dds::DATAWRITER_QOS_DEFAULT;
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
    const auto dyn_writer = dyn_publisher_->create_datawriter(dyn_topic, wqos);
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

} /* namespace replayer */
} /* namespace ddsrecorder */
} /* namespace eprosima */
