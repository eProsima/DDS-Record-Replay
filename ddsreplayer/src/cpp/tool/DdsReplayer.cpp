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
#include <vector>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>

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
#include <ddspipe_participants/utils/idl_types.hpp>

#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollection.hpp>
#include <ddsrecorder_participants/constants.hpp>
#include <ddsrecorder_participants/replayer/McapReaderParticipant.hpp>
#include <ddsrecorder_participants/replayer/ReplayerParticipant.hpp>
#include <ddsrecorder_participants/replayer/XmlReplayerParticipant.hpp>
#include <ddsrecorder_participants/replayer/SqlReaderParticipant.hpp>

#include "DdsReplayer.hpp"

#include <ddsrecorder_participants/replayer/DynamicTypesSupport.hpp>

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

    if (configuration.dds_enabled)
    {
        replayer_participant = std::make_shared<participants::XmlReplayerParticipant>(
            configuration.replayer_configuration,
            payload_pool_,
            discovery_database,
            configuration.replay_types,
            configuration.replayer_configuration->domain);

        std::dynamic_pointer_cast<participants::XmlReplayerParticipant>(replayer_participant)->init();
    }
    else
    {
        replayer_participant = std::make_shared<participants::ReplayerParticipant>(
            std::dynamic_pointer_cast<ddspipe::participants::SimpleParticipantConfiguration>(configuration.
                    replayer_configuration),
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

    // adds the allowed partitions list (this will ensure to not add the blocked data)
    reader_participant_->add_partition_list(configuration.replayer_configuration->allowed_partition_list);

    reader_participant_->process_summary(topics, types);

    std::map<std::string, fastdds::dds::xtypes::TypeIdentifierPair> registered_types;
    if (configuration.replay_types)
    {
        // Register the dynamic types
        registered_types = register_dynamic_types_(types);

        // Complete them with the types loaded from IDL
        register_idl_types_(registered_types);
    }
    else if (!configuration.replayer_configuration->idl_types_paths.empty())
    {
        EPROSIMA_LOG_WARNING(DDSREPLAYER_EXECUTION,
                "The IDL types paths are ignored, as the types are not replayed.");
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
    // Update the partitions
    reader_participant_->update_partition_list(new_configuration.replayer_configuration->allowed_partition_list);
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
    return participants::detail::type_identifiers_from(participants::detail::register_dynamic_types(dynamic_types));
}

void DdsReplayer::register_idl_types_(
        std::map<std::string, fastdds::dds::xtypes::TypeIdentifierPair>& registered_types)
{
    const auto& idl_types_paths = configuration_.replayer_configuration->idl_types_paths;

    if (idl_types_paths.empty())
    {
        return;
    }

    // Problems are logged while loading, and the types that could be loaded are still used
    std::vector<fastdds::dds::DynamicType::_ref_type> idl_types;
    ddspipe::participants::load_idl_types({idl_types_paths.begin(), idl_types_paths.end()},
            configuration_.replayer_configuration->idl_include_paths, idl_types);

    auto& type_object_registry = fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry();

    for (const auto& type : idl_types)
    {
        const auto type_name = type->get_name().to_string();

        // The recorded types are kept, as the recorded data was serialized with them
        if (registered_types.find(type_name) != registered_types.end())
        {
            continue;
        }

        fastdds::dds::xtypes::TypeIdentifierPair type_identifiers;

        if (fastdds::dds::RETCODE_OK != type_object_registry.register_typeobject_w_dynamic_type(type, type_identifiers))
        {
            EPROSIMA_LOG_WARNING(DDSREPLAYER_EXECUTION, "Failed to register type " << type_name << " loaded from IDL.");
            continue;
        }

        registered_types[type_name] = type_identifiers;
    }
}

} /* namespace replayer */
} /* namespace ddsrecorder */
} /* namespace eprosima */
