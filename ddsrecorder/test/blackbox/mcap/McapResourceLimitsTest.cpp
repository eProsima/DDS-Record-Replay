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

#include <cstdint>
#include <filesystem>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <ddspipe_yaml/Yaml.hpp>
#include <ddspipe_yaml/YamlReader.hpp>

#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>
#include <ddsrecorder_yaml/recorder/YamlReaderConfiguration.hpp>

#include <tool/DdsRecorder.hpp>

#if FASTRTPS_VERSION_MAJOR < 2 || (FASTRTPS_VERSION_MAJOR == 2 && FASTRTPS_VERSION_MINOR < 13)
    #include "../../resources/types/hello_world/v1/HelloWorld.h"
    #include "../../resources/types/hello_world/v1/HelloWorldPubSubTypes.h"
#else
    #include "../../resources/types/hello_world/v2/HelloWorld.h"
    #include "../../resources/types/hello_world/v2/HelloWorldPubSubTypes.h"
#endif // if FASTRTPS_VERSION_MAJOR < 2 || (FASTRTPS_VERSION_MAJOR == 2 && FASTRTPS_VERSION_MINOR < 13)

#include "../constants.hpp"

using namespace eprosima;
using namespace eprosima::fastdds::dds;


class McapResourceLimitsTest : public testing::Test
{
public:

    void SetUp() override
    {
        // Create the participant
        DomainParticipantQos pqos;
        pqos.name(test::PARTICIPANT_ID);

        participant_ = DomainParticipantFactory::get_instance()->create_participant(test::DOMAIN, pqos);

        ASSERT_NE(participant_, nullptr);

        // Register the type
        TypeSupport type(new HelloWorldPubSubType());
        type.register_type(participant_);

        // Create the topic
        topic_ = participant_->create_topic(test::TOPIC_NAME, type.get_type_name(), TOPIC_QOS_DEFAULT);

        ASSERT_NE(topic_, nullptr);

        // Create the publisher
        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

        ASSERT_NE(publisher_, nullptr);

        // Create the RecorderConfiguration
        Yaml yml;
        configuration_ = std::make_unique<ddsrecorder::yaml::RecorderConfiguration>(yml);
        configuration_->simple_configuration->domain = test::DOMAIN;
        configuration_->mcap_writer_options.compression = mcap::Compression::None;
        configuration_->buffer_size = 1;
    }

    void TearDown() override
    {
        // Delete the participant
        if (participant_ != nullptr)
        {
            participant_->delete_contained_entities();
            DomainParticipantFactory::get_instance()->delete_participant(participant_);
        }

        // Remove the output files
        for (const auto& path : paths_)
        {
            delete_file_(path);
        }
    }

protected:

    void reset_datawriter_()
    {
        if (writer_ != nullptr)
        {
            publisher_->delete_datawriter(writer_);
        }

        // Configure the DataWriter's QoS to ensure that the DDS Recorder receives all the msgs
        DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;
        wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
        wqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        wqos.history().kind = KEEP_ALL_HISTORY_QOS;

        // Create the writer
        writer_ = publisher_->create_datawriter(topic_, wqos);

        ASSERT_NE(writer_, nullptr);
    }

    void publish_msgs_(
            const std::uint32_t num_msgs)
    {
        // Reset the DataWriter to clear its history
        reset_datawriter_();

        for (std::uint32_t i = 0; i < num_msgs; i++)
        {
            HelloWorld hello;
            hello.index(i);
            writer_->write(&hello);

            // Wait for a millisecond
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    std::vector<std::filesystem::path> get_output_file_paths_(
            const std::uint32_t number_of_files,
            const std::string& output_file_name)
    {
        for (std::uint32_t i = 0; i < number_of_files; i++)
        {
            paths_.push_back(get_output_file_path_(output_file_name + "_" + std::to_string(i)));
        }

        return paths_;
    }

    std::filesystem::path get_output_file_path_(
            const std::string& output_file_name)
    {
        return std::filesystem::current_path() / (output_file_name + ".mcap");
    }

    bool delete_file_(
            const std::filesystem::path& file_path)
    {
        if (!std::filesystem::exists(file_path))
        {
            return true;
        }

        return std::filesystem::remove(file_path);
    }

    bool is_file_size_acceptable_(
            const std::filesystem::path& file_path)
    {
        if (!std::filesystem::exists(file_path))
        {
            return false;
        }

        const auto file_size = std::filesystem::file_size(file_path);
        const auto is_acceptable =
                file_size >= test::limits::MIN_ACCEPTABLE_FILE_SIZE &&
                file_size <= test::limits::MAX_ACCEPTABLE_FILE_SIZE;

        return is_acceptable;
    }

    DomainParticipant* participant_ = nullptr;
    Publisher* publisher_ = nullptr;
    Topic* topic_ = nullptr;
    DataWriter* writer_ = nullptr;

    std::unique_ptr<ddsrecorder::yaml::RecorderConfiguration> configuration_;
    std::vector<std::filesystem::path> paths_;

    std::shared_ptr<ddsrecorder::participants::FileTracker> file_tracker_;
};

/**
 * Test that the DDS Recorder doesn't exceed the max-file-size.
 *
 * CASES:
 * - check that the DDS Recorder records data until it reaches the max-file-size.
 */
TEST_F(McapResourceLimitsTest, max_file_size)
{
    const std::string OUTPUT_FILE_NAME = "max_file_size_test";
    const auto OUTPUT_FILE_PATH = get_output_file_path_(OUTPUT_FILE_NAME);

    configuration_->output_resource_limits_max_file_size = test::limits::MAX_FILE_SIZE;

    // Delete the output file if it exists
    ASSERT_TRUE(delete_file_(OUTPUT_FILE_PATH));

    ddsrecorder::recorder::DdsRecorder recorder(*configuration_, ddsrecorder::recorder::DdsRecorderStateCode::RUNNING,
            file_tracker_, OUTPUT_FILE_NAME);

    // Send many more messages than can be stored in a file with a size of max-file-size
    const auto WAY_TOO_MANY_MSGS = test::limits::FILE_OVERFLOW_THRESHOLD * 2;
    publish_msgs_(WAY_TOO_MANY_MSGS);

    // Make sure the DDS Recorder has received all the messages
    ASSERT_EQ(writer_->wait_for_acknowledgments(test::MAX_WAITING_TIME), ReturnCode_t::RETCODE_OK);

    // All the messages have been sent. Stop the DDS Recorder.
    recorder.stop();

    ASSERT_TRUE(is_file_size_acceptable_(OUTPUT_FILE_PATH));
}

/**
 * @brief Test that the DDS Recorder's output doesn't exceed the max-size.
 *
 * In this test, the DDS Recorder's output is configured to have a max-size of 30KiB and a max-file-size of 7.5KiB.
 * The DDS Recorder should create 4 output files, each of them with a size between 7KiB and 8KiB.
 *
 * A writer publishes 110 messages 4 times to verify that the DDS Recorder creates a new file after each batch of
 * messages. The writer then publishes 110 messages again to verify that the DDS Recorder doesn't create a new file,
 * since it would exceed the max-size. Then, the test verifies that the size of each of the DDS Recorder's output files
 * in bounds.
 *
 * CASES:
 * - check that the size of each of the DDS Recorder's output files is in bounds.
 * - check that the aggregate size of the DDS Recorder's output files is in bounds.
 */
TEST_F(McapResourceLimitsTest, max_size)
{
    constexpr std::uint32_t NUMBER_OF_FILES = test::limits::MAX_FILES + 1;
    const std::string OUTPUT_FILE_NAME = "max_size_test";
    const auto OUTPUT_FILE_PATHS = get_output_file_paths_(NUMBER_OF_FILES, OUTPUT_FILE_NAME);

    configuration_->output_resource_limits_max_file_size = test::limits::MAX_FILE_SIZE;
    configuration_->output_resource_limits_max_size = test::limits::MAX_SIZE;

    // Delete the output files if they exist
    for (const auto& path : OUTPUT_FILE_PATHS)
    {
        ASSERT_TRUE(delete_file_(path));
    }

    ddsrecorder::recorder::DdsRecorder recorder(*configuration_, ddsrecorder::recorder::DdsRecorderStateCode::RUNNING,
            file_tracker_, OUTPUT_FILE_NAME);

    for (std::uint32_t i = 0; i < test::limits::MAX_FILES; i++)
    {
        // Send more messages than can be stored in a file with a size of max-file-size
        publish_msgs_(test::limits::FILE_OVERFLOW_THRESHOLD);

        // Make sure the DDS Recorder has received all the messages
        ASSERT_EQ(writer_->wait_for_acknowledgments(test::MAX_WAITING_TIME), ReturnCode_t::RETCODE_OK);

        // All the messages have been sent. Stop the DDS Recorder.
        if (i == test::limits::MAX_FILES - 1)
        {
            recorder.stop();
        }

        // Verify that the DDS Recorder has created the expected number of output files and that their size is close
        // but doesn't exceed the max-file-size
        for (std::uint32_t j = 0; j <= i; j++)
        {
            ASSERT_TRUE(is_file_size_acceptable_(OUTPUT_FILE_PATHS[j]));
        }

        // Verify that the DDS Recorder hasn't created any extra files
        for (std::uint32_t j = i + 1; j < NUMBER_OF_FILES; j++)
        {
            ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATHS[j]));
        }
    }

    // Send more messages than can be stored in a file with a size of max-file-size
    publish_msgs_(test::limits::FILE_OVERFLOW_THRESHOLD);

    // Make sure the DDS Recorder has received all the messages
    ASSERT_EQ(writer_->wait_for_acknowledgments(test::MAX_WAITING_TIME), ReturnCode_t::RETCODE_OK);

    // Verify that the DDS Recorder hasn't created an extra file, since it would exceed the max-size
    ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATHS[4]));

    // Verify that the size of the rest of the files hasn't changed
    for (std::uint32_t i = 0; i < test::limits::MAX_FILES; i++)
    {
        ASSERT_TRUE(is_file_size_acceptable_(OUTPUT_FILE_PATHS[i]));
    }
}

/**
 * @brief Test that the DDS Recorder's applies file-rotation to its output after reaching the max-size.
 *
 * In this test, the DDS Recorder's output is configured to have a max-size of 30KiB and a max-file-size of 7.5KiB.
 * The DDS Recorder should create 3 output files, each of them with a size between 7KiB and 8KiB. Then, after
 * receiving more data, the DDS Recorder should remove the oldest files and create new ones.
 *
 * A writer publishes 110 messages 3 times to verify that the DDS Recorder creates a new file after each batch of
 * messages. The writer then publishes 110 messages 5 more times to verify that, after each batch of messages, the DDS
 * Recorder deletes the oldest file and creates a new one. The test also verifies that the size of each of the DDS
 * Recorder's output files is close to the max-file-size but doesn't surpass it.
 *
 * CASES:
 * - check that the size of each of the DDS Recorder's output files is close but doesn't exceed the max-file-size.
 * - check that the aggregate size of the DDS Recorder's output files doesn't exceed the max-size.
 * - check that the DDS Recorder applies file rotation after reaching the max-size.
 */
TEST_F(McapResourceLimitsTest, file_rotation)
{
    constexpr std::uint32_t NUMBER_OF_FILES = 9;
    const std::string OUTPUT_FILE_NAME = "file_rotation_test";
    const auto OUTPUT_FILE_PATHS = get_output_file_paths_(NUMBER_OF_FILES + 1, OUTPUT_FILE_NAME);

    configuration_->output_resource_limits_max_file_size = test::limits::MAX_FILE_SIZE;
    configuration_->output_resource_limits_max_size = test::limits::MAX_SIZE;
    configuration_->output_resource_limits_file_rotation = true;

    // Delete the output files if they exist
    for (const auto& path : OUTPUT_FILE_PATHS)
    {
        ASSERT_TRUE(delete_file_(path));
    }

    ddsrecorder::recorder::DdsRecorder recorder(*configuration_, ddsrecorder::recorder::DdsRecorderStateCode::RUNNING,
            file_tracker_, OUTPUT_FILE_NAME);

    // Verify that the DDS Recorder creates a new file after each batch of messages, before reaching the max-size
    for (std::uint32_t i = 0; i < test::limits::MAX_FILES - 1; i++)
    {
        // Send more messages than can be stored in a file with a size of max-file-size
        publish_msgs_(test::limits::FILE_OVERFLOW_THRESHOLD);

        // Make sure the DDS Recorder has received all the messages
        ASSERT_EQ(writer_->wait_for_acknowledgments(test::MAX_WAITING_TIME), ReturnCode_t::RETCODE_OK);

        // Verify that the DDS Recorder has created the expected number of output files
        for (std::uint32_t j = 0; j <= i; j++)
        {
            ASSERT_TRUE(is_file_size_acceptable_(OUTPUT_FILE_PATHS[j]));
        }

        // Verify that the DDS Recorder hasn't created any extra files
        for (std::uint32_t j = i + 1; j < NUMBER_OF_FILES; j++)
        {
            ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATHS[j]));
        }
    }

    // Verify that the DDS Recorder applies file rotation after reaching the max-size
    for (std::uint32_t i = 0; i < NUMBER_OF_FILES - (test::limits::MAX_FILES - 1); i++)
    {
        // Send more messages than can be stored in a file with a size of max-file-size
        publish_msgs_(test::limits::FILE_OVERFLOW_THRESHOLD);

        // Make sure the DDS Recorder has received all the messages
        ASSERT_EQ(writer_->wait_for_acknowledgments(test::MAX_WAITING_TIME), ReturnCode_t::RETCODE_OK);

        // Verify that the DDS Recorder has removed the oldest files
        for (std::uint32_t j = 0; j <= i; j++)
        {
            ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATHS[j]));
        }

        // Verify that the DDS Recorder has created the expected number of output files and that their size is close
        // but doesn't exceed the max-file-size
        for (std::uint32_t j = i + 1; j < i + test::limits::MAX_FILES; j++)
        {
            ASSERT_TRUE(is_file_size_acceptable_(OUTPUT_FILE_PATHS[j]));
        }

        // Verify that the DDS Recorder hasn't created any extra files
        for (std::uint32_t j = i + (NUMBER_OF_FILES - test::limits::MAX_FILES); j < NUMBER_OF_FILES; j++)
        {
            ASSERT_FALSE(std::filesystem::exists(OUTPUT_FILE_PATHS[j]));
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
