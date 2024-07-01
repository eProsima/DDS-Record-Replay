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

#include <memory>

#include <gtest/gtest.h>
#include <mcap/writer.hpp>

#include <cpp_utils/Log.hpp>
#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/testing/gtest_aux.hpp>
#include <cpp_utils/testing/LogChecker.hpp>

#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>

#include <ddsrecorder_participants/recorder/mcap/McapHandler.hpp>
#include <ddsrecorder_participants/recorder/mcap/McapHandlerConfiguration.hpp>
#include <ddsrecorder_participants/recorder/output/BaseHandler.hpp>
#include <ddsrecorder_participants/recorder/output/FileTracker.hpp>
#include <ddsrecorder_participants/recorder/output/OutputSettings.hpp>

#include "../constants.hpp"

using namespace eprosima;

/**
 * Test case to verify a logError is displayed when the opening mcap file fails
 *
 * CASES:
 *  This test attemps to open a mcap file in a folder that does not exist, leading to
 *  its correspondent Log Error. An additional logError failing to rename the MCAP file
 *  will appear when the McapHandler destructor is called (this happens after
 *  log_checker.check_valid() assertion)
 */
TEST(McapLogErrorTest, fail_to_open_file) {

    // Create an instance of the Log Checker to capture the LogError
    utils::testing::LogChecker log_checker(utils::Log::Kind::Error, 1, 1);

    // Verify that no logs have been captured yet
    ASSERT_FALSE(log_checker.check_valid());

    // Configure a non-existent file path and name to trigger the error
    ddsrecorder::participants::OutputSettings output_settings;
    output_settings.filepath = "./fake_folder";
    output_settings.filename = "output_dummy";

    // Configure the max sizes to avoid triggering a different error
    output_settings.max_file_size = 10000;
    output_settings.max_size = output_settings.max_file_size;

    // Create the MCAP Handler's Configuration
    mcap::McapWriterOptions mcap_writer_options{"ros2"};

    ddsrecorder::participants::McapHandlerConfiguration config(
        output_settings,
        test::handler::MAX_PENDING_SAMPLES,
        test::handler::BUFFER_SIZE,
        test::handler::EVENT_WINDOW,
        test::handler::CLEANUP_PERIOD,
        test::handler::LOG_PUBLISH_TIME,
        test::handler::ONLY_WITH_SCHEMA,
        mcap_writer_options,
        test::handler::RECORD_TYPES,
        test::handler::ROS2_TYPES);

    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool;
    const auto init_state = ddsrecorder::participants::BaseHandlerStateCode::RUNNING;

    // Create the FileTracker
    auto file_tracker = std::make_shared<ddsrecorder::participants::FileTracker>(config.output_settings);

    // Verify that an InitializationException was thrown
    ASSERT_THROW(
        ddsrecorder::participants::McapHandler(config, payload_pool, file_tracker, init_state),
        utils::InitializationException);

    // Verify that a logError was captured
    ASSERT_TRUE(log_checker.check_valid());
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
