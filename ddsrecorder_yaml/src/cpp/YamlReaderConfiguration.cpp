// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <ddsrecorder/configuration/participant/DiscoveryServerParticipantConfiguration.hpp>
#include <ddsrecorder/configuration/participant/ParticipantConfiguration.hpp>
#include <ddsrecorder/configuration/participant/SimpleParticipantConfiguration.hpp>
#include <ddsrecorder/configuration/participant/recorder/RecorderConfiguration.hpp>
#include <ddsrecorder/types/topic/filter/DdsFilterTopic.hpp>
#include <ddsrecorder/types/topic/dds/DdsTopic.hpp>
#include <ddsrecorder/types/topic/filter/WildcardDdsFilterTopic.hpp>

#include <ddsrecorder_yaml/Yaml.hpp>
#include <ddsrecorder_yaml/YamlReaderConfiguration.hpp>
#include <ddsrecorder_yaml/YamlReader.hpp>
#include <ddsrecorder_yaml/YamlManager.hpp>
#include <ddsrecorder_yaml/yaml_configuration_tags.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace yaml {

using namespace eprosima::ddsrecorder::core;

core::configuration::DDSRouterConfiguration
YamlReaderConfiguration::load_ddsrouter_configuration(
        const Yaml& yml)
{
    try
    {
        core::configuration::DDSRouterConfiguration configuration;
        YamlReaderVersion version = LATEST;

        /////
        // Get optional allowlist
        if (YamlReader::is_tag_present(yml, ALLOWLIST_TAG))
        {
            configuration.allowlist = utils::convert_set_to_shared<types::DdsFilterTopic, types::WildcardDdsFilterTopic>(
                YamlReader::get_set<types::WildcardDdsFilterTopic>(yml, ALLOWLIST_TAG, version));
        }

        /////
        // Get optional blocklist
        if (YamlReader::is_tag_present(yml, BLOCKLIST_TAG))
        {
            configuration.blocklist = utils::convert_set_to_shared<types::DdsFilterTopic, types::WildcardDdsFilterTopic>(
                YamlReader::get_set<types::WildcardDdsFilterTopic>(yml, BLOCKLIST_TAG, version));
        }

        /////
        // Get optional domain
        std::shared_ptr<configuration::SimpleParticipantConfiguration> simple_configuration = std::make_shared<configuration::SimpleParticipantConfiguration>();
        // Set the domain in a SimplePart Config
        if (is_tag_present(yml, DOMAIN_ID_TAG))
        {
            simple_configuration->domain = get<types::DomainId>(yml, DOMAIN_ID_TAG, version);
        }

        /////
        // Get optional file path
        std::string path = "./";
        if (is_tag_present(yml, RECORDER_PATH_FILE_TAG))
        {
            path = get<std::string>(yml, RECORDER_PATH_FILE_TAG, version);
        }

        /////
        // Get optional file extension
        std::string extension = ".mcap";
        if (is_tag_present(yml, RECORDER_EXTENSION_FILE_TAG))
        {
            extension = get<std::string>(yml, RECORDER_EXTENSION_FILE_TAG, version);
        }

        /////
        // Get optional file use timestamp
        bool use_timestamp = true;
        if (is_tag_present(yml, RECORDER_USE_TIMESTAMP_FILE_NAME_TAG))
        {
            extension = get<bool>(yml, RECORDER_USE_TIMESTAMP_FILE_NAME_TAG, version);
        }

        /////
        // Get mandatory file name
        std::string filename = get<std::string>(yml, RECORDER_FILE_NAME_TAG, version);

        /////
        // Fill the Recorder configuration
        std::shared_ptr<configuration::RecorderConfiguration> recorder_configuration = std::make_shared<configuration::RecorderConfiguration>(
            filename,
            path,
            extension,
            use_timestamp
        );

        /////
        // Fill Participant configurations in router configuration
        configuration.participants_configurations.insert(
            simple_configuration);
        configuration.participants_configurations.insert(
            recorder_configuration);

        return configuration;
    }
    catch (const std::exception& e)
    {
        throw eprosima::utils::ConfigurationException(
                  utils::Formatter() << "Error loading DDS Router configuration from yaml:\n " << e.what());
    }
}

core::configuration::DDSRouterConfiguration
YamlReaderConfiguration::load_ddsrouter_configuration_from_file(
        const std::string& file_path)
{
    yaml::Yaml yml;

    // Load file
    try
    {
        yml = yaml::YamlManager::load_file(file_path);
    }
    catch (const std::exception& e)
    {
        throw eprosima::utils::ConfigurationException(
                  utils::Formatter() << "Error loading DDSRouter configuration from file: <" << file_path <<
                      "> :\n " << e.what());
    }

    return YamlReaderConfiguration::load_ddsrouter_configuration(yml);
}

YamlReaderVersion YamlReaderConfiguration::default_yaml_version()
{
    return V_1_0;
}

} /* namespace yaml */
} /* namespace ddsrecorder */
} /* namespace eprosima */
