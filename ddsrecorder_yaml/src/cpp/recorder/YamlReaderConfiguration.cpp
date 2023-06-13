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

#include <ddspipe_core/types/dynamic_types/types.hpp>
#include <ddspipe_core/types/topic/filter/WildcardDdsFilterTopic.hpp>
#include <ddspipe_participants/types/address/Address.hpp>

#include <ddspipe_yaml/yaml_configuration_tags.hpp>
#include <ddspipe_yaml/Yaml.hpp>
#include <ddspipe_yaml/YamlManager.hpp>

#include <ddsrecorder_yaml/recorder/yaml_configuration_tags.hpp>

#include <ddsrecorder_yaml/recorder/YamlReaderConfiguration.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace yaml {

using namespace eprosima::ddspipe::core;
using namespace eprosima::ddspipe::core::types;
using namespace eprosima::ddspipe::participants;
using namespace eprosima::ddspipe::participants::types;
using namespace eprosima::ddspipe::yaml;

RecorderConfiguration::RecorderConfiguration(
        const Yaml& yml)
{
    load_ddsrecorder_configuration_(yml);
}

RecorderConfiguration::RecorderConfiguration(
        const std::string& file_path)
{
    load_ddsrecorder_configuration_from_file_(file_path);
}

void RecorderConfiguration::load_ddsrecorder_configuration_(
        const Yaml& yml)
{
    try
    {
        YamlReaderVersion version = LATEST;

        ////////////////////////////////////////
        // Create participants configurations //
        ////////////////////////////////////////

        /////
        // Create Simple Participant Configuration
        simple_configuration = std::make_shared<SimpleParticipantConfiguration>();
        simple_configuration->id = "SimpleRecorderParticipant";
        simple_configuration->is_repeater = false;

        /////
        // Create Recorder Participant Configuration
        recorder_configuration = std::make_shared<ParticipantConfiguration>();
        recorder_configuration->id = "RecorderRecorderParticipant";
        recorder_configuration->is_repeater = false;

        /////
        // Get optional Recorder configuration options
        if (YamlReader::is_tag_present(yml, RECORDER_RECORDER_TAG))
        {
            auto recorder_yml = YamlReader::get_value_in_tag(yml, RECORDER_RECORDER_TAG);
            load_recorder_configuration_(recorder_yml, version);
        }

        // Initialize cleanup_period with twice the value of event_window
        // WARNING: event_window tag (under recorder tag) must have been parsed beforehand
        cleanup_period = 2 * event_window;

        /////
        // Get optional specs configuration
        // WARNING: Parse builtin topics (dds tag) AFTER specs, as some topic-specific default values are set there
        if (YamlReader::is_tag_present(yml, SPECS_TAG))
        {
            auto specs_yml = YamlReader::get_value_in_tag(yml, SPECS_TAG);
            load_specs_configuration_(specs_yml, version);
        }

        /////
        // Get optional DDS configuration options
        if (YamlReader::is_tag_present(yml, RECORDER_DDS_TAG))
        {
            auto dds_yml = YamlReader::get_value_in_tag(yml, RECORDER_DDS_TAG);
            load_dds_configuration_(dds_yml, version);
        }

        // Block ROS 2 services (RPC) topics
        // RATIONALE:
        // At the time of this writting, services in ROS 2 behave in the following manner: a ROS 2 service
        // client awaits to discover a server, and it is then when a request is sent to this (and only this) server,
        // from which a response is expected.
        // Hence, if these topics are not blocked, the client would wrongly believe DDS-Recorder is a server, thus
        // sending a request for which a response will not be received.
        WildcardDdsFilterTopic rpc_request_topic, rpc_response_topic;
        rpc_request_topic.topic_name.set_value("rq/*");
        rpc_response_topic.topic_name.set_value("rr/*");
        blocklist.insert(
            utils::Heritable<WildcardDdsFilterTopic>::make_heritable(rpc_request_topic));
        blocklist.insert(
            utils::Heritable<WildcardDdsFilterTopic>::make_heritable(rpc_response_topic));

        // Initialize controller domain with the same as the one being recorded
        // WARNING: dds tag must have been parsed beforehand
        controller_domain = simple_configuration->domain;

        /////
        // Get optional remote controller configuration
        if (YamlReader::is_tag_present(yml, RECORDER_REMOTE_CONTROLLER_TAG))
        {
            auto controller_yml = YamlReader::get_value_in_tag(yml, RECORDER_REMOTE_CONTROLLER_TAG);
            load_controller_configuration_(controller_yml, version);
        }
    }
    catch (const std::exception& e)
    {
        throw eprosima::utils::ConfigurationException(
                  utils::Formatter() << "Error loading DDS Recorder configuration from yaml:\n " << e.what());
    }
}

void RecorderConfiguration::load_recorder_configuration_(
        const Yaml& yml,
        const YamlReaderVersion& version)
{
    if (YamlReader::is_tag_present(yml, RECORDER_OUTPUT_TAG))
    {
        auto output_yml = YamlReader::get_value_in_tag(yml, RECORDER_OUTPUT_TAG);

        // Default path and output values ("./output")
        std::string path = ".";
        std::string filename = "output";

        /////
        // Get optional file path
        if (YamlReader::is_tag_present(output_yml, RECORDER_PATH_FILE_TAG))
        {
            path = YamlReader::get<std::string>(output_yml, RECORDER_PATH_FILE_TAG, version);
        }

        /////
        // Get optional file name
        if (YamlReader::is_tag_present(output_yml, RECORDER_FILE_NAME_TAG))
        {
            filename = YamlReader::get<std::string>(output_yml, RECORDER_FILE_NAME_TAG, version);
        }

        // Generate complete output file name
        recorder_output_file = path + "/" + filename;
    }

    /////
    // Get optional buffer size
    if (YamlReader::is_tag_present(yml, RECORDER_BUFFER_SIZE_TAG))
    {
        buffer_size = YamlReader::get_positive_int(yml, RECORDER_BUFFER_SIZE_TAG);
    }

    /////
    // Get optional event window length
    if (YamlReader::is_tag_present(yml, RECORDER_EVENT_WINDOW_TAG))
    {
        event_window = YamlReader::get_positive_int(yml, RECORDER_EVENT_WINDOW_TAG);
    }

    /////
    // Get optional log publishTime
    if (YamlReader::is_tag_present(yml, RECORDER_LOG_PUBLISH_TIME_TAG))
    {
        log_publish_time = YamlReader::get<bool>(yml, RECORDER_LOG_PUBLISH_TIME_TAG, version);
    }

    /////
    // Get optional downsampling
    if (YamlReader::is_tag_present(yml, DOWNSAMPLING_TAG))
    {
        downsampling = YamlReader::get_positive_int(yml, DOWNSAMPLING_TAG);
        // Set default value for downsampling
        TopicQoS::default_downsampling.store(downsampling);
    }

    /////
    // Get optional max reception rate
    if (YamlReader::is_tag_present(yml, MAX_RECEPTION_RATE_TAG))
    {
        // Set default value for max reception rate
        TopicQoS::default_max_reception_rate.store(YamlReader::get_nonnegative_float(yml,
                MAX_RECEPTION_RATE_TAG));
    }

    /////
    // Get optional only_with_type
    if (YamlReader::is_tag_present(yml, RECORDER_ONLY_WITH_TYPE_TAG))
    {
        only_with_type = YamlReader::get<bool>(yml, RECORDER_ONLY_WITH_TYPE_TAG, version);
    }
}

void RecorderConfiguration::load_controller_configuration_(
        const Yaml& yml,
        const YamlReaderVersion& version)
{
    // Get optional enable remote controller
    if (YamlReader::is_tag_present(yml, RECORDER_REMOTE_CONTROLLER_ENABLE_TAG))
    {
        enable_remote_controller = YamlReader::get<bool>(yml, RECORDER_REMOTE_CONTROLLER_ENABLE_TAG,
                        version);
    }

    // Get optional DDS domain
    if (YamlReader::is_tag_present(yml, DOMAIN_ID_TAG))
    {
        controller_domain = YamlReader::get<DomainId>(yml, DOMAIN_ID_TAG, version);
    }

    // Get optional initial state
    if (YamlReader::is_tag_present(yml, RECORDER_REMOTE_CONTROLLER_INITIAL_STATE_TAG))
    {
        // Convert to enum and check valid wherever used to avoid mcap library dependency in YAML module
        initial_state = YamlReader::get<std::string>(yml,
                        RECORDER_REMOTE_CONTROLLER_INITIAL_STATE_TAG, version);
        // Case insensitive
        eprosima::utils::to_uppercase(initial_state);
    }

    // Get optional command topic name
    if (YamlReader::is_tag_present(yml, RECORDER_REMOTE_CONTROLLER_COMMAND_TOPIC_NAME_TAG))
    {
        command_topic_name = YamlReader::get<std::string>(yml,
                        RECORDER_REMOTE_CONTROLLER_COMMAND_TOPIC_NAME_TAG, version);
    }

    // Get optional status topic name
    if (YamlReader::is_tag_present(yml, RECORDER_REMOTE_CONTROLLER_STATUS_TOPIC_NAME_TAG))
    {
        status_topic_name = YamlReader::get<std::string>(yml,
                        RECORDER_REMOTE_CONTROLLER_STATUS_TOPIC_NAME_TAG, version);
    }
}

void RecorderConfiguration::load_specs_configuration_(
        const Yaml& yml,
        const YamlReaderVersion& version)
{
    // Get number of threads
    if (YamlReader::is_tag_present(yml, NUMBER_THREADS_TAG))
    {
        n_threads = YamlReader::get_positive_int(yml, NUMBER_THREADS_TAG);
    }

    // Get maximum history depth
    if (YamlReader::is_tag_present(yml, MAX_HISTORY_DEPTH_TAG))
    {
        max_history_depth = YamlReader::get_positive_int(yml, MAX_HISTORY_DEPTH_TAG);
        // Set default value for history
        TopicQoS::default_history_depth.store(max_history_depth);
    }

    // Get max pending samples
    if (YamlReader::is_tag_present(yml, RECORDER_SPECS_MAX_PENDING_SAMPLES_TAG))
    {
        max_pending_samples = YamlReader::get<int>(yml, RECORDER_SPECS_MAX_PENDING_SAMPLES_TAG, version);
        if (max_pending_samples < -1)
        {
            throw eprosima::utils::ConfigurationException(
                      utils::Formatter() << "Error reading value under tag <" << RECORDER_SPECS_MAX_PENDING_SAMPLES_TAG <<
                          "> : value cannot be lower than -1.");
        }
    }

    // Get cleanup period
    if (YamlReader::is_tag_present(yml, RECORDER_SPECS_CLEANUP_PERIOD_TAG))
    {
        cleanup_period = YamlReader::get_positive_int(yml, RECORDER_SPECS_CLEANUP_PERIOD_TAG);
    }
}

void RecorderConfiguration::load_dds_configuration_(
        const Yaml& yml,
        const YamlReaderVersion& version)
{
    // Get optional DDS domain
    if (YamlReader::is_tag_present(yml, DOMAIN_ID_TAG))
    {
        simple_configuration->domain = YamlReader::get<DomainId>(yml, DOMAIN_ID_TAG, version);
    }

    /////
    // Get optional whitelist interfaces
    if (YamlReader::is_tag_present(yml, WHITELIST_INTERFACES_TAG))
    {
        simple_configuration->whitelist = YamlReader::get_set<IpType>(yml, WHITELIST_INTERFACES_TAG,
                        version);
    }

    // Optional get Transport protocol
    if (YamlReader::is_tag_present(yml, TRANSPORT_DESCRIPTORS_TRANSPORT_TAG))
    {
        simple_configuration->transport = YamlReader::get<TransportDescriptors>(yml, TRANSPORT_DESCRIPTORS_TRANSPORT_TAG, version);
    }
    else
    {
        simple_configuration->transport = TransportDescriptors::builtin;
    }

    // Optional get ignore participant flags
    if (YamlReader::is_tag_present(yml, IGNORE_PARTICIPANT_FLAGS_TAG))
    {
        simple_configuration->ignore_participant_flags = YamlReader::get<IgnoreParticipantFlags>(yml, IGNORE_PARTICIPANT_FLAGS_TAG, version);
    }
    else
    {
        simple_configuration->ignore_participant_flags = IgnoreParticipantFlags::no_filter;
    }

    /////
    // Get optional allowlist
    if (YamlReader::is_tag_present(yml, ALLOWLIST_TAG))
    {
        allowlist = YamlReader::get_set<utils::Heritable<IFilterTopic>>(yml, ALLOWLIST_TAG, version);

        // Add to allowlist always the type object topic
        WildcardDdsFilterTopic internal_topic;
        internal_topic.topic_name.set_value(TYPE_OBJECT_TOPIC_NAME);
        allowlist.insert(
            utils::Heritable<WildcardDdsFilterTopic>::make_heritable(internal_topic));
    }

    /////
    // Get optional blocklist
    if (YamlReader::is_tag_present(yml, BLOCKLIST_TAG))
    {
        blocklist = YamlReader::get_set<utils::Heritable<IFilterTopic>>(yml, BLOCKLIST_TAG, version);
    }

    /////
    // Get optional builtin topics
    if (YamlReader::is_tag_present(yml, BUILTIN_TAG))
    {
        // WARNING: Parse builtin topics AFTER specs and recorder, as some topic-specific default values are set there
        builtin_topics = YamlReader::get_set<utils::Heritable<DistributedTopic>>(yml, BUILTIN_TAG,
                        version);
    }
}

void RecorderConfiguration::load_ddsrecorder_configuration_from_file_(
        const std::string& file_path)
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
                  utils::Formatter() << "Error loading DDS Recorder configuration from file: <" << file_path <<
                      "> :\n " << e.what());
    }

    RecorderConfiguration::load_ddsrecorder_configuration_(yml);
}

} /* namespace yaml */
} /* namespace ddsrecorder */
} /* namespace eprosima */
