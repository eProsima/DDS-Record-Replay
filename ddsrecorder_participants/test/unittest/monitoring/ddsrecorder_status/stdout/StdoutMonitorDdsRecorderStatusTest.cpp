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
#include <thread>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <ddspipe_core/configuration/MonitorConfiguration.hpp>

#include <ddsrecorder_participants/recorder/monitoring/DdsRecorderMonitor.hpp>
#include <ddsrecorder_participants/recorder/monitoring/producers/DdsRecorderStatusMonitorProducer.hpp>

#include "../../constants.hpp"

using namespace eprosima;
using namespace eprosima::fastdds::dds;


class StdoutMonitorDdsRecorderStatusTest : public testing::Test
{
public:

    void SetUp() override
    {
        // Initialize the Monitor
        ddspipe::core::MonitorConfiguration configuration;
        configuration.producers["status"].enabled = true;
        configuration.producers["status"].period = test::monitor::PERIOD_MS;

        utils::Formatter error_msg;
        ASSERT_TRUE(configuration.is_valid(error_msg));

        monitor_ = std::make_unique<ddsrecorder::participants::DdsRecorderMonitor>(configuration);

        if (configuration.producers["status"].enabled)
        {
            monitor_->monitor_status();
        }
    }

    void TearDown() override
    {
        monitor_.reset(nullptr);
    }

protected:

    std::unique_ptr<ddsrecorder::participants::DdsRecorderMonitor> monitor_{nullptr};
};

/**
 * Test that the Monitor monitors the type mismatch correctly.
 *
 * CASES:
 * - check that the Monitor prints the type_mismatch correctly.
 */
TEST_F(StdoutMonitorDdsRecorderStatusTest, type_mismatch)
{
    // Mock a type mismatch
    monitor_error("TYPE_MISMATCH");

    testing::internal::CaptureStdout();

    // Wait for the monitor to print the message
    std::this_thread::sleep_for(std::chrono::milliseconds(test::monitor::PERIOD_MS+1));

    ASSERT_EQ(testing::internal::GetCapturedStdout(),
            "DdsRecorder Monitoring Status: [TYPE_MISMATCH]\n");
}

/**
 * Test that the Monitor monitors the QoS mismatch correctly.
 *
 * CASES:
 * - check that the Monitor prints the qos_mismatch correctly.
 */
TEST_F(StdoutMonitorDdsRecorderStatusTest, qos_mismatch)
{
    // Mock a qos mismatch
    monitor_error("QOS_MISMATCH");

    testing::internal::CaptureStdout();

    // Wait for the monitor to print the message
    std::this_thread::sleep_for(std::chrono::milliseconds(test::monitor::PERIOD_MS+1));

    ASSERT_EQ(testing::internal::GetCapturedStdout(),
            "DdsRecorder Monitoring Status: [QOS_MISMATCH]\n");
}

/**
 * Test that the Monitor monitors the MCAP file creation failure correctly.
 *
 * CASES:
 * - check that the Monitor prints the mcap_file_creation_failure correctly.
 */
TEST_F(StdoutMonitorDdsRecorderStatusTest, mcap_file_creation_failure)
{
    // Mock a mcap file creation failure
    monitor_error("MCAP_FILE_CREATION_FAILURE");

    testing::internal::CaptureStdout();

    // Wait for the monitor to print the message
    std::this_thread::sleep_for(std::chrono::milliseconds(test::monitor::PERIOD_MS+1));

    ASSERT_EQ(testing::internal::GetCapturedStdout(),
            "DdsRecorder Monitoring Status: [MCAP_FILE_CREATION_FAILURE]\n");
}

/**
 * Test that the Monitor monitors the disk full correctly.
 *
 * CASES:
 * - check that the Monitor prints the disk_full correctly.
 */
TEST_F(StdoutMonitorDdsRecorderStatusTest, disk_full)
{
    // Mock a disk full
    monitor_error("DISK_FULL");

    testing::internal::CaptureStdout();

    // Wait for the monitor to print the message
    std::this_thread::sleep_for(std::chrono::milliseconds(test::monitor::PERIOD_MS+1));

    ASSERT_EQ(testing::internal::GetCapturedStdout(),
            "DdsRecorder Monitoring Status: [DISK_FULL]\n");
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
