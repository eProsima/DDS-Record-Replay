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

#include <filesystem>
#include <fstream>
#include <iostream>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/exception/ConfigurationException.hpp>

#include <ddspipe_yaml/YamlManager.hpp>
#include <ddspipe_yaml/YamlValidator.hpp>

#include <ddsrecorder_yaml/recorder/YamlReaderConfiguration.hpp>
#include <ddsrecorder_yaml/replayer/YamlReaderConfiguration.hpp>

using namespace eprosima;
using namespace eprosima::ddspipe::yaml;
using namespace eprosima::ddsrecorder::yaml;

namespace test {
// Paths and files for the tests
std::string recorder_schema_path = "./ddsrecorder_config_schema.json";
std::string replayer_schema_path = "./ddsreplayer_config_schema.json";

// Vectors with the valid and invalid YAML files for the recorder
std::vector<std::string> recorder_valid_files = []()
        {
            std::vector<std::string> files;
            for (const auto& entry : std::filesystem::directory_iterator("./valid_config_files_recorder/"))
            {
                if (entry.path().extension() == ".yaml")
                {
                    files.push_back(entry.path().generic_string());
                }
            }
            return files;
        }();
std::vector<std::string> recorder_invalid_files = []()
        {
            std::vector<std::string> files;
            for (const auto& entry : std::filesystem::directory_iterator("./invalid_config_files_recorder/"))
            {
                if (entry.path().extension() == ".yaml")
                {
                    files.push_back(entry.path().generic_string());
                }
            }
            return files;
        }();

// Vectors with the valid and invalid YAML files for the replayer
std::vector<std::string> replayer_valid_files = []()
        {
            std::vector<std::string> files;
            for (const auto& entry : std::filesystem::directory_iterator("./valid_config_files_replayer/"))
            {
                if (entry.path().extension() == ".yaml")
                {
                    files.push_back(entry.path().generic_string());
                }
            }
            return files;
        }();
std::vector<std::string> replayer_invalid_files = []()
        {
            std::vector<std::string> files;
            for (const auto& entry : std::filesystem::directory_iterator("./invalid_config_files_replayer/"))
            {
                if (entry.path().extension() == ".yaml")
                {
                    files.push_back(entry.path().generic_string());
                }
            }
            return files;
        }();
} // namespace test

/**
 * Test directly that a set of valid YAML configurations pass the validation in the recorder
 */
TEST(YamlValidatorDdsRecorderReplayerTest, recorder_direct_validation_passed)
{
    YamlValidator validator = YamlValidator(YamlValidator::InputType::FROM_FILE, test::recorder_schema_path);

    // valid files
    {
        for (std::string st : test::recorder_valid_files)
        {
            Yaml yml = YamlManager::load_file(st);
            ASSERT_TRUE(validator.validate_YAML(yml)) << "Failed for file: " << st;
        }
    }
}

/**
 * Test directly that a set of invalid YAML configurations don't pass the validation in the recorder
 */
TEST(YamlValidatorDdsRecorderReplayerTest, recorder_direct_validation_failed)
{
    YamlValidator validator = YamlValidator(YamlValidator::InputType::FROM_FILE, test::recorder_schema_path);

    // invalid files
    {
        for (std::string st : test::recorder_invalid_files)
        {
            Yaml yml = YamlManager::load_file(st);
            // Validate is called with false to prevent filling the output with the specific errors
            ASSERT_FALSE(validator.validate_YAML(yml, false)) << "Failed for file: " << st;
        }
    }
}

/**
 * Test directly that a set of valid YAML configurations pass the validation in the replayer
 */
TEST(YamlValidatorDdsRecorderReplayerTest, replayer_direct_validation_passed)
{
    YamlValidator validator = YamlValidator(YamlValidator::InputType::FROM_FILE, test::replayer_schema_path);

    // valid files
    {
        for (std::string st : test::replayer_valid_files)
        {
            Yaml yml = YamlManager::load_file(st);
            ASSERT_TRUE(validator.validate_YAML(yml)) << "Failed for file: " << st;
        }
    }
}

/**
 * Test directly that a set of invalid YAML configurations don't pass the validation in the replayer
 */
TEST(YamlValidatorDdsRecorderReplayerTest, replayer_direct_validation_failed)
{
    YamlValidator validator = YamlValidator(YamlValidator::InputType::FROM_FILE, test::replayer_schema_path);

    // invalid files
    {
        for (std::string st : test::replayer_invalid_files)
        {
            Yaml yml = YamlManager::load_file(st);
            // Validate is called with false to prevent filling the output with the specific errors
            ASSERT_FALSE(validator.validate_YAML(yml, false)) << "Failed for file: " << st;
        }
    }
}

/**
 * Test using the recorder YamlReader that a set of valid YAML configurations pass the validation
 */
TEST(YamlValidatorDdsRecorderReplayerTest, recorder_reader_validation_passed)
{
    // valid files
    {
        for (std::string st : test::recorder_valid_files)
        {
            ASSERT_NO_THROW(RecorderConfiguration recorder_config = RecorderConfiguration(st); )
                << "Failed for file: " << st;
        }
    }
}

/**
 * Test using the recorder YamlReader that a set of invalid YAML configurations don't pass the validation
 */
TEST(YamlValidatorDdsRecorderReplayerTest, recorder_reader_validation_failed)
{
    // invalid files
    {
        for (std::string st : test::recorder_invalid_files)
        {
            try
            {
                RecorderConfiguration recorder_config = RecorderConfiguration(st);
                FAIL() << "Expected eprosima::utils::ConfigurationException for file:\n'" << st << "'\n";
            }
            catch (const eprosima::utils::ConfigurationException& e)
            {
                EXPECT_NE(std::string(e.what()).find("is not a valid ddsrecorder configuration"), std::string::npos)
                    << "Failed for file\n'" << st << "'\nActual message: " << e.what();
            }
            catch (const std::exception& e)
            {
                FAIL() << "Expected eprosima::utils::ConfigurationException but "
                       << "caught a different exception for file:\n'"
                       << st << "'\n"
                       << "Actual message: " << e.what();
            }
        }
    }
}

/**
 * Test using the replayer YamlReader that a set of valid YAML configurations pass the validation
 */
TEST(YamlValidatorDdsRecorderReplayerTest, replayer_reader_validation_passed)
{
    // valid files
    {
        for (std::string st : test::replayer_valid_files)
        {
            ASSERT_NO_THROW(ReplayerConfiguration replayer_config = ReplayerConfiguration(st); )
                << "Failed for file: " << st;
        }
    }
}

/**
 * Test using the replayer YamlReader that a set of invalid YAML configurations don't pass the validation
 */
TEST(YamlValidatorDdsRecorderReplayerTest, replayer_reader_validation_failed)
{
    // invalid files
    {
        for (std::string st : test::replayer_invalid_files)
        {
            try
            {
                ReplayerConfiguration replayer_config = ReplayerConfiguration(st);
                FAIL() << "Expected eprosima::utils::ConfigurationException for file:\n'" << st << "'\n";
            }
            catch (const eprosima::utils::ConfigurationException& e)
            {
                EXPECT_NE(std::string(e.what()).find("is not a valid ddsreplayer configuration"), std::string::npos)
                    << "Failed for file\n'" << st << "'\nActual message: " << e.what();
            }
            catch (const std::exception& e)
            {
                FAIL() << "Expected eprosima::utils::ConfigurationException but "
                       << "caught a different exception for file:\n'"
                       << st << "'\n"
                       << "Actual message: " << e.what();
            }
        }
    }
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
