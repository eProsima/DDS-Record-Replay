// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fstream>
#include <string>

#include <cpp_utils/exception/ConfigurationException.hpp>
#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <ddspipe_yaml/YamlReader.hpp>

#include <ddsrecorder_yaml/recorder/CommandlineArgsRecorder.hpp>
#include <ddsrecorder_yaml/recorder/YamlReaderConfiguration.hpp>
#include <ddsrecorder_yaml/replayer/CommandlineArgsReplayer.hpp>
#include <ddsrecorder_yaml/replayer/YamlReaderConfiguration.hpp>

using namespace eprosima;
using namespace eprosima::ddsrecorder::yaml;

// Writes yml_str to a file under the current (test binary) working directory and returns its path.
std::string write_temp_yaml(
        const std::string& filename,
        const std::string& yml_str)
{
    std::ofstream file(filename);
    file << yml_str;
    file.close();
    return filename;
}

/////////////////////////
// RECORDER
/////////////////////////

/**
 * Check that a Domain ID configured through the Command-Line overrides the one set in the YAML,
 * both for the DDS participant and for the remote controller.
 */
TEST(YamlReaderDdsRecorderReplayerTest, recorder_domain_cli_overrides_yaml)
{
    const char* yml_str =
            R"(
            dds:
              domain: 5
            recorder:
              sql:
                enable: true
        )";

    Yaml yml = YAML::Load(yml_str);

    CommandlineArgsRecorder commandline_args;
    commandline_args.domain.set_value(ddspipe::core::types::DomainId(10u));

    RecorderConfiguration configuration(yml, &commandline_args);

    ASSERT_EQ(configuration.dds_configuration->domain, ddspipe::core::types::DomainId(10u));
    ASSERT_EQ(configuration.controller_domain, ddspipe::core::types::DomainId(10u));
}

/**
 * Check that a 'max-pending-samples' value lower than -1 is rejected while loading the YAML.
 */
TEST(YamlReaderDdsRecorderReplayerTest, recorder_max_pending_samples_below_minus_one_throws)
{
    const char* yml_str =
            R"(
            recorder:
              max-pending-samples: -2
        )";

    Yaml yml = YAML::Load(yml_str);

    ASSERT_THROW(RecorderConfiguration configuration(yml), utils::ConfigurationException);
}

/**
 * Check that, when only 'max-size' is set for the SQL resource limits (and 'max-file-size' is left
 * unset), 'max-file-size' is copied from 'max-size' (the SQL handler only writes a single file).
 */
TEST(YamlReaderDdsRecorderReplayerTest, recorder_sql_resource_limits_max_size_copies_to_max_file_size)
{
    const char* yml_str =
            R"(
            recorder:
              sql:
                enable: true
                resource-limits:
                  max-size: "10MB"
        )";

    Yaml yml = YAML::Load(yml_str);

    RecorderConfiguration configuration(yml);

    ASSERT_EQ(
        configuration.sql_resource_limits.resource_limits_struct.max_file_size_,
        configuration.sql_resource_limits.resource_limits_struct.max_size_);
    ASSERT_NE(configuration.sql_resource_limits.resource_limits_struct.max_size_, 0u);
}

/**
 * Check that, when two manual topics share the same name, the last one's filter overwrites the
 * previous one in the resulting content_topic_filter_dict.
 */
TEST(YamlReaderDdsRecorderReplayerTest, recorder_duplicate_manual_topic_overwrites_filter)
{
    const char* yml_str =
            R"(
            dds:
              topics:
                - name: "duplicated_topic"
                  filter: "1"
                - name: "duplicated_topic"
                  filter: "2"
        )";

    Yaml yml = YAML::Load(yml_str);

    RecorderConfiguration configuration(yml);

    ASSERT_EQ(configuration.dds_configuration->content_topic_filter_dict.at("duplicated_topic"), "2");
}

/**
 * Check that loading the DDS Recorder configuration from a malformed (unparseable) YAML file throws
 * a ConfigurationException wrapping the underlying parse error.
 */
TEST(YamlReaderDdsRecorderReplayerTest, recorder_malformed_file_throws)
{
    const std::string file_path = write_temp_yaml(
        "recorder_malformed_config.yaml",
        "recorder:\n  output: [unclosed\n");

    ASSERT_THROW(RecorderConfiguration configuration(file_path), utils::ConfigurationException);
}

/**
 * Check RecorderConfiguration::is_valid() defensive checks: a null dds_configuration, and a
 * DDS/remote-controller domain above the maximum allowed value (both only reachable by mutating
 * the configuration directly, since normal construction paths already prevent these states).
 */
TEST(YamlReaderDdsRecorderReplayerTest, recorder_is_valid_defensive_checks)
{
    Yaml yml = YAML::Load("recorder:\n  sql:\n    enable: true\n");
    RecorderConfiguration configuration(yml);

    utils::Formatter error_msg_null;
    configuration.dds_configuration = nullptr;
    ASSERT_FALSE(configuration.is_valid(error_msg_null));

    RecorderConfiguration configuration_domain(yml);
    utils::Formatter error_msg_domain;
    configuration_domain.dds_configuration->domain.domain_id =
            ddspipe::core::types::DomainId::MAX_DOMAIN_ID + 1;
    ASSERT_FALSE(configuration_domain.is_valid(error_msg_domain));

    RecorderConfiguration configuration_controller_domain(yml);
    utils::Formatter error_msg_controller_domain;
    configuration_controller_domain.enable_remote_controller = true;
    configuration_controller_domain.controller_domain.domain_id =
            ddspipe::core::types::DomainId::MAX_DOMAIN_ID + 1;
    ASSERT_FALSE(configuration_controller_domain.is_valid(error_msg_controller_domain));
}

/////////////////////////
// REPLAYER
/////////////////////////

/**
 * Check that a Domain ID configured through the Command-Line overrides the one set in the YAML.
 */
TEST(YamlReaderDdsRecorderReplayerTest, replayer_domain_cli_overrides_yaml)
{
    const char* yml_str =
            R"(
            dds:
              domain: 5
            replayer:
              input-file: "session.mcap"
        )";

    Yaml yml = YAML::Load(yml_str);

    CommandlineArgsReplayer commandline_args;
    commandline_args.domain.set_value(ddspipe::core::types::DomainId(10u));

    ReplayerConfiguration configuration(yml, &commandline_args);

    ASSERT_EQ(configuration.replayer_configuration->domain, ddspipe::core::types::DomainId(10u));
}

/**
 * Check that a 'begin-time' later than or equal to 'end-time' is rejected while loading the YAML.
 */
TEST(YamlReaderDdsRecorderReplayerTest, replayer_begin_time_after_end_time_throws)
{
    const char* yml_str =
            R"(
            replayer:
              input-file: "session.mcap"
              begin-time:
                datetime: 2024-01-01_01-00-00
              end-time:
                datetime: 2024-01-01_00-00-00
        )";

    Yaml yml = YAML::Load(yml_str);

    ASSERT_THROW(ReplayerConfiguration configuration(yml), utils::ConfigurationException);
}

/**
 * Check that loading the DDS Replayer configuration from a malformed (unparseable) YAML file throws
 * a ConfigurationException wrapping the underlying parse error.
 */
TEST(YamlReaderDdsRecorderReplayerTest, replayer_malformed_file_throws)
{
    const std::string file_path = write_temp_yaml(
        "replayer_malformed_config.yaml",
        "replayer:\n  input-file: [unclosed\n");

    ASSERT_THROW(ReplayerConfiguration configuration(file_path), utils::ConfigurationException);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
