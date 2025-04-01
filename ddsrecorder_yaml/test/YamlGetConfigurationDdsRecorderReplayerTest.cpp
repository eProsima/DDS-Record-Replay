// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <ddspipe_yaml/YamlReader.hpp>

#include <ddsrecorder_yaml/recorder/CommandlineArgsRecorder.hpp>
#include <ddsrecorder_yaml/recorder/YamlReaderConfiguration.hpp>
#include <ddsrecorder_yaml/replayer/CommandlineArgsReplayer.hpp>
#include <ddsrecorder_yaml/replayer/YamlReaderConfiguration.hpp>

using namespace eprosima;
using namespace eprosima::ddsrecorder::yaml;

/**
 * Check RecorderConfiguration structure creation.
 *
 * CASES:
 *  Check if chooses correctly log configuration when parsing from terminal and from YAML.
 *  In this case, it checks that:
 *    - The error filter is the one configured through the YAML
 *    - The warning filter is the one configured through the Command-Line
 *    - The info filter is the default (DDSRECORDER)
 */
TEST(YamlGetConfigurationDdsRecorderReplayerTest, get_ddsrecorder_configuration_yaml_vs_commandline)
{
    CommandlineArgsRecorder commandline_args;

    // Setting CommandLine arguments as if configured from CommandLine
    commandline_args.log_filter[eprosima::utils::VerbosityKind::Warning].set_value("DDSRECORDER|DDSPIPE|DEBUG");

    const char* yml_str =
            R"(
            specs:
              logging:
                verbosity: info
                filter:
                  error: "DEBUG"
                  warning: "DDSRECORDER"
        )";

    Yaml yml = YAML::Load(yml_str);

    // Load configuration from YAML
    RecorderConfiguration configuration(yml, &commandline_args);

    utils::Formatter error_msg;

    ASSERT_TRUE(configuration.ddspipe_configuration.log_configuration.is_valid(error_msg));
    ASSERT_EQ(configuration.ddspipe_configuration.log_configuration.verbosity.get_value(), utils::VerbosityKind::Info);
    ASSERT_EQ(
        configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Error].get_value(),
        "DEBUG");
    ASSERT_EQ(
        configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Warning].get_value(),
        "DDSRECORDER|DDSPIPE|DEBUG");
    ASSERT_EQ(
        configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Info].get_value(),
        "DDSRECORDER");
}

/**
 * Check ReplayerConfiguration structure creation.
 *
 * CASES:
 *  Check if chooses correctly log configuration when parsing from terminal and from YAML.
 *  In this case, it checks that:
 *    - The error filter is the one configured through the YAML
 *    - The warning filter is the one configured through the Command-Line
 *    - The info filter is the default (DDSREPLAYER)
 */
TEST(YamlGetConfigurationDdsRecorderReplayerTest, get_ddsreplayer_configuration_yaml_vs_commandline)
{
    CommandlineArgsReplayer commandline_args;

    // Setting CommandLine arguments as if configured from CommandLine
    commandline_args.log_filter[eprosima::utils::VerbosityKind::Warning].set_value("DDSREPLAYER|DDSPIPE|DEBUG");

    const char* yml_str =
            R"(
            specs:
              logging:
                verbosity: info
                filter:
                  error: "DEBUG"
                  warning: "DDSREPLAYER"
        )";

    Yaml yml = YAML::Load(yml_str);

    // Load configuration from YAML
    ReplayerConfiguration configuration(yml, &commandline_args);

    utils::Formatter error_msg;

    ASSERT_TRUE(configuration.ddspipe_configuration.log_configuration.is_valid(error_msg));
    ASSERT_EQ(configuration.ddspipe_configuration.log_configuration.verbosity.get_value(), utils::VerbosityKind::Info);
    ASSERT_EQ(
        configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Error].get_value(),
        "DEBUG");
    ASSERT_EQ(
        configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Warning].get_value(),
        "DDSREPLAYER|DDSPIPE|DEBUG");
    ASSERT_EQ(
        configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Info].get_value(),
        "DDSREPLAYER");
}

/**
 * Test load SimpleParticipant configurations from YAML node.
 */
TEST(YamlGetConfigurationDdsRecorderReplayerTest, get_simple_participant_configuration_from_yaml)
{
    // Load configuration
    RecorderConfiguration recorder_config("DdsConfiguration.yaml");
    ReplayerConfiguration replayer_config("DdsConfiguration.yaml");

    // Check that it is valid
    utils::Formatter error_msg;
    ASSERT_TRUE(recorder_config.simple_configuration->is_valid(error_msg));
    ASSERT_TRUE(replayer_config.replayer_configuration->is_valid(error_msg));

    ASSERT_EQ(recorder_config.simple_configuration->domain, 0);
    ASSERT_EQ(recorder_config.simple_configuration->whitelist, std::set<std::string>{"127.0.0.1"});
    ASSERT_EQ(recorder_config.simple_configuration->transport, ddspipe::core::types::TransportDescriptors::builtin);
    ASSERT_EQ(recorder_config.simple_configuration->easy_mode_ip, "2.2.2.2");
    ASSERT_EQ(recorder_config.simple_configuration->ignore_participant_flags,
            ddspipe::core::types::IgnoreParticipantFlags::no_filter);

    ASSERT_EQ(replayer_config.replayer_configuration->domain, 0);
    ASSERT_EQ(replayer_config.replayer_configuration->whitelist, std::set<std::string>{"127.0.0.1"});
    ASSERT_EQ(replayer_config.replayer_configuration->transport, ddspipe::core::types::TransportDescriptors::builtin);
    ASSERT_EQ(replayer_config.replayer_configuration->easy_mode_ip, "2.2.2.2");
    ASSERT_EQ(replayer_config.replayer_configuration->ignore_participant_flags,
            ddspipe::core::types::IgnoreParticipantFlags::no_filter);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
