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

// #define MCAP_IMPLEMENTATION

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/event/MultipleEventHandler.hpp>

#include "dds/TypeIntrospectionSubscriber.h"

#include "tool/DdsReplayer.hpp"

#include <iostream>
#include <thread>
#include <chrono>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps;
using namespace eprosima::ddsrecorder::replayer;

namespace test {

// Publisher

const unsigned int DOMAIN = 0;

std::string topic_name = "TypeIntrospectionTopic";
std::string data_type_name = "HelloWorld";

unsigned int n_msgs = 3;
std::string send_message = "Hello World";
unsigned int index = 6;
unsigned int downsampling = 3;

} // test

enum class DataTypeKind
{
    HELLO_WORLD,
};


TEST(McapFileReadTest, trivial)
{

    std::cout << "creating subscriber !!!!" << std::endl;

    // Create Subscriber
    TypeIntrospectionSubscriber subscriber(
        test::topic_name,
        static_cast<uint32_t>(test::DOMAIN));

    std::cout << "subscriber created !!!!" << std::endl;

    // Create a multiple event handler that handles all events that make subscriber and replayer stop
    auto close_handler_replayer = std::make_shared<eprosima::utils::event::MultipleEventHandler>();
    auto close_handler_subscriber = std::make_shared<eprosima::utils::event::MultipleEventHandler>();

    // Run Participant
    std::thread run_subscriber([&] {
        try
        {
            subscriber.run(static_cast<uint32_t>(1));
        }
        catch (const eprosima::utils::InconsistencyException& e)
        {
            logError(DDSREPLAYER_ERROR,
                    "Error running subscriber. Error message:\n " <<
                    e.what());
        }
        close_handler_subscriber->simulate_event_occurred();
    });

    std::cout << "subscriber running !!!!" << std::endl;

    std::cout << "creating replayer !!!!" << std::endl;

    // Load configuration from YAML
    YAML::Node yml;
    eprosima::ddsrecorder::yaml::ReplayerConfiguration configuration(yml);

    std::string input_file = "resources/input_file.mcap";

    // Create replayer instance
    auto replayer = std::make_unique<DdsReplayer>(configuration, input_file);

    std::cout << "replayer created !!!!" << std::endl;

    std::cout << "start replaying data !!!!" << std::endl;
    // Start replaying data
    bool read_success;
    std::thread process_mcap_thread([&] {
        try
        {
            replayer->process_mcap();
            read_success = true;
        }
        catch (const eprosima::utils::InconsistencyException& e)
        {
            logError(DDSREPLAYER_ERROR,
                    "Error processing MCAP file. Error message:\n " <<
                    e.what());
            read_success = false;
        }
        close_handler_replayer->simulate_event_occurred();
    });

    // Wait until signal arrives (or all messages in MCAP file sent)
    close_handler_replayer->wait_for_event();
    std::cout << "event replayer ocurred" << std::endl;

    // Wait until signal arrives (or all messages in MCAP file sent)
    close_handler_subscriber->wait_for_event();
    std::cout << "event subscriber ocurred" << std::endl;

    // Disable inner pipe, which would abort replaying messages in case execution stopped by signal
    replayer->stop();
    std::cout << "replayer stopped!!!!" << std::endl;

    run_subscriber.join();
    process_mcap_thread.join();

    std::cout << "threads joined!!!!" << std::endl;

    std::cout << "process info..." << std::endl;

    // get_information();
    // eprosima::utils::sleep_for(5000);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
