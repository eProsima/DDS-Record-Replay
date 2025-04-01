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

#include <string>

struct ResourceLimits {
    uint64_t max_size = 35 * 1024;        // Default max size
    uint64_t max_file_size = 7 * 1024;    // Default max file size
    uint64_t size_tolerance = 2 * 1024;   // Default size tolerance
    bool log_rotation = true;             // Default log rotation
};

struct RecorderConfig {
    struct OutputConfig {
        uint64_t safety_margin = 350 * 1024; // Default safety margin
    } output;

    struct McapConfig {
        bool enable = false;                 // Default disabled
        ResourceLimits resource_limits = {}; // Default initialized ResourceLimits
    } mcap;

    struct SqlConfig {
        bool enable = false;                 // Default disabled
        ResourceLimits resource_limits = {}; // Default initialized ResourceLimits
    } sql;
};

void resource_limits_builder(
    const ResourceLimits &resource_limits, 
    std::string &yml_str)
    {
        if(resource_limits.max_size > 0)
            yml_str += 
            "      max-size: \"" + std::to_string(resource_limits.max_size) + "KB\"\n";
        if(resource_limits.max_file_size > 0)
            yml_str += 
            "      max-file-size: \"" + std::to_string(resource_limits.max_file_size) + "KB\"\n";
        if(resource_limits.size_tolerance > 0)
            yml_str += 
            "      size-tolerance: \"" + std::to_string(resource_limits.size_tolerance) + "KB\"\n";
        if(resource_limits.log_rotation)
            yml_str += 
            "      log-rotation: true\n";
    }


std::unique_ptr<RecorderConfiguration> config_builder(
        RecorderConfig config)
    {

        std::string yml_str = 
            "dds:\n"
            "  domain: 1\n"
            "recorder:\n"
            "  output:\n";
            // "    filename: test\n"
            // "    path: \"" + std::filesystem::current_path().string() + "\"\n";
        
        if(config.output.safety_margin > 0)
            yml_str += 
            "    safety-margin: \"" + std::to_string(config.output.safety_margin) + "B\"\n";

        if(config.mcap.enable)
        {
            yml_str += 
            "  mcap:\n"
            "    enable: true\n"
            "    resource-limits:\n";
            resource_limits_builder(config.mcap.resource_limits, yml_str);            
        }
        else
        {
            yml_str += 
            "  mcap:\n"
            "    enable: false\n";
        }

        if(config.sql.enable)
        {
            yml_str += 
            "  sql:\n"
            "    enable: true\n"
            "    resource-limits:\n";
            resource_limits_builder(config.sql.resource_limits, yml_str);
        }
        else
        {
            yml_str += 
            "  sql:\n"
            "    enable: false\n";
        }

        std::cout << yml_str << std::endl;

        Yaml yml = YAML::Load(yml_str);

        CommandlineArgsRecorder commandline_args;

        // Setting CommandLine arguments as if configured from CommandLine
        commandline_args.log_filter[eprosima::utils::VerbosityKind::Warning].set_value("DDSRECORDER|DDSPIPE|DEBUG");

        // Load configuration from YAML
        return std::make_unique<RecorderConfiguration>(yml, &commandline_args);
    }

/**
 * Check RecorderConfiguration structure creation.
 *
 * CASE
 * Neither SQL nor MCAP enabled
 */
TEST(YamlResourceLimitsTest, none)
{
    RecorderConfig config;
    config.sql.enable = false;
    config.mcap.enable = false;

    
    std::unique_ptr<RecorderConfiguration> configuration = config_builder(config);

    utils::Formatter error_msg;

    ASSERT_FALSE(configuration->is_valid(error_msg));
    ASSERT_GE(std::string(error_msg).size(), 0);
}

/**
 * Check RecorderConfiguration structure creation.
 *
 * CASE
 * - Full correct configuration
 */
TEST(YamlResourceLimitsTest, full_config)
{
    RecorderConfig config;
    config.sql.enable = true;
    config.mcap.enable = true;
    config.sql.resource_limits.max_file_size = 0;

    
    std::unique_ptr<RecorderConfiguration> configuration = config_builder(config);

    utils::Formatter error_msg;

    ASSERT_TRUE(configuration->is_valid(error_msg));
    ASSERT_EQ(std::string(error_msg).size(), 0);
}

/**
 * Check RecorderConfiguration SQL structure creation.
 *
 * CASE
 * A Setting max_file_size to a different value than max_size leads to an invalid configuration
 * B Just setting max_size_file leads to a valid configuration as max_size will be set to the same value in configuration
 * C Setting max_file_size to the same value as max_size leads to a valid configuration with a warning
 */
TEST(YamlResourceLimitsTest, sql_max_file_size)
{
    // A
    RecorderConfig config;
    config.sql.enable = true;
    config.sql.resource_limits.max_file_size = 7 * 1024;

    
    std::unique_ptr<RecorderConfiguration> configuration = config_builder(config);

    utils::Formatter error_msg_A;

    ASSERT_FALSE(configuration->is_valid(error_msg_A));
    ASSERT_GE(std::string(error_msg_A).size(), 0);
    
    // B
    config.sql.resource_limits.max_size = 0;
    
    configuration = config_builder(config);

    utils::Formatter error_msg_B;

    ASSERT_TRUE(configuration->is_valid(error_msg_B));
    ASSERT_EQ(std::string(error_msg_B).size(), 0);

    // C
    config.sql.resource_limits.max_size = config.sql.resource_limits.max_file_size;
    
    configuration = config_builder(config);

    utils::Formatter error_msg_C;

    ASSERT_TRUE(configuration->is_valid(error_msg_C));
    ASSERT_EQ(std::string(error_msg_C).size(), 0);
}

/**
 * Check RecorderConfiguration SQL structure creation.
 *
 * CASE
 * Setting log rotation to true withouth setting max_file_size or max_size leads to an invalid configuration
 */
TEST(YamlResourceLimitsTest, sql_log_rotation)
{
    RecorderConfig config;
    config.sql.enable = true;
    config.sql.resource_limits.max_size = 0;
    config.sql.resource_limits.max_file_size = 0;
    config.sql.resource_limits.log_rotation = true;

    
    std::unique_ptr<RecorderConfiguration> configuration = config_builder(config);

    utils::Formatter error_msg;

    ASSERT_FALSE(configuration->is_valid(error_msg));
    ASSERT_GE(std::string(error_msg).size(), 0);
}

/**
 * Check RecorderConfiguration MCAP structure creation.
 *
 * CASE
 * A Setting max_file_size to a greater value than max_size leads to an invalid configuration
 */
TEST(YamlResourceLimitsTest, mcap_max_size)
{
    // A
    RecorderConfig config;
    config.sql.enable = true;
    config.sql.resource_limits.max_file_size = 7 * 1024;

    
    std::unique_ptr<RecorderConfiguration> configuration = config_builder(config);

    utils::Formatter error_msg_A;

    ASSERT_FALSE(configuration->is_valid(error_msg_A));
    ASSERT_GE(std::string(error_msg_A).size(), 0);
}

/**
 * Check RecorderConfiguration MCAP structure creation.
 *
 * CASE
 * A Setting file rotation to true withouth setting max_file_size leads to an invalid configuration
 * B Setting file rotation to true with neither max_size not safety_margin set leads to an invalid configuration
 */
TEST(YamlResourceLimitsTest, mcap_file_rotation)
{
    // A
    RecorderConfig config;
    config.mcap.enable = true;
    config.mcap.resource_limits.max_size = 0;
    config.mcap.resource_limits.max_file_size = 0;
    config.mcap.resource_limits.log_rotation = true;

    
    std::unique_ptr<RecorderConfiguration> configuration = config_builder(config);

    utils::Formatter error_msg_A;

    ASSERT_FALSE(configuration->is_valid(error_msg_A));
    ASSERT_GE(std::string(error_msg_A).size(), 0);

    // B
    config.mcap.resource_limits.max_file_size = 30 * 1024;
    config.mcap.resource_limits.max_size = 0;
    config.output.safety_margin = 0;

    configuration = config_builder(config);

    utils::Formatter error_msg;

    ASSERT_FALSE(configuration->is_valid(error_msg));
    ASSERT_GE(std::string(error_msg).size(), 0);
}



int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}