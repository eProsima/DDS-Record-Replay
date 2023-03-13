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

#include <ddspipe_yaml/yaml_configuration_tags.hpp>
#include <ddspipe_yaml/Yaml.hpp>
#include <ddspipe_yaml/YamlManager.hpp>
#include <ddspipe_yaml/YamlReader.hpp>

#include <ddsrecorder_yaml/yaml_configuration_tags.hpp>

#include <ddsrecorder_yaml/YamlReaderConfiguration.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace yaml {

using namespace eprosima::ddspipe::core;
using namespace eprosima::ddspipe::participants;
using namespace eprosima::ddspipe::yaml;

Configuration::Configuration(
        const Yaml& yml)
{
    load_ddsrecorder_configuration_(yml);
}

Configuration::Configuration(
        const std::string& file_path)
{
    load_ddsrecorder_configuration_from_file_(file_path);
}

void Configuration::load_ddsrecorder_configuration_(
        const Yaml& yml)
{
    try
    {
        YamlReaderVersion version = LATEST;

        // Default path and output values ("./output")
        std::string path = ".";
        std::string filename = "output";

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

            if (YamlReader::is_tag_present(recorder_yml, RECORDER_OUTPUT_TAG))
            {
                auto output_yml = YamlReader::get_value_in_tag(recorder_yml, RECORDER_OUTPUT_TAG);

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
            }

            /////
            // Get optional buffer size
            if (YamlReader::is_tag_present(recorder_yml, RECORDER_BUFFER_SIZE_TAG))
            {
                buffer_size = YamlReader::get_positive_int(recorder_yml, RECORDER_BUFFER_SIZE_TAG);
            }

            /////
            // Get optional event window length
            if (YamlReader::is_tag_present(recorder_yml, RECORDER_EVENT_WINDOW_TAG))
            {
                event_window = YamlReader::get_positive_int(recorder_yml, RECORDER_EVENT_WINDOW_TAG);
            }

            /////
            // Get optional log publishTime
            if (YamlReader::is_tag_present(recorder_yml, RECORDER_LOG_PUBLISH_TIME_TAG))
            {
                log_publish_time = YamlReader::get<bool>(recorder_yml, RECORDER_LOG_PUBLISH_TIME_TAG, version);
            }
        }

        // Initialize controller domain with the same as the one being recorded
        // WARNING: dds tag must have been parsed beforehand
        controller_domain = simple_configuration->domain;

        /////
        // Get optional remote controller configuration
        if (YamlReader::is_tag_present(yml, RECORDER_REMOTE_CONTROLLER_TAG))
        {
            auto controller_yml = YamlReader::get_value_in_tag(yml, RECORDER_REMOTE_CONTROLLER_TAG);

            // Get optional enable remote controller
            if (YamlReader::is_tag_present(controller_yml, RECORDER_REMOTE_CONTROLLER_ENABLE_TAG))
            {
                enable_remote_controller = YamlReader::get<bool>(controller_yml, RECORDER_REMOTE_CONTROLLER_ENABLE_TAG,
                                version);
            }

            // Get optional DDS domain
            if (YamlReader::is_tag_present(controller_yml, DOMAIN_ID_TAG))
            {
                controller_domain = YamlReader::get<types::DomainId>(controller_yml, DOMAIN_ID_TAG, version);
            }

            // Get optional initial state
            if (YamlReader::is_tag_present(controller_yml, RECORDER_REMOTE_CONTROLLER_INITIAL_STATE_TAG))
            {
                // Convert to enum and check valid wherever used to avoid mcap library dependency in YAML module
                initial_state = YamlReader::get<std::string>(controller_yml,
                                RECORDER_REMOTE_CONTROLLER_INITIAL_STATE_TAG, version);
                // Case insensitive
                eprosima::utils::to_uppercase(initial_state);
            }

            // Get optional command topic name
            if (YamlReader::is_tag_present(controller_yml, RECORDER_REMOTE_CONTROLLER_COMMAND_TOPIC_NAME_TAG))
            {
                command_topic_name = YamlReader::get<std::string>(controller_yml,
                                RECORDER_REMOTE_CONTROLLER_COMMAND_TOPIC_NAME_TAG, version);
            }

            // Get optional status topic name
            if (YamlReader::is_tag_present(controller_yml, RECORDER_REMOTE_CONTROLLER_STATUS_TOPIC_NAME_TAG))
            {
                status_topic_name = YamlReader::get<std::string>(controller_yml,
                                RECORDER_REMOTE_CONTROLLER_STATUS_TOPIC_NAME_TAG, version);
            }
        }

        // Initialize cleanup_period with twice the value of event_window
        // WARNING: event_window tag (under recorder tag) must have been parsed beforehand
        cleanup_period = 2 * event_window;

        /////
        // Get optional specs configuration
        // WARNING: Parse builtin topics AFTER specs, as some topic-specific default values are set there
        if (YamlReader::is_tag_present(yml, SPECS_TAG))
        {
            auto specs_yml = YamlReader::get_value_in_tag(yml, SPECS_TAG);

            // Get number of threads
            if (YamlReader::is_tag_present(specs_yml, NUMBER_THREADS_TAG))
            {
                n_threads = YamlReader::get_positive_int(specs_yml, NUMBER_THREADS_TAG);
            }

            // Get maximum history depth
            if (YamlReader::is_tag_present(specs_yml, MAX_HISTORY_DEPTH_TAG))
            {
                max_history_depth = YamlReader::get_positive_int(specs_yml, MAX_HISTORY_DEPTH_TAG);
                // Set default value for history
                types::TopicQoS::default_history_depth.store(max_history_depth);
            }

            // Get downsampling
            if (YamlReader::is_tag_present(specs_yml, DOWNSAMPLING_TAG))
            {
                downsampling = YamlReader::get_positive_int(specs_yml, DOWNSAMPLING_TAG);
                // Set default value for downsampling
                types::TopicQoS::default_downsampling.store(downsampling);
            }

            // Get max reception rate
            if (YamlReader::is_tag_present(specs_yml, MAX_RECEPTION_RATE_TAG))
            {
                max_reception_rate = YamlReader::get<unsigned int>(specs_yml, MAX_RECEPTION_RATE_TAG, version);
                // Set default value for max reception rate
                types::TopicQoS::default_max_reception_rate.store(max_reception_rate);
            }

            // Get max pending samples
            if (YamlReader::is_tag_present(specs_yml, RECORDER_SPECS_MAX_PENDING_SAMPLES_TAG))
            {
                max_pending_samples = YamlReader::get_positive_int(specs_yml, RECORDER_SPECS_MAX_PENDING_SAMPLES_TAG);
            }

            // Get cleanup period
            if (YamlReader::is_tag_present(specs_yml, RECORDER_SPECS_CLEANUP_PERIOD_TAG))
            {
                cleanup_period = YamlReader::get_positive_int(specs_yml, RECORDER_SPECS_CLEANUP_PERIOD_TAG);
            }
        }

        /////
        // Get optional DDS configuration options
        if (YamlReader::is_tag_present(yml, RECORDER_DDS_TAG))
        {
            auto dds_yml = YamlReader::get_value_in_tag(yml, RECORDER_DDS_TAG);

            // Get optional DDS domain
            if (YamlReader::is_tag_present(dds_yml, DOMAIN_ID_TAG))
            {
                simple_configuration->domain = YamlReader::get<types::DomainId>(dds_yml, DOMAIN_ID_TAG, version);
            }

            /////
            // Get optional allowlist
            if (YamlReader::is_tag_present(dds_yml, ALLOWLIST_TAG))
            {
                allowlist = YamlReader::get_set<utils::Heritable<types::IFilterTopic>>(dds_yml, ALLOWLIST_TAG, version);

                // Add to allowlist always the type object topic
                types::WildcardDdsFilterTopic internal_topic;
                internal_topic.topic_name.set_value(types::TYPE_OBJECT_TOPIC_NAME);
                allowlist.insert(
                    utils::Heritable<types::WildcardDdsFilterTopic>::make_heritable(internal_topic));
            }

            /////
            // Get optional blocklist
            if (YamlReader::is_tag_present(dds_yml, BLOCKLIST_TAG))
            {
                blocklist = YamlReader::get_set<utils::Heritable<types::IFilterTopic>>(dds_yml, BLOCKLIST_TAG, version);
            }

            /////
            // Get optional builtin topics
            if (YamlReader::is_tag_present(dds_yml, BUILTIN_TAG))
            {
                // WARNING: Parse builtin topics AFTER specs, as some topic-specific default values are set there
                builtin_topics = YamlReader::get_set<utils::Heritable<types::DistributedTopic>>(dds_yml, BUILTIN_TAG,
                                version);
            }
        }

        // Block controller's status and command topics
        types::WildcardDdsFilterTopic status_topic, command_topic;
        status_topic.type_name.set_value("DdsRecorderStatus");
        command_topic.type_name.set_value("DdsRecorderCommand");
        blocklist.insert(
            utils::Heritable<types::WildcardDdsFilterTopic>::make_heritable(status_topic));
        blocklist.insert(
            utils::Heritable<types::WildcardDdsFilterTopic>::make_heritable(command_topic));

        // Generate complete output file name
        recorder_output_file = path + "/" + filename;
    }
    catch (const std::exception& e)
    {
        throw eprosima::utils::ConfigurationException(
                  utils::Formatter() << "Error loading DDS Recorder configuration from yaml:\n " << e.what());
    }
}

void Configuration::load_ddsrecorder_configuration_from_file_(
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

    Configuration::load_ddsrecorder_configuration_(yml);
}

} /* namespace yaml */
} /* namespace ddsrecorder */
} /* namespace eprosima */
