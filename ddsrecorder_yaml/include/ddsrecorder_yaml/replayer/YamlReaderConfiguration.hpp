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
 * @file YamlReaderConfiguration.hpp
 */

#pragma once

#include <cpp_utils/memory/Heritable.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/types/Fuzzy.hpp>

#include <ddspipe_core/types/topic/dds/DistributedTopic.hpp>
#include <ddspipe_core/types/topic/filter/IFilterTopic.hpp>

#include <ddspipe_participants/configuration/SimpleParticipantConfiguration.hpp>

#include <ddspipe_yaml/Yaml.hpp>
#include <ddspipe_yaml/YamlReader.hpp>

#include <ddsrecorder_participants/replayer/McapReaderParticipantConfiguration.hpp>
#include <ddsrecorder_yaml/library/library_dll.h>

namespace eprosima {
namespace ddsrecorder {
namespace yaml {

/**
 * @brief Class that encapsulates specific methods to get a full DDS Replayer Configuration from a yaml node.
 *
 * TODO: Add version configuration so it could load different versions
 */
class DDSRECORDER_YAML_DllAPI ReplayerConfiguration
{
public:

    ReplayerConfiguration(
            const Yaml& yml);

    ReplayerConfiguration(
            const std::string& file_path);

    // Participants configurations
    std::shared_ptr<ddsrecorder::participants::McapReaderParticipantConfiguration> mcap_reader_configuration;
    std::shared_ptr<ddspipe::participants::SimpleParticipantConfiguration> replayer_configuration;

    // Topic filtering
    std::set<utils::Heritable<ddspipe::core::types::IFilterTopic>> allowlist{};
    std::set<utils::Heritable<ddspipe::core::types::IFilterTopic>> blocklist{};
    std::set<utils::Heritable<ddspipe::core::types::DistributedTopic>> builtin_topics{};

    // Replaying params
    std::string input_file;
    utils::Fuzzy<utils::Timestamp> begin_time{};
    utils::Fuzzy<utils::Timestamp> end_time{};
    float rate{1};
    utils::Fuzzy<utils::Timestamp> start_replay_time{};

    // Specs
    unsigned int n_threads = 12;
    unsigned int max_history_depth = 5000;

protected:

    void load_ddsreplayer_configuration_(
            const Yaml& yml);

    void load_replay_configuration_(
            const Yaml& yml,
            const ddspipe::yaml::YamlReaderVersion& version);

    void load_specs_configuration_(
            const Yaml& yml,
            const ddspipe::yaml::YamlReaderVersion& version);

    void load_dds_configuration_(
            const Yaml& yml,
            const ddspipe::yaml::YamlReaderVersion& version);

    void load_ddsreplayer_configuration_from_file_(
            const std::string& file_path);
};

} /* namespace yaml */
} /* namespace ddsrecorder */
} /* namespace eprosima */
