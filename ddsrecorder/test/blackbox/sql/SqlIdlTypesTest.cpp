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

#include <cstdint>
#include <string>
#include <vector>

#include <sqlite/sqlite3.h>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/Log.hpp>
#include <cpp_utils/testing/LogChecker.hpp>

#include "../constants.hpp"
#include "../FileCreationTest.hpp"

using namespace eprosima;

namespace test {

const std::string HELLO_WORLD_IDL = "../../resources/types/hello_world/HelloWorld.idl";
const std::string MALFORMED_IDL = "../../resources/types/idl/Malformed.idl";
const std::string UNRELATED_IDL = "../../resources/types/idl/Unrelated.idl";
const std::string MISSING_IDL = "../../resources/types/idl/Missing.idl";

constexpr unsigned int IDL_TYPES_MESSAGES = 10;

// The number of errors a parsing failure logs depends on the IDL parser
constexpr unsigned int MAX_PARSING_ERRORS = 100;

} // namespace test

/**
 * Records with a writer that does not share its type, so the type can only come from the IDL files.
 */
class SqlIdlTypesTest : public FileCreationTest
{
public:

    void SetUp() override
    {
        FileCreationTest::SetUp();

        // Set the output library to SQL
        configuration_->sql_enabled = true;
        configuration_->dds_enabled = false;
        configuration_->mcap_enabled = false;
    }

protected:

    void configure_participant_qos_(
            fastdds::dds::DomainParticipantQos& pqos) override
    {
        pqos.properties().properties().emplace_back("fastdds.type_propagation", "disabled");
    }

    void record_(
            const std::string& file_name,
            const std::vector<std::string>& idl_types_paths)
    {
        configuration_->dds_configuration->idl_types_paths = {idl_types_paths.begin(), idl_types_paths.end()};

        record_messages_(file_name, test::IDL_TYPES_MESSAGES);

        // The logs are consumed asynchronously
        utils::Log::Flush();
    }

    std::uint64_t count_rows_(
            const std::string& file_path,
            const std::string& statement)
    {
        sqlite3* database = nullptr;
        std::uint64_t count = 0;

        if (sqlite3_open(file_path.c_str(), &database) == SQLITE_OK)
        {
            sqlite3_stmt* stmt = nullptr;

            if (sqlite3_prepare_v2(database, statement.c_str(), -1, &stmt, nullptr) == SQLITE_OK &&
                    sqlite3_step(stmt) == SQLITE_ROW)
            {
                count = static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0));
            }

            sqlite3_finalize(stmt);
        }

        sqlite3_close(database);

        return count;
    }

    std::uint64_t count_messages_(
            const std::string& file_path)
    {
        return count_rows_(file_path, "SELECT COUNT(*) FROM Messages;");
    }

    std::uint64_t count_hello_world_types_(
            const std::string& file_path)
    {
        return count_rows_(file_path, "SELECT COUNT(*) FROM Types WHERE name = 'HelloWorld';");
    }

};

/**
 * Records with a writer that shares its type.
 */
class SqlIdlTypesTestDiscovery : public SqlIdlTypesTest
{
protected:

    void configure_participant_qos_(
            fastdds::dds::DomainParticipantQos& /*pqos*/) override
    {
    }

};

/**
 * Without IDL files the type of a writer that does not share it is not recorded.
 */
TEST_F(SqlIdlTypesTest, no_idl_types)
{
    const std::string OUTPUT_FILE_NAME = "sql_no_idl_types";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    utils::testing::LogChecker log_checker(utils::Log::Kind::Error, 0, 0);

    record_(OUTPUT_FILE_NAME, {});

    ASSERT_TRUE(log_checker.check_valid());
    ASSERT_GT(count_messages_(OUTPUT_FILE_PATH), 0u);
    ASSERT_EQ(count_hello_world_types_(OUTPUT_FILE_PATH), 0u);
}

/**
 * The type of a writer that does not share it is resolved with the IDL file.
 */
TEST_F(SqlIdlTypesTest, idl_types_resolve_type)
{
    const std::string OUTPUT_FILE_NAME = "sql_idl_types_resolve_type";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    utils::testing::LogChecker log_checker(utils::Log::Kind::Error, 0, 0);

    record_(OUTPUT_FILE_NAME, {test::HELLO_WORLD_IDL});

    ASSERT_TRUE(log_checker.check_valid());
    ASSERT_GT(count_messages_(OUTPUT_FILE_PATH), 0u);
    ASSERT_EQ(count_hello_world_types_(OUTPUT_FILE_PATH), 1u);
}

/**
 * A file that does not exist is reported once, and the data is still recorded.
 */
TEST_F(SqlIdlTypesTest, idl_types_missing_file)
{
    const std::string OUTPUT_FILE_NAME = "sql_idl_types_missing_file";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    utils::testing::LogChecker log_checker(utils::Log::Kind::Error, 1, 1);

    record_(OUTPUT_FILE_NAME, {test::MISSING_IDL});

    ASSERT_TRUE(log_checker.check_valid());
    ASSERT_GT(count_messages_(OUTPUT_FILE_PATH), 0u);
    ASSERT_EQ(count_hello_world_types_(OUTPUT_FILE_PATH), 0u);
}

/**
 * An empty path is reported once, and the data is still recorded.
 */
TEST_F(SqlIdlTypesTest, idl_types_empty_path)
{
    const std::string OUTPUT_FILE_NAME = "sql_idl_types_empty_path";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    utils::testing::LogChecker log_checker(utils::Log::Kind::Error, 1, 1);

    record_(OUTPUT_FILE_NAME, {""});

    ASSERT_TRUE(log_checker.check_valid());
    ASSERT_GT(count_messages_(OUTPUT_FILE_PATH), 0u);
    ASSERT_EQ(count_hello_world_types_(OUTPUT_FILE_PATH), 0u);
}

/**
 * A file that does not parse is reported, and the data is still recorded.
 */
TEST_F(SqlIdlTypesTest, idl_types_malformed_file)
{
    const std::string OUTPUT_FILE_NAME = "sql_idl_types_malformed_file";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    utils::testing::LogChecker log_checker(utils::Log::Kind::Error, 1, test::MAX_PARSING_ERRORS);

    record_(OUTPUT_FILE_NAME, {test::MALFORMED_IDL});

    ASSERT_TRUE(log_checker.check_valid());
    ASSERT_GT(count_messages_(OUTPUT_FILE_PATH), 0u);
    ASSERT_EQ(count_hello_world_types_(OUTPUT_FILE_PATH), 0u);
}

/**
 * A valid file that does not declare the type of the writer loads without errors, but does not resolve it.
 */
TEST_F(SqlIdlTypesTest, idl_types_unrelated_file)
{
    const std::string OUTPUT_FILE_NAME = "sql_idl_types_unrelated_file";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    utils::testing::LogChecker log_checker(utils::Log::Kind::Error, 0, 0);

    record_(OUTPUT_FILE_NAME, {test::UNRELATED_IDL});

    ASSERT_TRUE(log_checker.check_valid());
    ASSERT_GT(count_messages_(OUTPUT_FILE_PATH), 0u);
    ASSERT_EQ(count_hello_world_types_(OUTPUT_FILE_PATH), 0u);
}

/**
 * A file that does not exist does not stop the other files from resolving the type.
 */
TEST_F(SqlIdlTypesTest, idl_types_missing_and_valid_files)
{
    const std::string OUTPUT_FILE_NAME = "sql_idl_types_missing_and_valid_files";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    utils::testing::LogChecker log_checker(utils::Log::Kind::Error, 1, 1);

    record_(OUTPUT_FILE_NAME, {test::MISSING_IDL, test::HELLO_WORLD_IDL});

    ASSERT_TRUE(log_checker.check_valid());
    ASSERT_GT(count_messages_(OUTPUT_FILE_PATH), 0u);
    ASSERT_EQ(count_hello_world_types_(OUTPUT_FILE_PATH), 1u);
}

/**
 * A file that does not parse does not stop the other files from resolving the type.
 */
TEST_F(SqlIdlTypesTest, idl_types_malformed_and_valid_files)
{
    const std::string OUTPUT_FILE_NAME = "sql_idl_types_malformed_and_valid_files";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    utils::testing::LogChecker log_checker(utils::Log::Kind::Error, 1, test::MAX_PARSING_ERRORS);

    record_(OUTPUT_FILE_NAME, {test::MALFORMED_IDL, test::HELLO_WORLD_IDL});

    ASSERT_TRUE(log_checker.check_valid());
    ASSERT_GT(count_messages_(OUTPUT_FILE_PATH), 0u);
    ASSERT_EQ(count_hello_world_types_(OUTPUT_FILE_PATH), 1u);
}

/**
 * Each path that does not exist is reported once, and the valid file still resolves the type.
 */
TEST_F(SqlIdlTypesTest, idl_types_several_invalid_paths)
{
    const std::string OUTPUT_FILE_NAME = "sql_idl_types_several_invalid_paths";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    utils::testing::LogChecker log_checker(utils::Log::Kind::Error, 2, 2);

    record_(OUTPUT_FILE_NAME, {test::MISSING_IDL, "", test::HELLO_WORLD_IDL});

    ASSERT_TRUE(log_checker.check_valid());
    ASSERT_GT(count_messages_(OUTPUT_FILE_PATH), 0u);
    ASSERT_EQ(count_hello_world_types_(OUTPUT_FILE_PATH), 1u);
}

/**
 * A file that does not parse does not prevent recording the type shared by the writer.
 */
TEST_F(SqlIdlTypesTestDiscovery, idl_types_malformed_file_keeps_discovered_type)
{
    const std::string OUTPUT_FILE_NAME = "sql_idl_types_malformed_file_keeps_discovered_type";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME + ".db");
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    utils::testing::LogChecker log_checker(utils::Log::Kind::Error, 1, test::MAX_PARSING_ERRORS);

    record_(OUTPUT_FILE_NAME, {test::MALFORMED_IDL});

    ASSERT_TRUE(log_checker.check_valid());
    ASSERT_GT(count_messages_(OUTPUT_FILE_PATH), 0u);
    ASSERT_EQ(count_hello_world_types_(OUTPUT_FILE_PATH), 1u);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
