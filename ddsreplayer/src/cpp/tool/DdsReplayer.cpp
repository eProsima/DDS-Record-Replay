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

#include "DdsReplayer.hpp"

namespace eprosima {
namespace ddsrecorder {
namespace replayer {

using namespace eprosima::ddspipe::core;
using namespace eprosima::ddspipe::core::types;
using namespace eprosima::ddspipe::participants;
using namespace eprosima::ddspipe::participants::rtps;
using namespace eprosima::utils;

DdsReplayer::DdsReplayer(
        const yaml::ReplayerConfiguration& configuration,
        std::string& input_file)
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
    mcap_reader_participant_ = std::make_shared<participants::McapReaderParticipant>(
        configuration.mcap_reader_configuration,
        payload_pool_,
        input_file);

    // Create Replayer Participant
    replayer_participant_ = std::make_shared<participants::ReplayerParticipant>(
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

    // Cast to DdsTopic so both topic and type names are taken into account on lookups
    std::set<utils::Heritable<DdsTopic>> builtin_topics_dds{};
    for (auto topic : builtin_topics)
    {
        auto dds_topic = utils::Heritable<DdsTopic>(topic);
        builtin_topics_dds.insert(dds_topic);
    }

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
    status = mcap_reader.readSummary(mcap::ReadSummaryMethod::NoFallbackScan, onProblem);
    if (status.code != mcap::StatusCode::Success)
    {
        throw utils::InitializationException(
                  STR_ENTRY << "Failed to read summary."
                  );
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

        if (builtin_topics_dds.count(channel_topic) == 1)
        {
            // Already present in builtin_topics list, using qos provided through configuration
            continue;
        }

        // Use QoS stored in MCAP file (discovered when recording, or given to recorder's builtin topics list)
        channel_topic->topic_qos = deserialize_qos_(it->second->metadata["qos"]);

        builtin_topics.insert(channel_topic);
    }
    mcap_reader.close();

    return builtin_topics;
}

TopicQoS DdsReplayer::deserialize_qos_(
        const std::string& qos_str)
{
    // TODO: Reuse code from ddspipe_yaml

    TopicQoS qos{};

    YAML::Node qos_yaml = YAML::Load(qos_str);
    bool reliable = qos_yaml["reliability"].as<bool>();
    bool transient_local = qos_yaml["durability"].as<bool>();
    bool exclusive_ownership = qos_yaml["ownership"].as<bool>();
    bool keyed = qos_yaml["keyed"].as<bool>();

    // Parse reliability
    if (reliable)
    {
        qos.reliability_qos = eprosima::ddspipe::core::types::ReliabilityKind::RELIABLE;
    }
    else
    {
        qos.reliability_qos = eprosima::ddspipe::core::types::ReliabilityKind::BEST_EFFORT;
    }

    // Parse durability
    if (transient_local)
    {
        qos.durability_qos = eprosima::ddspipe::core::types::DurabilityKind::TRANSIENT_LOCAL;
    }
    else
    {
        qos.durability_qos = eprosima::ddspipe::core::types::DurabilityKind::VOLATILE;
    }

    // Parse ownership
    if (exclusive_ownership)
    {
        qos.ownership_qos = eprosima::ddspipe::core::types::OwnershipQosPolicyKind::EXCLUSIVE_OWNERSHIP_QOS;
    }
    else
    {
        qos.ownership_qos = eprosima::ddspipe::core::types::OwnershipQosPolicyKind::SHARED_OWNERSHIP_QOS;
    }

    // Parse keyed
    qos.keyed = keyed;

    return qos;
}

} /* namespace replayer */
} /* namespace ddsrecorder */
} /* namespace eprosima */
