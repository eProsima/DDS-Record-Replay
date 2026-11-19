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

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <cpp_utils/Log.hpp>
#include <cpp_utils/testing/LogChecker.hpp>

#include <ddspipe_yaml/Yaml.hpp>

#include <ddsrecorder_yaml/recorder/YamlReaderConfiguration.hpp>
#include <ddsrecorder_yaml/replayer/YamlReaderConfiguration.hpp>

#include <tool/DdsRecorder.hpp>
#include <tool/DdsReplayer.hpp>

#include "../../resources/constants.hpp"
#include "../../resources/dds/DataToCheck.hpp"
#include "../../resources/dds/TestDynTypesSubscriber.h"
#include "../../resources/dds/TestSubscriber.h"
#include "../../resources/types/configuration/ConfigurationPubSubTypes.hpp"

using namespace eprosima;

namespace test {

// The recordings use their own domains, so they cannot reach each other nor the replays
constexpr std::uint32_t RECORD_WITHOUT_IDL_TYPES_DOMAIN = 81;
constexpr std::uint32_t RECORD_WITH_IDL_TYPES_DOMAIN = 82;

constexpr unsigned int RECORD_REPLAY_MESSAGES = 10;

// Written by the recording tests and read by the replay tests, which CTest runs afterwards in other processes
const std::string RECORDING_WITHOUT_IDL_TYPES = "record_replay_without_idl_types";
const std::string RECORDING_WITH_IDL_TYPES = "record_replay_with_idl_types";

const std::string REPLAYER_CONFIGURATION = "../../resources/config/type/config_file.yaml";
const std::string CONFIGURATION_IDL = "../../resources/types/configuration/Configuration.idl";
const std::string MISSING_IDL = "../../resources/types/configuration/Missing.idl";

} // namespace test

/**
 * Records data whose type is not shared, and replays the recordings.
 *
 * The recordings and the replays are different tests, so CTest runs them in different processes. Registering the
 * type to publish fills the type registry of the process, so a replay in the same process could find the type
 * without loading it from IDL.
 */
class RecordReplayTest : public testing::Test
{
protected:

    static std::string recording_path_(
            const std::string& file_name)
    {
        return (std::filesystem::current_path() / (file_name + ".db")).string();
    }

    //! Publish the messages without sharing their type, while recording them
    void record_(
            const std::string& file_name,
            const std::uint32_t domain,
            const std::vector<std::string>& idl_types_paths)
    {
        const auto file_path = recording_path_(file_name);
        std::filesystem::remove(file_path);

        Yaml yml;
        ddsrecorder::yaml::RecorderConfiguration configuration(yml);
        configuration.dds_configuration->domain = domain;
        configuration.dds_configuration->allowed_partition_list.insert("*");
        configuration.dds_configuration->idl_types_paths = {idl_types_paths.begin(), idl_types_paths.end()};
        configuration.sql_enabled = true;
        configuration.mcap_enabled = false;
        configuration.dds_enabled = false;

        auto recorder = std::make_unique<ddsrecorder::recorder::DdsRecorder>(
            configuration, ddsrecorder::recorder::DdsRecorderStateCode::RUNNING, file_name);

        publish_(domain);

        // Destroying the recorder writes the file
        recorder.reset();

        // The logs are consumed asynchronously
        utils::Log::Flush();

        ASSERT_TRUE(std::filesystem::exists(file_path));
    }

    //! Replay a recording, receiving the messages with a subscriber that needs the type to be announced
    DataToCheck replay_(
            const std::string& file_name,
            const std::vector<std::string>& idl_types_paths,
            const bool type_announced = true)
    {
        DataToCheck data;

        const auto file_path = recording_path_(file_name);

        if (!std::filesystem::exists(file_path))
        {
            ADD_FAILURE() << "Recording " << file_path << " not found, the test recording it must run first.";
            return data;
        }

        std::unique_ptr<fastdds::dds::DomainParticipantListener> subscriber;

        if (type_announced)
        {
            subscriber = std::make_unique<TestDynTypesSubscriber>(
                test::DDS_TOPIC_NAME, test::DDS_TYPE_NAME, false, test::DOMAIN, data);
        }
        else
        {
            subscriber = std::make_unique<TestSubscriber>(test::DDS_TOPIC_NAME, test::DDS_TYPE_NAME, test::DOMAIN,
                            data);
        }

        ddsrecorder::yaml::ReplayerConfiguration configuration(
            (std::filesystem::current_path() / test::REPLAYER_CONFIGURATION).string());
        configuration.replayer_configuration->domain.domain_id = test::DOMAIN;
        configuration.replayer_configuration->idl_types_paths = {idl_types_paths.begin(), idl_types_paths.end()};

        auto replayer = std::make_unique<ddsrecorder::replayer::DdsReplayer>(configuration, file_path);

        // Give time for the replayer and the subscriber to match
        std::this_thread::sleep_for(std::chrono::seconds(1));

        replayer->process_file();
        replayer->stop();
        replayer.reset();

        // The logs are consumed asynchronously
        utils::Log::Flush();

        return data;
    }

    void publish_(
            const std::uint32_t domain)
    {
        fastdds::dds::DomainParticipantQos pqos;
        pqos.properties().properties().emplace_back("fastdds.type_propagation", "disabled");

        auto factory = fastdds::dds::DomainParticipantFactory::get_instance();
        auto participant = factory->create_participant(domain, pqos);
        ASSERT_NE(participant, nullptr);

        fastdds::dds::TypeSupport type(new ConfigurationPubSubType());
        participant->register_type(type);

        auto topic = participant->create_topic(test::DDS_TOPIC_NAME, test::DDS_TYPE_NAME,
                        fastdds::dds::TOPIC_QOS_DEFAULT);
        auto publisher = participant->create_publisher(fastdds::dds::PUBLISHER_QOS_DEFAULT);

        fastdds::dds::DataWriterQos wqos = fastdds::dds::DATAWRITER_QOS_DEFAULT;
        wqos.reliability().kind = fastdds::dds::RELIABLE_RELIABILITY_QOS;
        wqos.durability().kind = fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
        wqos.history().kind = fastdds::dds::KEEP_ALL_HISTORY_QOS;

        auto writer = publisher->create_datawriter(topic, wqos);
        ASSERT_NE(writer, nullptr);

        // Wait for the recorder to match the writer
        fastdds::dds::PublicationMatchedStatus status;
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);

        do
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            writer->get_publication_matched_status(status);
        }
        while (status.current_count == 0 && std::chrono::steady_clock::now() < deadline);

        ASSERT_GT(status.current_count, 0);

        for (std::uint32_t index = 1; index <= test::RECORD_REPLAY_MESSAGES; index++)
        {
            Configuration sample;
            sample.index(index);
            writer->write(&sample);

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        EXPECT_EQ(writer->wait_for_acknowledgments(fastdds::dds::Duration_t(10)), fastdds::dds::RETCODE_OK);

        // Let the recorder take the acknowledged samples before the writer disappears
        std::this_thread::sleep_for(std::chrono::seconds(1));

        participant->delete_contained_entities();
        factory->delete_participant(participant);
    }

    static void check_all_received_(
            const DataToCheck& data)
    {
        ASSERT_EQ(data.n_received_msgs, test::RECORD_REPLAY_MESSAGES);
        ASSERT_EQ(data.min_index_msg, 1);
        ASSERT_EQ(data.max_index_msg, static_cast<int>(test::RECORD_REPLAY_MESSAGES));
    }

};

/**
 * Record data whose type is not shared, without IDL files, so the recording does not have its type.
 */
TEST_F(RecordReplayTest, record_without_idl_types)
{
    utils::testing::LogChecker log_checker(utils::Log::Kind::Error, 0, 0);

    record_(test::RECORDING_WITHOUT_IDL_TYPES, test::RECORD_WITHOUT_IDL_TYPES_DOMAIN, {});

    ASSERT_TRUE(log_checker.check_valid());
}

/**
 * Record data whose type is not shared, with an IDL file, so the recording has the type loaded from it.
 */
TEST_F(RecordReplayTest, record_with_idl_types)
{
    utils::testing::LogChecker log_checker(utils::Log::Kind::Error, 0, 0);

    record_(test::RECORDING_WITH_IDL_TYPES, test::RECORD_WITH_IDL_TYPES_DOMAIN, {test::CONFIGURATION_IDL});

    ASSERT_TRUE(log_checker.check_valid());
}

/**
 * Without IDL files the replayer has no type to announce for a recording that does not have it.
 */
TEST_F(RecordReplayTest, replay_no_idl_types)
{
    const auto data = replay_(test::RECORDING_WITHOUT_IDL_TYPES, {});

    ASSERT_EQ(data.n_received_msgs, 0u);
}

/**
 * The replayer announces the type loaded from IDL for a recording that does not have it.
 */
TEST_F(RecordReplayTest, replay_idl_types)
{
    utils::testing::LogChecker log_checker(utils::Log::Kind::Error, 0, 0);

    const auto data = replay_(test::RECORDING_WITHOUT_IDL_TYPES, {test::CONFIGURATION_IDL});

    ASSERT_TRUE(log_checker.check_valid());
    check_all_received_(data);
}

/**
 * The type loaded from IDL by the recorder is stored, and announced by the replayer without IDL files.
 */
TEST_F(RecordReplayTest, replay_types_recorded_from_idl)
{
    utils::testing::LogChecker log_checker(utils::Log::Kind::Error, 0, 0);

    const auto data = replay_(test::RECORDING_WITH_IDL_TYPES, {});

    ASSERT_TRUE(log_checker.check_valid());
    check_all_received_(data);
}

/**
 * A file that does not exist is reported once, and the replayer still replays the data.
 */
TEST_F(RecordReplayTest, replay_missing_idl_file)
{
    utils::testing::LogChecker log_checker(utils::Log::Kind::Error, 1, 1);

    const auto data = replay_(test::RECORDING_WITHOUT_IDL_TYPES, {test::MISSING_IDL}, false);

    ASSERT_TRUE(log_checker.check_valid());
    check_all_received_(data);
}

/**
 * A file that does not exist does not stop the replayer from announcing the type of the other files.
 */
TEST_F(RecordReplayTest, replay_missing_and_valid_idl_files)
{
    utils::testing::LogChecker log_checker(utils::Log::Kind::Error, 1, 1);

    const auto data = replay_(test::RECORDING_WITHOUT_IDL_TYPES, {test::MISSING_IDL, test::CONFIGURATION_IDL});

    ASSERT_TRUE(log_checker.check_valid());
    check_all_received_(data);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
