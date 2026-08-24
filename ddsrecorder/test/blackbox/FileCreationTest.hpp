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


#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include <cpp_utils/Log.hpp>
#include <cpp_utils/logging/BaseLogConfiguration.hpp>
#include <cpp_utils/logging/StdLogConsumer.hpp>
#include <cpp_utils/ros2_mangling.hpp>

#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>
#include <ddspipe_yaml/Yaml.hpp>

#include <ddsrecorder_yaml/recorder/YamlReaderConfiguration.hpp>

#include <tool/DdsRecorder.hpp>

#include "constants.hpp"

#include "../resources/types/hello_world/HelloWorld.hpp"
#include "../resources/types/hello_world/HelloWorldPubSubTypes.hpp"

using namespace eprosima;
using DdsRecorderState = ddsrecorder::recorder::DdsRecorderStateCode;

enum class EventKind
{
    NO_EVENT,
    EVENT,
    EVENT_START,
    EVENT_STOP,
    EVENT_SUSPEND,
};

/**
 * @brief Match notifier owned by one DataWriter.
 *
 * The fixture creates a fresh writer for every message batch. A shared listener flag can be
 * signalled by an older writer and let the test publish before the current writer is matched.
 */
class FileCreationWriterMatchListener : public fastdds::dds::DataWriterListener
{
public:

    void on_publication_matched(
            fastdds::dds::DataWriter* /*writer*/,
            const fastdds::dds::PublicationMatchedStatus& info) override
    {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            matched_ = info.current_count > 0;
        }
        cv_.notify_all();
    }

    bool wait_for_matching(
            const std::chrono::seconds timeout)
    {
        std::unique_lock<std::mutex> lock(mtx_);
        return cv_.wait_for(lock, timeout, [this]()
                       {
                           return matched_;
                       });
    }

private:

    bool matched_{false};
    std::mutex mtx_;
    std::condition_variable cv_;
};

class FileCreationTest : public testing::Test
{
public:

    void SetUp() override
    {
        // Route DDS Pipe / DDS Recorder diagnostics to the test output.
        // NOTE: the log configuration is only ever applied by the tool's main(), so without this the
        // blackbox tests run with no consumer registered at all and every library warning or error is
        // silently discarded. That is why a dropped sample surfaces only as an unexplained
        // message-count mismatch. The default configuration is Warning verbosity with an empty
        // filter, which accepts every category.
        utils::Log::ClearConsumers();
        utils::Log::SetVerbosity(log_configuration_.verbosity);
        utils::Log::RegisterConsumer(std::make_unique<utils::StdLogConsumer>(&log_configuration_));

        // Create the participant
        fastdds::dds::DomainParticipantQos pqos;
        pqos.name(test::PARTICIPANT_ID);

        participant_ = fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(test::DOMAIN, pqos);

        ASSERT_NE(participant_, nullptr);

        // Register the type
        type_support_ = fastdds::dds::TypeSupport(new HelloWorldPubSubType());
        participant_->register_type(type_support_);

        // Create the publisher
        publisher_ = participant_->create_publisher(fastdds::dds::PUBLISHER_QOS_DEFAULT, nullptr);

        ASSERT_NE(publisher_, nullptr);

        // Create the RecorderConfiguration
        Yaml yml;
        configuration_ = std::make_unique<ddsrecorder::yaml::RecorderConfiguration>(yml);
        configuration_->dds_configuration->domain = test::DOMAIN;
        configuration_->dds_configuration->allowed_partition_list.insert("*");

        // Flush every sample to the handler. The drain helper below uses output-file progress
        // to know when the recorder has consumed all samples; with the default batch size, a
        // partially filled final batch is invisible and can be lost when the recorder is
        // destroyed.
        configuration_->buffer_size = 1;

        // Create the topic
        create_topic_();

        // Create the DataWriter
        create_datawriter_();
    }

    void TearDown() override
    {
        // Delete the participant
        if (participant_ != nullptr)
        {
            participant_->delete_contained_entities();
            fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
        }

        // Remove the output files
        for (const auto& path : paths_)
        {
            delete_file_(path);
        }

        // Flush before dropping the consumer,
        // so entries queued during teardown are still printed
        utils::Log::Flush();
        utils::Log::ClearConsumers();
    }

protected:

    std::vector<HelloWorld> record_messages_(
            const std::string& file_name,
            const unsigned int messages1,
            const DdsRecorderState state1 = DdsRecorderState::RUNNING,
            const unsigned int messages2 = 0,
            const DdsRecorderState state2 = DdsRecorderState::RUNNING,
            const unsigned int wait = 0,
            const EventKind event = EventKind::NO_EVENT,
            const std::string partition_filter = "*")
    {
        // Create the Recorder
        auto recorder = std::make_unique<ddsrecorder::recorder::DdsRecorder>(*configuration_, state1, file_name);

        recorder->update_filter(std::set<std::string>{partition_filter});

        // Send messages
        auto sent_messages = send_messages_(messages1);

        if (state1 != state2)
        {
            // Change the Recorder's state
            switch (state2)
            {
                case DdsRecorderState::RUNNING:
                    recorder->start();
                    break;
                case DdsRecorderState::PAUSED:
                    recorder->pause();
                    break;
                case DdsRecorderState::SUSPENDED:
                    recorder->suspend();
                    break;
                case DdsRecorderState::STOPPED:
                    recorder->stop();
                    break;
                default:
                    break;
            }
        }

        // Wait for the event window
        std::this_thread::sleep_for(std::chrono::seconds(wait));

        // Send more messages
        const auto sent_messages_after_transition = send_messages_(messages2);
        sent_messages.insert(sent_messages.end(),
                sent_messages_after_transition.begin(), sent_messages_after_transition.end());

        if (event != EventKind::NO_EVENT && state2 == DdsRecorderState::PAUSED)
        {
            recorder->trigger_event();

            switch (event)
            {
                case EventKind::EVENT_START:
                    recorder->start();
                    break;

                case EventKind::EVENT_SUSPEND:
                    recorder->suspend();
                    break;

                case EventKind::EVENT_STOP:
                    recorder->stop();
                    break;

                default:
                    break;
            }
        }

        // Give the recorder a chance to finish writing before it is destroyed below.
        //
        // wait_for_acknowledgments() in send_messages_ only proves the samples reached the
        // reader's history; the pipe still has to take them and hand them to the handler. Anything
        // untaken when the recorder is destroyed is lost, which under a sanitizer build is enough to
        // leave a recording short of the messages that were sent.
        wait_for_recording_to_drain_(file_name, sent_messages.size());

        return sent_messages;
    }

    /**
     * @brief Wait until the recorder has stopped writing, before it is destroyed.
     *
     * Only needed when more messages were sent than the handler buffers. Below that the handler has
     * not written anything yet and BaseHandler::stop() flushes the lot on destruction. Above it, at
     * least one batch has been flushed and the pipe may still be taking the rest from the reader;
     * whatever it has not taken when the recorder is destroyed is lost.
     *
     * NOTE: progress is measured from the size of the files the recorder is writing, deliberately
     * never by opening the database. SQLite serialises cross-connection WAL access with file locks,
     * which ThreadSanitizer cannot model as happens-before, so polling the database from here
     * reports races in walIndexWriteHdr and fails the sanitizer builds.
     *
     * @param file_name  Name of the output file, without extension.
     * @param expected   Number of messages sent, i.e. the most that could be recorded.
     */
    void wait_for_recording_to_drain_(
            const std::string& file_name,
            const std::size_t expected)
    {
        if (expected <= configuration_->buffer_size)
        {
            // Nothing has been written yet, so nothing can be left behind
            return;
        }

        const auto base = (std::filesystem::current_path() / file_name).string();
        const std::vector<std::string> outputs =
        {
            base + ".db.tmp~",
            base + ".db.tmp~-wal",
            base + ".mcap.tmp~"
        };

        const auto written_bytes = [&outputs]() -> std::uintmax_t
                {
                    std::error_code ec;
                    std::uintmax_t total = 0;

                    for (const auto& output : outputs)
                    {
                        if (std::filesystem::exists(output, ec))
                        {
                            total += std::filesystem::file_size(output, ec);
                        }
                    }

                    return total;
                };

        constexpr auto POLL_PERIOD = std::chrono::milliseconds(100);
        // Kept above the time one batch of buffer_size messages takes to publish, so a pause between
        // flushes is not mistaken for the end of the recording
        constexpr auto STABLE_PERIOD = std::chrono::milliseconds(1500);
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);

        auto last_size = written_bytes();
        auto last_change = std::chrono::steady_clock::now();

        while (std::chrono::steady_clock::now() < deadline)
        {
            std::this_thread::sleep_for(POLL_PERIOD);

            const auto size = written_bytes();

            if (size != last_size)
            {
                last_size = size;
                last_change = std::chrono::steady_clock::now();
            }
            else if (std::chrono::steady_clock::now() - last_change >= STABLE_PERIOD)
            {
                // The recorder is no longer writing
                return;
            }
        }
    }

    std::vector<HelloWorld> send_messages_(
            const unsigned int number_of_messages)
    {
        // Create the DataWriter
        create_datawriter_();

        // Wait for the DataWriter to match the DataReader
        if (!writer_match_listener_->wait_for_matching(std::chrono::seconds(2)))
        {
            ADD_FAILURE() << "DataWriter did not match any DataReader within the timeout.";
            delete_datawriter_();
            return {};
        }

        // Send the messages
        std::vector<HelloWorld> sent_messages;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        for (std::uint32_t i = 0; i < number_of_messages; i++)
        {
            // Create the message
            HelloWorld hello;
            hello.index(i);
            hello.message("Hello World!");

            // Send the message
            writer_->write(&hello);

            // Store the message
            sent_messages.push_back(hello);

            // Wait for the message to be sent
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // Do not delete the DataWriter while samples are still unacknowledged. This history is
        // RELIABLE / KEEP_ALL / TRANSIENT_LOCAL, so any change the DDS Recorder has not acknowledged
        // yet is discarded together with the writer, and the recording ends up silently short.
        if (number_of_messages > 0)
        {
            EXPECT_EQ(writer_->wait_for_acknowledgments(test::MAX_WAITING_TIME), fastdds::dds::RETCODE_OK)
                << "The DDS Recorder did not acknowledge all " << number_of_messages << " samples.";
        }

        // Delete the DataWriter
        delete_datawriter_();

        return sent_messages;
    }

    std::shared_ptr<fastdds::rtps::SerializedPayload_t> to_cdr(
            const HelloWorld& message)
    {
        HelloWorldPubSubType pubsubType;
        const auto payload_size = pubsubType.calculate_serialized_size(&message,
                        fastdds::dds::DEFAULT_DATA_REPRESENTATION);
        auto payload = std::make_shared<fastdds::rtps::SerializedPayload_t>(payload_size);
        pubsubType.serialize(&message, *payload, fastdds::dds::DEFAULT_DATA_REPRESENTATION);

        return payload;
    }

    std::string to_json(
            const HelloWorld& message)
    {
        // Get type object
        fastdds::dds::xtypes::TypeObjectPair type_objects;
        fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
            type_support_->get_name(),
            type_objects);

        // Build dynamic type
        fastdds::dds::DynamicType::_ref_type dyn_type =
                fastdds::dds::DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
            type_objects.complete_type_object)->build();

        // Build dynamic data
        fastdds::dds::DynamicData::_ref_type dyn_data =
                fastdds::dds::DynamicDataFactory::get_instance()->create_data(dyn_type);

        // Transform the message into DynamicData
        const auto payload_size = type_support_->calculate_serialized_size(&message,
                        fastdds::dds::DEFAULT_DATA_REPRESENTATION);
        auto payload = std::make_shared<fastdds::rtps::SerializedPayload_t>(payload_size);
        type_support_->serialize(&message, *payload, fastdds::dds::DEFAULT_DATA_REPRESENTATION);

        fastdds::dds::TypeSupport dyn_type_support(new fastdds::dds::DynamicPubSubType(dyn_type));
        dyn_type_support->deserialize(*payload, &dyn_data);

        // Serialize DynamicType into its IDL representation
        std::stringstream data_json;
        json_serialize(dyn_data, fastdds::dds::DynamicDataJsonFormat::EPROSIMA, data_json);

        return data_json.str();
    }

    void recreate_datawriter_()
    {
        // Delete the existing DataWriter
        delete_datawriter_();

        // delete the topic
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }

        // Create a new topic
        create_topic_();

        // Create a new DataWriter
        create_datawriter_();
    }

    void create_topic_()
    {
        const auto topic_name = (configuration_->ros2_types) ? test::ROS2_TOPIC_NAME : test::TOPIC_NAME;
        topic_ = participant_->create_topic(topic_name, "HelloWorld", fastdds::dds::TOPIC_QOS_DEFAULT);
    }

    void create_datawriter_()
    {
        // Configure the DataWriter's QoS to ensure that the DDS Recorder receives all the msgs
        fastdds::dds::DataWriterQos wqos = fastdds::dds::DATAWRITER_QOS_DEFAULT;
        wqos.reliability().kind = fastdds::dds::RELIABLE_RELIABILITY_QOS;
        wqos.durability().kind = fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
        wqos.history().kind = fastdds::dds::KEEP_ALL_HISTORY_QOS;

        // Keep each listener alive for the lifetime of its writer. Older writers are deliberately
        // kept alive to avoid a transient loss of all writers on the topic between batches.
        auto match_listener = std::make_unique<FileCreationWriterMatchListener>();
        writer_match_listener_ = match_listener.get();
        writer_match_listeners_.push_back(std::move(match_listener));

        writer_ = publisher_->create_datawriter(topic_, wqos, writer_match_listener_);

        ASSERT_NE(writer_, nullptr);
    }

    void delete_datawriter_()
    {
        if (writer_ != nullptr)
        {
            publisher_->delete_datawriter(writer_);
            writer_ = nullptr;
        }
    }

    std::string get_output_file_path_(
            const std::string& output_file_name)
    {
        const auto file_path = std::filesystem::current_path() / output_file_name;

        paths_.push_back(file_path);

        return file_path.string();
    }

    bool delete_file_(
            const std::filesystem::path& file_path)
    {
        if (std::filesystem::exists(file_path) && !std::filesystem::remove(file_path))
        {
            return false;
        }

        const auto file_path_tmp = file_path.string() + ".tmp~";

        if (std::filesystem::exists(file_path_tmp) && !std::filesystem::remove(file_path_tmp))
        {
            return false;
        }

        return true;
    }

    fastdds::dds::DomainParticipant* participant_ = nullptr;
    fastdds::dds::TypeSupport type_support_;
    fastdds::dds::Publisher* publisher_ = nullptr;
    fastdds::dds::Topic* topic_ = nullptr;
    fastdds::dds::DataWriter* writer_ = nullptr;

    std::vector<std::filesystem::path> paths_;

    std::unique_ptr<ddsrecorder::yaml::RecorderConfiguration> configuration_;

    FileCreationWriterMatchListener* writer_match_listener_ = nullptr;
    std::vector<std::unique_ptr<FileCreationWriterMatchListener>> writer_match_listeners_;

    //! Warning verbosity with an empty (match-all) filter, see BaseLogConfiguration's constructor
    utils::BaseLogConfiguration log_configuration_;
};
