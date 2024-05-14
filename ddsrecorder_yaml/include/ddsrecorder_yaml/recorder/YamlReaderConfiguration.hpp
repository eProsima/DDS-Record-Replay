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

#include <mcap/mcap.hpp>

#include <cpp_utils/Formatter.hpp>
#include <cpp_utils/memory/Heritable.hpp>

#include <ddspipe_core/configuration/DdsPipeConfiguration.hpp>
#include <ddspipe_core/configuration/MonitorConfiguration.hpp>
#include <ddspipe_core/types/dds/TopicQoS.hpp>
#include <ddspipe_core/types/topic/dds/DistributedTopic.hpp>
#include <ddspipe_core/types/topic/filter/IFilterTopic.hpp>

#include <ddspipe_participants/configuration/ParticipantConfiguration.hpp>
#include <ddspipe_participants/configuration/SimpleParticipantConfiguration.hpp>

#include <ddspipe_yaml/Yaml.hpp>
#include <ddspipe_yaml/YamlReader.hpp>

#include <ddsrecorder_participants/recorder/output/OutputSettings.hpp>

#include <ddsrecorder_yaml/library/library_dll.h>
#include <ddsrecorder_yaml/recorder/CommandlineArgsRecorder.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace yaml {

/**
 * @brief Class that encapsulates specific methods to get a full DDS Recorder Configuration from a yaml node.
 *
 * TODO: Add version configuration so it could load different versions
 */
class DDSRECORDER_YAML_DllAPI RecorderConfiguration
{
public:

    RecorderConfiguration(
            const Yaml& yml,
            const CommandlineArgsRecorder* args = nullptr);

    RecorderConfiguration(
            const std::string& file_path,
            const CommandlineArgsRecorder* args = nullptr);

    virtual bool is_valid(
            utils::Formatter& error_msg) const noexcept;

    // DDS Pipe Configuration
    ddspipe::core::DdsPipeConfiguration ddspipe_configuration;

    // Participants configurations
    std::shared_ptr<ddspipe::participants::SimpleParticipantConfiguration> simple_configuration;
    std::shared_ptr<ddspipe::participants::ParticipantConfiguration> recorder_configuration;

    // Output file params
    participants::OutputLibrary output_library = participants::OutputLibrary::mcap;
    std::string output_filepath = ".";
    std::string output_filename = "output";
    std::string output_timestamp_format = "%Y-%m-%d_%H-%M-%S_%Z";
    bool output_local_timestamp = true;
    uint64_t safety_margin = 0;

    // Output resource limits
    bool output_resource_limits_file_rotation = false;
    std::uint64_t output_resource_limits_max_size = 0;
    std::uint64_t output_resource_limits_max_file_size = 0;

    // Recording params
    unsigned int buffer_size = 100;
    unsigned int event_window = 20;
    bool log_publish_time = false;
    bool only_with_type = false;
    mcap::McapWriterOptions mcap_writer_options{"ros2"};
    bool record_types = true;
    bool ros2_types = false;

    // Remote controller configuration
    bool enable_remote_controller = true;
    ddspipe::core::types::DomainId controller_domain;
    std::string initial_state = "RUNNING";
    std::string command_topic_name = "/ddsrecorder/command";
    std::string status_topic_name = "/ddsrecorder/status";

    // Specs
    unsigned int n_threads = 12;
    int max_pending_samples = 5000;  // -1 <-> no limit || 0 <-> no pending samples
    unsigned int cleanup_period;
    ddspipe::core::types::TopicQoS topic_qos{};
    ddspipe::core::MonitorConfiguration monitor_configuration{};

protected:

    void load_ddsrecorder_configuration_(
            const Yaml& yml,
            const CommandlineArgsRecorder* args);

    void load_recorder_configuration_(
            const Yaml& yml,
            const ddspipe::yaml::YamlReaderVersion& version);

    void load_controller_configuration_(
            const Yaml& yml,
            const ddspipe::yaml::YamlReaderVersion& version);

    void load_specs_configuration_(
            const Yaml& yml,
            const ddspipe::yaml::YamlReaderVersion& version);

    void load_dds_configuration_(
            const Yaml& yml,
            const ddspipe::yaml::YamlReaderVersion& version);

    void load_ddsrecorder_configuration_from_file_(
            const std::string& file_path,
            const CommandlineArgsRecorder* args);
};

} /* namespace yaml */
} /* namespace ddsrecorder */
} /* namespace eprosima */
