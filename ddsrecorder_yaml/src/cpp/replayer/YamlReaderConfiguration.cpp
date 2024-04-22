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

/**
 * @file YamlReaderConfiguration.cpp
 *
 */

#include <cpp_utils/utils.hpp>

#include <ddspipe_core/configuration/DdsPipeLogConfiguration.hpp>
#include <ddspipe_core/types/dynamic_types/types.hpp>
#include <ddspipe_core/types/topic/filter/ManualTopic.hpp>
#include <ddspipe_core/types/topic/filter/WildcardDdsFilterTopic.hpp>

#include <ddspipe_participants/writer/rtps/CommonWriter.hpp>

#include <ddspipe_yaml/yaml_configuration_tags.hpp>
#include <ddspipe_yaml/Yaml.hpp>
#include <ddspipe_yaml/YamlManager.hpp>

#include <ddsrecorder_yaml/replayer/yaml_configuration_tags.hpp>
#include <ddsrecorder_yaml/replayer/YamlReaderConfiguration.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace yaml {

using namespace eprosima::ddspipe::core;
using namespace eprosima::ddspipe::core::types;
using namespace eprosima::ddspipe::participants;
using namespace eprosima::ddspipe::participants::rtps;
using namespace eprosima::ddspipe::participants::types;
using namespace eprosima::ddspipe::yaml;
using namespace eprosima::ddsrecorder::participants;

ReplayerConfiguration::ReplayerConfiguration(
        const Yaml& yml,
        const CommandlineArgsReplayer* args /*= nullptr*/)
{
    load_ddsreplayer_configuration_(yml, args);
}

ReplayerConfiguration::ReplayerConfiguration(
        const std::string& file_path,
        const CommandlineArgsReplayer* args /*= nullptr*/)
{
    load_ddsreplayer_configuration_from_file_(file_path, args);
}

void ReplayerConfiguration::load_ddsreplayer_configuration_(
        const Yaml& yml,
        const CommandlineArgsReplayer* args)
{
    try
    {
        YamlReaderVersion version = LATEST;

        /////
        // Get optional Replayer configuration options
        if (YamlReader::is_tag_present(yml, REPLAYER_REPLAY_TAG))
        {
            auto replayer_yml = YamlReader::get_value_in_tag(yml, REPLAYER_REPLAY_TAG);
            load_replay_configuration_(replayer_yml, version);
        }

        /////
        // Get optional specs configuration
        if (YamlReader::is_tag_present(yml, SPECS_TAG))
        {
            auto specs_yml = YamlReader::get_value_in_tag(yml, SPECS_TAG);
            load_specs_configuration_(specs_yml, version);
        }

        ////////////////////////////////////////
        // Create participants configurations //
        ////////////////////////////////////////

        /////
        // Create MCAP Reader Participant Configuration
        // WARNING: Replayer configuration must have been parsed beforehand
        mcap_reader_configuration = std::make_shared<McapReaderParticipantConfiguration>();
        mcap_reader_configuration->id = "McapReaderParticipant";
        mcap_reader_configuration->app_id = "DDS_REPLAYER";
        mcap_reader_configuration->app_metadata = "";
        mcap_reader_configuration->is_repeater = false;
        mcap_reader_configuration->begin_time = begin_time;
        mcap_reader_configuration->end_time = end_time;
        mcap_reader_configuration->rate = rate;
        mcap_reader_configuration->start_replay_time = start_replay_time;

        /////
        // Create Replayer Participant Configuration
        replayer_configuration = std::make_shared<SimpleParticipantConfiguration>();
        replayer_configuration->id = "ReplayerParticipant";
        replayer_configuration->app_id = "DDS_REPLAYER";
        // TODO: fill metadata field once its content has been defined.
        replayer_configuration->app_metadata = "";
        replayer_configuration->is_repeater = false;

        /////
        // Get optional DDS configuration options
        // NOTE: Replayer Participant Configuration must have already been created
        if (YamlReader::is_tag_present(yml, REPLAYER_DDS_TAG))
        {
            auto dds_yml = YamlReader::get_value_in_tag(yml, REPLAYER_DDS_TAG);
            load_dds_configuration_(dds_yml, version);
        }

        // Block ROS 2 services (RPC) topics
        // RATIONALE:
        // At the time of this writing, services in ROS 2 behave in the following manner: a ROS 2 service
        // client awaits to discover a server, and it is then when a request is sent to this (and only this) server,
        // from which a response is expected.
        // Hence, if these topics are not blocked, the client would wrongly believe DDS-Replayer is a server, thus
        // sending a request for which a response will not be received.
        WildcardDdsFilterTopic rpc_request_topic, rpc_response_topic;
        rpc_request_topic.topic_name.set_value("rq/*");
        rpc_response_topic.topic_name.set_value("rr/*");

        ddspipe_configuration.blocklist.insert(
            utils::Heritable<WildcardDdsFilterTopic>::make_heritable(rpc_request_topic));

        ddspipe_configuration.blocklist.insert(
            utils::Heritable<WildcardDdsFilterTopic>::make_heritable(rpc_response_topic));

        ddspipe_configuration.init_enabled = true;

        // Don't trigger the DdsPipe's callbacks when discovering or removing external entities
        ddspipe_configuration.discovery_trigger = DiscoveryTrigger::NONE;

        /////
        // Log Configuration's set methods: Depending on where Log Configuration has been configured
        // (Yaml, Command-Line and/or by default) these methods will set DdsPipeConfiguration's log_configuration
        // taking into account this precedence:
        //  1. Log Configuration set on Command-line.
        //  2. Log Configuration set by YAML.
        //  3. Log Configuration set by default.
        if (args != nullptr)
        {
            ddspipe_configuration.log_configuration.set(args->log_verbosity);
            ddspipe_configuration.log_configuration.set(args->log_filter);
        }
    }
    catch (const std::exception& e)
    {
        throw eprosima::utils::ConfigurationException(
                  utils::Formatter() << "Error loading DDS Replayer configuration from yaml:\n " << e.what());
    }

}

void ReplayerConfiguration::load_replay_configuration_(
        const Yaml& yml,
        const YamlReaderVersion& version)
{
    // Get optional input_file
    if (YamlReader::is_tag_present(yml, REPLAYER_REPLAY_INPUT_TAG))
    {
        input_file = YamlReader::get<std::string>(yml, REPLAYER_REPLAY_INPUT_TAG, version);
    }

    // Get optional begin_time
    if (YamlReader::is_tag_present(yml, REPLAYER_REPLAY_BEGIN_TAG))
    {
        begin_time = YamlReader::get<utils::Timestamp>(yml, REPLAYER_REPLAY_BEGIN_TAG, version);
    }

    // Get optional end_time
    if (YamlReader::is_tag_present(yml, REPLAYER_REPLAY_END_TAG))
    {
        end_time = YamlReader::get<utils::Timestamp>(yml, REPLAYER_REPLAY_END_TAG, version);
    }

    // Assert begin_time prior to end_time
    if ((begin_time.is_set() && end_time.is_set()) && begin_time.get_reference() >= end_time.get_reference())
    {
        throw eprosima::utils::ConfigurationException(
                  utils::Formatter() << "Error loading DDS Replayer configuration from yaml:\n "
                                     << "begin_time must be earlier than end_time");
    }

    // Get optional rate
    if (YamlReader::is_tag_present(yml, REPLAYER_REPLAY_RATE_TAG))
    {
        rate = YamlReader::get_positive_float(yml, REPLAYER_REPLAY_RATE_TAG);
    }

    // Get optional start_replay_time
    if (YamlReader::is_tag_present(yml, REPLAYER_REPLAY_START_TIME_TAG))
    {
        start_replay_time = YamlReader::get<utils::Timestamp>(yml, REPLAYER_REPLAY_START_TIME_TAG, version);
    }

    // Get optional replay_types
    if (YamlReader::is_tag_present(yml, REPLAYER_REPLAY_TYPES_TAG))
    {
        replay_types = YamlReader::get<bool>(yml, REPLAYER_REPLAY_TYPES_TAG, version);
    }
}

void ReplayerConfiguration::load_specs_configuration_(
        const Yaml& yml,
        const YamlReaderVersion& version)
{
    // Get number of threads
    if (YamlReader::is_tag_present(yml, NUMBER_THREADS_TAG))
    {
        n_threads = YamlReader::get_positive_int(yml, NUMBER_THREADS_TAG);
    }

    /////
    // Get optional Topic QoS
    if (YamlReader::is_tag_present(yml, SPECS_QOS_TAG))
    {
        YamlReader::fill<TopicQoS>(topic_qos, YamlReader::get_value_in_tag(yml, SPECS_QOS_TAG), version);
        TopicQoS::default_topic_qos.set_value(topic_qos);
    }

    // Get wait all acknowledged timeout
    if (YamlReader::is_tag_present(yml, WAIT_ALL_ACKED_TIMEOUT_TAG))
    {
        // Set value for static attribute
        CommonWriter::wait_all_acked_timeout.store(YamlReader::get_nonnegative_int(yml, WAIT_ALL_ACKED_TIMEOUT_TAG));
    }

    /////
    // Get optional Log Configuration
    if (YamlReader::is_tag_present(yml, LOG_CONFIGURATION_TAG))
    {
        ddspipe_configuration.log_configuration = YamlReader::get<DdsPipeLogConfiguration>(yml, LOG_CONFIGURATION_TAG,
                        version);
    }
}

void ReplayerConfiguration::load_dds_configuration_(
        const Yaml& yml,
        const YamlReaderVersion& version)
{
    // Get optional DDS domain
    if (YamlReader::is_tag_present(yml, DOMAIN_ID_TAG))
    {
        replayer_configuration->domain = YamlReader::get<DomainId>(yml, DOMAIN_ID_TAG, version);
    }

    /////
    // Get optional whitelist interfaces
    if (YamlReader::is_tag_present(yml, WHITELIST_INTERFACES_TAG))
    {
        replayer_configuration->whitelist = YamlReader::get_set<IpType>(yml, WHITELIST_INTERFACES_TAG,
                        version);
    }

    // Optional get Transport protocol
    if (YamlReader::is_tag_present(yml, TRANSPORT_DESCRIPTORS_TRANSPORT_TAG))
    {
        replayer_configuration->transport = YamlReader::get<TransportDescriptors>(yml,
                        TRANSPORT_DESCRIPTORS_TRANSPORT_TAG,
                        version);
    }
    else
    {
        replayer_configuration->transport = TransportDescriptors::builtin;
    }

    // Optional get ignore participant flags
    if (YamlReader::is_tag_present(yml, IGNORE_PARTICIPANT_FLAGS_TAG))
    {
        replayer_configuration->ignore_participant_flags = YamlReader::get<IgnoreParticipantFlags>(yml,
                        IGNORE_PARTICIPANT_FLAGS_TAG,
                        version);
    }
    else
    {
        replayer_configuration->ignore_participant_flags = IgnoreParticipantFlags::no_filter;
    }

    /////
    // Get optional allowlist
    if (YamlReader::is_tag_present(yml, ALLOWLIST_TAG))
    {
        ddspipe_configuration.allowlist = YamlReader::get_set<utils::Heritable<IFilterTopic>>(yml, ALLOWLIST_TAG,
                        version);

        // Add to allowlist always the type object topic
        WildcardDdsFilterTopic internal_topic;
        internal_topic.topic_name.set_value(TYPE_OBJECT_TOPIC_NAME);
        ddspipe_configuration.allowlist.insert(
            utils::Heritable<WildcardDdsFilterTopic>::make_heritable(internal_topic));
    }

    /////
    // Get optional blocklist
    if (YamlReader::is_tag_present(yml, BLOCKLIST_TAG))
    {
        ddspipe_configuration.blocklist = YamlReader::get_set<utils::Heritable<IFilterTopic>>(yml, BLOCKLIST_TAG,
                        version);
    }

    /////
    // Get optional topics
    if (YamlReader::is_tag_present(yml, TOPICS_TAG))
    {
        const auto& manual_topics = YamlReader::get_list<ManualTopic>(yml, TOPICS_TAG, version);
        ddspipe_configuration.manual_topics =
                std::vector<ManualTopic>(manual_topics.begin(), manual_topics.end());
    }
}

void ReplayerConfiguration::load_ddsreplayer_configuration_from_file_(
        const std::string& file_path,
        const CommandlineArgsReplayer* args)
{
    Yaml yml;

    // Load file
    try
    {
        if (!file_path.empty())
        {
            yml = YamlManager::load_file(file_path);
        }
    }
    catch (const std::exception& e)
    {
        throw eprosima::utils::ConfigurationException(
                  utils::Formatter() << "Error loading DDS Replayer configuration from file: <" << file_path <<
                      "> :\n " << e.what());
    }

    ReplayerConfiguration::load_ddsreplayer_configuration_(yml, args);
}

} /* namespace yaml */
} /* namespace ddsrecorder */
} /* namespace eprosima */
