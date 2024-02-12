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

#include <cpp_utils/logging/LogConfiguration.hpp>

#include <ddspipe_yaml/YamlReader.hpp>

#include <ddsrecorder_yaml/recorder/CommandlineArgsRecorder.hpp>
#include <ddsrecorder_yaml/recorder/YamlReaderConfiguration.hpp>

using namespace eprosima;
// using namespace eprosima::ddsrecorder;
using namespace eprosima::ddsrecorder::yaml;

/**
 * Check the get function for LogConfiguration.
 *
 * CASES:
 *  Check if logging shows a valid configuration.
 */
TEST(ParseYamlTest, is_valid_log_config)
{
    const char* yml_str =
            R"(
            specs:
              logging:
                verbosity: warning
                filter:
                  error: ""
                  warning: "DDSPIPE"
                  info: "DEBUG"
        )";

    Yaml yml = YAML::Load(yml_str);

    utils::LogConfiguration conf = ddspipe::yaml::YamlReader::get<utils::LogConfiguration>(yml,  ddspipe::yaml::YamlReaderVersion::LATEST);

    utils::Formatter error_msg;
    ASSERT_TRUE(conf.is_valid(error_msg));
}

/**
 * Check the get function for LogConfiguration from a yaml.
 *
 * CASES:
 *  Check if logging shows a valid configuration (verbosity and filter).
 */
TEST(ParseYamlTest, parse_yaml_log_config)
{
    const char* yml_str =
            R"(
            verbosity: warning
            filter:
              error: ""
              warning: "DDSPIPE"
              info: "DEBUG"
        )";

    Yaml yml = YAML::Load(yml_str);

    utils::LogConfiguration conf = ddspipe::yaml::YamlReader::get<utils::LogConfiguration>(yml, ddspipe::yaml::YamlReaderVersion::LATEST);

    ASSERT_TRUE(conf.verbosity.get_value() == utils::VerbosityKind::Warning);
    ASSERT_TRUE(conf.filter[utils::VerbosityKind::Error].get_value() == "");
    ASSERT_TRUE(conf.filter[utils::VerbosityKind::Warning].get_value() == "DDSPIPE");
    ASSERT_TRUE(conf.filter[utils::VerbosityKind::Info].get_value() == "DEBUG");
}

/**
 * Check the RecorderConfiguration function.
 *
 * CASES:
 *  Check if chooses correctly log configuration when parsing from terminal and from YAML.
 */
TEST(ParseYamlTest, parse_correct_log_config_1)
{
    CommandlineArgsRecorder commandline_args;
    // Debug options
    commandline_args.log_filter[eprosima::utils::VerbosityKind::Error].set_value("");
    commandline_args.log_filter[eprosima::utils::VerbosityKind::Warning].set_value("DDSRECORDER|DDSPIPE");
    commandline_args.log_filter[eprosima::utils::VerbosityKind::Info].set_value("DDSRECORDER");

    const char* yml_str =
            R"(
            specs:
              logging:
                verbosity: info
                filter:
                  error: "DDSPIPE"
                  warning: "DDSRECORDER"
                  info: "DEBUG"
        )";

    Yaml yml = YAML::Load(yml_str);

    // Load configuration from YAML
    RecorderConfiguration configuration(yml, &commandline_args);

    ASSERT_TRUE(configuration.ddspipe_configuration.log_configuration.verbosity.get_value() == utils::VerbosityKind::Info);
    ASSERT_TRUE(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Error].get_value() == "DDSPIPE");
    ASSERT_TRUE(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Warning].get_value() == "DDSRECORDER");
    ASSERT_TRUE(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Info].get_value() == "DEBUG");
}

TEST(ParseYamlTest, parse_correct_log_config_2)
{
    CommandlineArgsRecorder commandline_args;

    // Set verbosity as if it was set from commandline
    commandline_args.log_verbosity = utils::VerbosityKind::Error;
    // Debug options
    commandline_args.log_filter[eprosima::utils::VerbosityKind::Error].set_value("");
    commandline_args.log_filter[eprosima::utils::VerbosityKind::Warning].set_value("DDSRECORDER|DDSPIPE");
    commandline_args.log_filter[eprosima::utils::VerbosityKind::Info].set_value("DDSRECORDER");

    const char* yml_str =
            R"(
            specs:
              logging:
                verbosity: info
                filter:
                  info: "DEBUG"
        )";

    Yaml yml = YAML::Load(yml_str);

    // Load configuration from YAML
    RecorderConfiguration configuration(yml, &commandline_args);

    ASSERT_TRUE(configuration.ddspipe_configuration.log_configuration.verbosity.get_value() == utils::VerbosityKind::Info);
    ASSERT_TRUE(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Error].get_value() == "");
    ASSERT_TRUE(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Warning].get_value() == "DDSRECORDER|DDSPIPE");
    ASSERT_TRUE(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Info].get_value() == "DEBUG");
}

TEST(ParseYamlTest, parse_correct_log_config_3)
{
    CommandlineArgsRecorder commandline_args;

    // Set verbosity as if it was set from commandline
    commandline_args.log_verbosity = utils::VerbosityKind::Error;
    // Debug options
    commandline_args.log_filter[eprosima::utils::VerbosityKind::Error].set_value("");
    commandline_args.log_filter[eprosima::utils::VerbosityKind::Warning].set_value("DDSRECORDER|DDSPIPE");
    commandline_args.log_filter[eprosima::utils::VerbosityKind::Info].set_value("DDSRECORDER");


    const char* yml_str =
            R"(
            specs:
              logging:
                filter:
                  info: "DEBUG"
        )";

    Yaml yml = YAML::Load(yml_str);

    // Load configuration from YAML
    RecorderConfiguration configuration(yml, &commandline_args);

    ASSERT_TRUE(configuration.ddspipe_configuration.log_configuration.verbosity.get_value() == utils::VerbosityKind::Error);
    ASSERT_TRUE(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Error].get_value() == "");
    ASSERT_TRUE(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Warning].get_value() == "DDSRECORDER|DDSPIPE");
    ASSERT_TRUE(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Info].get_value() == "DEBUG");
}


int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
