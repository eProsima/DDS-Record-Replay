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
 * Check RecorderConfiguration.
 *
 * CASES:
 *  Check if chooses correctly log configuration when parsing from terminal and from YAML.
 */
TEST(ParseYamlTest, parse_correct_log_config_yaml_vs_commandline)
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
                  warning: "DDSRECORDER"
                  info: "DDSRECORDER|DEBUG"
        )";

    Yaml yml = YAML::Load(yml_str);

    // Load configuration from YAML
    RecorderConfiguration configuration(yml, &commandline_args);

    ASSERT_EQ(configuration.ddspipe_configuration.log_configuration.verbosity.get_value(), utils::VerbosityKind::Info);
    ASSERT_EQ(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Error].get_value(), "");
    ASSERT_EQ(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Warning].get_value(), "DDSRECORDER|DDSPIPE|DEBUG");
    ASSERT_EQ(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Info].get_value(), "DDSRECORDER|DEBUG");
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
