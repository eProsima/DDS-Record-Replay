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
#include <memory>
#include <string>
#include <vector>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <sqlite/sqlite3.h>

#include <ddsrecorder_yaml/replayer/YamlReaderConfiguration.hpp>

#include "tool/McapToSqlConverter.hpp"
#include "user_interface/arguments_configuration.hpp"

namespace {

std::vector<char*> argv_from_(
        std::vector<std::string>& args)
{
    std::vector<char*> argv;
    argv.reserve(args.size());

    for (auto& arg : args)
    {
        argv.push_back(arg.data());
    }

    return argv;
}

std::filesystem::path recordings_root_()
{
    return std::filesystem::path(DDSREPLAYER_RECORDINGS_DIR);
}

int count_query_(
        const std::filesystem::path& database_path,
        const std::string& query)
{
    sqlite3* database = nullptr;
    if (sqlite3_open(database_path.string().c_str(), &database) != SQLITE_OK)
    {
        ADD_FAILURE() << "Failed to open database: " << database_path;
        return -1;
    }

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        ADD_FAILURE() << "Failed to prepare query: " << query;
        sqlite3_close(database);
        return -1;
    }

    int result = 0;
    if (sqlite3_step(stmt) != SQLITE_ROW)
    {
        ADD_FAILURE() << "Failed to step query: " << query;
        sqlite3_finalize(stmt);
        sqlite3_close(database);
        return -1;
    }
    result = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    sqlite3_close(database);
    return result;
}

} // namespace

class McapConvertTest : public ::testing::Test
{
protected:

    void SetUp() override
    {
        output_directory_ = std::filesystem::temp_directory_path() / "mcap_convert_tool_tests";
        std::filesystem::create_directories(output_directory_);
    }

    void TearDown() override
    {
        std::error_code ec;
        std::filesystem::remove_all(output_directory_, ec);
    }

    eprosima::ddsrecorder::converter::CommandlineArgsMcapConvert commandline_args_;
    eprosima::ddsrecorder::yaml::ReplayerConfiguration configuration_{
        eprosima::Yaml(),
        &commandline_args_};
    std::filesystem::path output_directory_;
};

TEST_F(McapConvertTest, help_argument)
{
    std::vector<std::string> args = {"mcap-convert", "--help"};
    auto argv = argv_from_(args);
    auto commandline_args = eprosima::ddsrecorder::converter::CommandlineArgsMcapConvert();

    const auto ret = eprosima::ddsrecorder::converter::parse_arguments(
        static_cast<int>(argv.size()),
        argv.data(),
        commandline_args);

    ASSERT_EQ(ret, eprosima::ddsrecorder::converter::ProcessReturnCode::help_argument);
}

TEST_F(McapConvertTest, version_argument)
{
    std::vector<std::string> args = {"mcap-convert", "--version"};
    auto argv = argv_from_(args);
    auto commandline_args = eprosima::ddsrecorder::converter::CommandlineArgsMcapConvert();

    const auto ret = eprosima::ddsrecorder::converter::parse_arguments(
        static_cast<int>(argv.size()),
        argv.data(),
        commandline_args);

    ASSERT_EQ(ret, eprosima::ddsrecorder::converter::ProcessReturnCode::version_argument);
}

TEST_F(McapConvertTest, missing_input)
{
    std::vector<std::string> args = {"mcap-convert", "--sql-output", "output.db"};
    auto argv = argv_from_(args);
    auto commandline_args = eprosima::ddsrecorder::converter::CommandlineArgsMcapConvert();

    const auto ret = eprosima::ddsrecorder::converter::parse_arguments(
        static_cast<int>(argv.size()),
        argv.data(),
        commandline_args);

    ASSERT_EQ(ret, eprosima::ddsrecorder::converter::ProcessReturnCode::incorrect_argument);
}

TEST_F(McapConvertTest, default_output)
{
    const auto source_input_file = recordings_root_() / "basic" / "configuration.mcap";
    const auto input_file = output_directory_ / "configuration.mcap";
    const auto expected_output = output_directory_ / "configuration.db";
    std::filesystem::copy_file(source_input_file, input_file, std::filesystem::copy_options::overwrite_existing);

    commandline_args_.input_file = input_file.string();

    eprosima::ddsrecorder::converter::McapToSqlConverter converter(
        configuration_,
        commandline_args_.input_file);
    converter.convert();

    ASSERT_TRUE(std::filesystem::exists(expected_output));
    ASSERT_GT(count_query_(expected_output, "SELECT COUNT(*) FROM Messages;"), 0);
}

TEST_F(McapConvertTest, explicit_output)
{
    const auto input_file = recordings_root_() / "type" / "configuration_type.mcap";
    const auto output_file = output_directory_ / "typed_output.db";

    commandline_args_.input_file = input_file.string();

    eprosima::ddsrecorder::converter::McapToSqlConverter converter(
        configuration_,
        commandline_args_.input_file,
        output_file.string());
    converter.convert();

    ASSERT_TRUE(std::filesystem::exists(output_file));
    ASSERT_GT(count_query_(output_file, "SELECT COUNT(*) FROM Messages;"), 0);
    ASSERT_GT(count_query_(output_file,
            "SELECT COUNT(*) FROM Messages WHERE data_json IS NOT NULL AND data_json != '';"), 0);
    ASSERT_GT(count_query_(output_file, "SELECT COUNT(*) FROM Messages WHERE key != '';"), 0);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
