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
#include <regex>
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

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/Log.hpp>
#include <cpp_utils/memory/Heritable.hpp>
#include <cpp_utils/ReturnCode.hpp>
#include <cpp_utils/thread_pool/pool/SlotThreadPool.hpp>
#include <cpp_utils/types/Fuzzy.hpp>

#include <ddspipe_core/core/DdsPipe.hpp>
#include <ddspipe_core/dynamic/DiscoveryDatabase.hpp>
#include <ddspipe_core/dynamic/ParticipantsDatabase.hpp>
#include <ddspipe_core/interface/IParticipant.hpp>

#include <cpp_utils/exception/InconsistencyException.hpp>
#include <ddspipe_core/efficiency/payload/FastPayloadPool.hpp>

#include <ddsrecorder_participants/common/serialize/Serializer.hpp>
#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollection.hpp>
#include <ddsrecorder_participants/constants.hpp>
#include <ddsrecorder_participants/replayer/McapReaderParticipant.hpp>
#include <ddsrecorder_participants/replayer/ReplayerParticipant.hpp>
#include <ddsrecorder_participants/replayer/XmlReplayerParticipant.hpp>
#include <ddsrecorder_participants/replayer/SqlReaderParticipant.hpp>

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
    payload_pool_ = std::make_shared<ddspipe::core::FastPayloadPool>();

    bool is_sql_file = false;
    // Match the filename with a regex that captures the extension
    std::regex ext_regex(R"(.*\.([a-zA-Z0-9]+)$)");
    std::smatch match;

    if (std::regex_match(input_file, match, ext_regex))
    {
        // Capture the extension (e.g., "db", "DB", etc.)
        std::string ext = match[1];

        // Make extension lowercase for comparison
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        is_sql_file = (ext == "db");
    }

    // Create Reader Participant
    if (is_sql_file)
    {
        reader_participant_ = std::make_shared<participants::SqlReaderParticipant>(
            configuration.base_reader_configuration,
            payload_pool_,
            input_file);
    }
    else
    {
        reader_participant_ = std::make_shared<participants::McapReaderParticipant>(
            configuration.base_reader_configuration,
            payload_pool_,
            input_file);
    }

    // Create Discovery Database
    auto discovery_database = std::make_shared<ddspipe::core::DiscoveryDatabase>();

    // Create Replayer Participant
    std::shared_ptr<ddspipe::core::IParticipant> replayer_participant;

    if(configuration.xml_enabled)
    {
        replayer_participant = std::make_shared<participants::XmlReplayerParticipant>(
            configuration.replayer_configuration,
            payload_pool_,
            discovery_database,
            configuration.replay_types);

        std::dynamic_pointer_cast<participants::XmlReplayerParticipant>(replayer_participant)->init();
    }
    else
    {
        replayer_participant = std::make_shared<participants::ReplayerParticipant>(
            std::dynamic_pointer_cast<ddspipe::participants::SimpleParticipantConfiguration>(configuration.replayer_configuration),
            payload_pool_,
            discovery_database,
            configuration.replay_types);

        std::dynamic_pointer_cast<participants::ReplayerParticipant>(replayer_participant)->init();
    }

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

    // Process the input file's summary
    std::set<utils::Heritable<ddspipe::core::types::DdsTopic>> topics;
    participants::DynamicTypesCollection types;

    reader_participant_->process_summary(topics, types);

    std::map<std::string, fastdds::dds::xtypes::TypeIdentifierPair> registered_types;
    if (configuration.replay_types)
    {
        // Register the dynamic types
        registered_types = register_dynamic_types_(types);
    }

    // Store the topics as built-in topics
    for (auto& topic : topics)
    {
        auto it = registered_types.find(topic->type_name);

        if (it != registered_types.end())
        {
            topic->type_identifiers = it->second;
        }

        configuration.ddspipe_configuration.builtin_topics.insert(topic);
    }

    // Create DDS Pipe
    pipe_ = std::make_unique<ddspipe::core::DdsPipe>(
        configuration.ddspipe_configuration,
        discovery_database,
        payload_pool_,
        participants_database,
        thread_pool_);
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

std::map<std::string, fastdds::dds::xtypes::TypeIdentifierPair> DdsReplayer::register_dynamic_types_(
        const participants::DynamicTypesCollection& dynamic_types)
{
    std::map<std::string, fastdds::dds::xtypes::TypeIdentifierPair> registered_types{};

    std::cout << "Registering dynamic types..." << dynamic_types.dynamic_types().size() << std::endl;

    for (const auto& dynamic_type : dynamic_types.dynamic_types())
    {
        // Deserialize type identifier
        const auto type_identifier_str = utils::base64_decode(dynamic_type.type_identifier());
        fastdds::dds::xtypes::TypeIdentifier type_identifier;
        participants::Serializer::deserialize<fastdds::dds::xtypes::TypeIdentifier>(type_identifier_str,
                type_identifier);

        // Deserialize type object
        const auto type_object_str = utils::base64_decode(dynamic_type.type_object());
        fastdds::dds::xtypes::TypeObject type_object;
        participants::Serializer::deserialize<fastdds::dds::xtypes::TypeObject>(type_object_str, type_object);

        // Register in factory
        fastdds::dds::xtypes::TypeIdentifierPair type_identifiers;
        type_identifiers.type_identifier1(type_identifier);
        fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().register_type_object(
            type_object, type_identifiers);

        std::cout << "Registered type: " << dynamic_type.type_name() << std::endl;

        registered_types.insert({dynamic_type.type_name(), type_identifiers});
    }

    return registered_types;
}

} /* namespace replayer */
} /* namespace ddsrecorder */
} /* namespace eprosima */
