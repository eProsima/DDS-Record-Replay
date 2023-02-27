// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <cpp_utils/event/FileWatcherHandler.hpp>
#include <cpp_utils/event/MultipleEventHandler.hpp>
#include <cpp_utils/event/PeriodicEventHandler.hpp>
#include <cpp_utils/event/SignalEventHandler.hpp>
#include <cpp_utils/exception/ConfigurationException.hpp>
#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/logging/CustomStdLogConsumer.hpp>
#include <cpp_utils/ReturnCode.hpp>
#include <cpp_utils/thread_pool/pool/SlotThreadPool.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/utils.hpp>

#include <ddspipe_core/core/DdsPipe.hpp>
#include <ddspipe_core/dynamic/AllowedTopicList.hpp>
#include <ddspipe_core/dynamic/DiscoveryDatabase.hpp>
#include <ddspipe_core/dynamic/ParticipantsDatabase.hpp>
#include <ddspipe_core/efficiency/payload/FastPayloadPool.hpp>

#include <ddspipe_participants/participant/dynamic_types/DynTypesParticipant.hpp>
#include <ddspipe_participants/participant/dynamic_types/SchemaParticipant.hpp>

#include <ddsrecorder_participants/mcap/McapHandler.hpp>

#include <ddsrecorder_yaml/YamlReaderConfiguration.hpp>

#include "user_interface/constants.hpp"
#include "user_interface/arguments_configuration.hpp"
#include "user_interface/ProcessReturnCode.hpp"

#include "command_receiver/CommandReceiver.hpp"

using namespace eprosima::ddspipe;
using namespace eprosima::ddsrecorder;

std::string file_path = "/home/irenebm/annapurna/DDS-Recorder/src/ddsrecorder/conf-recorder.yaml";

std::unique_ptr<core::DdsPipe> create_recorder(
        const eprosima::ddsrecorder::yaml::Configuration& configuration)
{
    std::shared_ptr<eprosima::ddsrecorder::participants::McapHandler> mcap_handler;

    // Create allowed topics list
    auto allowed_topics = std::make_shared<core::AllowedTopicList>(
        [],
        []);

    // Create Discovery Database
    std::shared_ptr<core::DiscoveryDatabase> discovery_database =
        std::make_shared<core::DiscoveryDatabase>();

    // Create Payload Pool
    std::shared_ptr<core::PayloadPool> payload_pool =
        std::make_shared<core::FastPayloadPool>();

    // Create Thread Pool
    std::shared_ptr<eprosima::utils::SlotThreadPool> thread_pool =
        std::make_shared<eprosima::utils::SlotThreadPool>(configuration.n_threads);

    // Create MCAP Handler
    std::string file_name = "output.mcap";
    mcap_handler = std::make_shared<eprosima::ddsrecorder::participants::McapHandler>(
        file_name.c_str(),
        payload_pool,
        10,
        5,
        3,
        10);

    // Create DynTypes Participant
    auto dyn_participant = std::make_shared<eprosima::ddspipe::participants::DynTypesParticipant>(
        configuration.simple_configuration,
        payload_pool,
        discovery_database);
    dyn_participant->init();

    // Create Recorder Participant
    auto recorder_participant = std::make_shared<eprosima::ddspipe::participants::SchemaParticipant>(
        configuration.recorder_configuration,
        payload_pool,
        discovery_database,
        mcap_handler);

    // Create and populate Participant Database
    std::shared_ptr<core::ParticipantsDatabase> participant_database =
        std::make_shared<core::ParticipantsDatabase>();

    // Populate Participant Database
    participant_database->add_participant(
        dyn_participant->id(),
        dyn_participant
    );
    participant_database->add_participant(
        recorder_participant->id(),
        recorder_participant
    );

    return std::make_unique<core::DdsPipe>(
        allowed_topics,
        discovery_database,
        payload_pool,
        participant_database,
        thread_pool,
        configuration.builtin_topics,
        true
    );
}


/**
 * Test a DDS Recorder
 *
 * ddsrecorder -c src/ddsrecorder/conf-recorder.yaml -d
 * ./build/fastrtps/examples/cpp/dds/TypeIntrospectionExample/TypeIntrospectionExample publisher -d 100 -i
 *
 */
TEST(McapFileCreationTest, mcap_data)
{
    // DDS Recorder Initialization
    logUser(DDSRECORDER_EXECUTION, "Starting DDS Recorder execution.");

    logUser(DDSRECORDER_EXECUTION, "DDS Recorder running.");
    auto recorder = create_recorder();


    logUser(DDSRECORDER_EXECUTION, "Stopping DDS Recorder.");

    logUser(DDSRECORDER_EXECUTION, "DDS Recorder stopped correctly.");

    logUser(DDSRECORDER_EXECUTION, "Finishing DDS Recorder execution correctly.");
}




int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
