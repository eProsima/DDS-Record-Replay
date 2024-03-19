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

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <ddspipe_core/configuration/MonitorConfiguration.hpp>

#include <ddsrecorder_participants/recorder/monitoring/DdsRecorderMonitor.hpp>
#include <ddsrecorder_participants/recorder/monitoring/producers/DdsRecorderStatusMonitorProducer.hpp>

#if FASTRTPS_VERSION_MAJOR < 2 || (FASTRTPS_VERSION_MAJOR == 2 && FASTRTPS_VERSION_MINOR < 13)
    #include <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v1/DdsRecorderMonitoringStatus.h>
    #include \
    <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v1/DdsRecorderMonitoringStatusPubSubTypes.h>
#else
    #include <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v2/DdsRecorderMonitoringStatus.h>
    #include \
    <ddsrecorder_participants/common/types/monitoring/ddsrecorder_status/v2/DdsRecorderMonitoringStatusPubSubTypes.h>
#endif // if FASTRTPS_VERSION_MAJOR < 2 || (FASTRTPS_VERSION_MAJOR == 2 && FASTRTPS_VERSION_MINOR < 13)

#include "../../constants.hpp"

using namespace eprosima;
using namespace eprosima::fastdds::dds;


class DdsMonitorDdsRecorderStatusTest : public testing::Test
{
public:

    void SetUp() override
    {
        // Initialize the Monitor
        ddspipe::core::MonitorConfiguration configuration;
        configuration.producers["status"].enabled = true;
        configuration.producers["status"].period = test::monitor::PERIOD_MS;
        configuration.consumers["status"].domain = test::monitor::DOMAIN;
        configuration.consumers["status"].topic_name = test::monitor::TOPIC_NAME;

        utils::Formatter error_msg;
        ASSERT_TRUE(configuration.is_valid(error_msg));

        monitor_ = std::make_unique<ddsrecorder::participants::DdsRecorderMonitor>(configuration);

        if (configuration.producers["status"].enabled)
        {
            monitor_->monitor_status();
        }

        // Create the participant
        DomainParticipantQos pqos;
        pqos.name(test::monitor::PARTICIPANT_ID);

        participant_ = DomainParticipantFactory::get_instance()->create_participant(test::monitor::DOMAIN, pqos);

        ASSERT_NE(participant_, nullptr);

        // Register the type
        // TODO(tempate): write another test in which the type is discovered.
        TypeSupport type(new DdsRecorderMonitoringStatusPubSubType());
        type.register_type(participant_);

        // Create the subscriber
        Subscriber* subscriber = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

        ASSERT_NE(subscriber, nullptr);

        // Create the topic
        Topic* topic = participant_->create_topic(test::monitor::TOPIC_NAME, type.get_type_name(), TOPIC_QOS_DEFAULT);

        ASSERT_NE(topic, nullptr);

        // Create the reader
        reader_ = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT);

        ASSERT_NE(reader_, nullptr);
    }

    void TearDown() override
    {
        monitor_.reset(nullptr);

        if (nullptr != participant_)
        {
            participant_->delete_contained_entities();
            DomainParticipantFactory::get_instance()->delete_participant(participant_);
        }
    }

protected:

    std::unique_ptr<ddsrecorder::participants::DdsRecorderMonitor> monitor_{nullptr};

    DomainParticipant* participant_ = nullptr;
    DataReader* reader_ = nullptr;
};

/**
 * Test that the Monitor monitors the type mismatch correctly.
 *
 * CASES:
 * - check that the Monitor publishes the type_mismatch correctly.
 */
TEST_F(DdsMonitorDdsRecorderStatusTest, type_mismatch)
{
    // Mock a type mismatch
    monitor_error("TYPE_MISMATCH");

    DdsRecorderMonitoringStatus status;
    SampleInfo info;

    // Wait for the monitor to publish the next message
    ASSERT_TRUE(reader_->wait_for_unread_message(test::monitor::MAX_WAITING_TIME));

    ASSERT_EQ(reader_->take_next_sample(&status, &info), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(info.instance_state, ALIVE_INSTANCE_STATE);

    // Verify that the content of the DdsRecorderMonitoringStatus published by the Monitor is correct
    ASSERT_TRUE(status.error_status().type_mismatch());
    ASSERT_FALSE(status.error_status().qos_mismatch());
    ASSERT_TRUE(status.has_errors());
}

/**
 * Test that the Monitor monitors the QoS mismatch correctly.
 *
 * CASES:
 * - check that the Monitor publishes the qos_mismatch correctly.
 */
TEST_F(DdsMonitorDdsRecorderStatusTest, qos_mismatch)
{
    // Mock a QoS mismatch
    monitor_error("QOS_MISMATCH");

    DdsRecorderMonitoringStatus status;
    SampleInfo info;

    // Wait for the monitor to publish the next message
    ASSERT_TRUE(reader_->wait_for_unread_message(test::monitor::MAX_WAITING_TIME));

    ASSERT_EQ(reader_->take_next_sample(&status, &info), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(info.instance_state, ALIVE_INSTANCE_STATE);

    // Verify that the content of the DdsRecorderMonitoringStatus published by the Monitor is correct
    ASSERT_FALSE(status.error_status().type_mismatch());
    ASSERT_TRUE(status.error_status().qos_mismatch());
    ASSERT_TRUE(status.has_errors());
}

/**
 * Test that the Monitor monitors the MCAP file creation failure correctly.
 *
 * CASES:
 * - check that the Monitor publishes the mcap_file_creation_failure correctly.
 */
TEST_F(DdsMonitorDdsRecorderStatusTest, mcap_file_creation_failure)
{
    // Mock a mcap file creation failure
    monitor_error("MCAP_FILE_CREATION_FAILURE");

    DdsRecorderMonitoringStatus status;
    SampleInfo info;

    // Wait for the monitor to publish the next message
    ASSERT_TRUE(reader_->wait_for_unread_message(test::monitor::MAX_WAITING_TIME));

    ASSERT_EQ(reader_->take_next_sample(&status, &info), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(info.instance_state, ALIVE_INSTANCE_STATE);

    // Verify that the content of the DdsRecorderMonitoringStatus published by the Monitor is correct
    ASSERT_FALSE(status.error_status().qos_mismatch());
    ASSERT_FALSE(status.error_status().type_mismatch());
    ASSERT_TRUE(status.ddsrecorder_error_status().mcap_file_creation_failure());
    ASSERT_FALSE(status.ddsrecorder_error_status().disk_full());
    ASSERT_TRUE(status.has_errors());
}

/**
 * Test that the Monitor monitors the disk full correctly.
 *
 * CASES:
 * - check that the Monitor publishes the disk_full correctly.
 */
TEST_F(DdsMonitorDdsRecorderStatusTest, disk_full)
{
    // Mock a disk full
    monitor_error("DISK_FULL");

    DdsRecorderMonitoringStatus status;
    SampleInfo info;

    // Wait for the monitor to publish the next message
    ASSERT_TRUE(reader_->wait_for_unread_message(test::monitor::MAX_WAITING_TIME));

    ASSERT_EQ(reader_->take_next_sample(&status, &info), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(info.instance_state, ALIVE_INSTANCE_STATE);

    // Verify that the content of the DdsRecorderMonitoringStatus published by the Monitor is correct
    ASSERT_FALSE(status.error_status().qos_mismatch());
    ASSERT_FALSE(status.error_status().type_mismatch());
    ASSERT_FALSE(status.ddsrecorder_error_status().mcap_file_creation_failure());
    ASSERT_TRUE(status.ddsrecorder_error_status().disk_full());
    ASSERT_TRUE(status.has_errors());
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
