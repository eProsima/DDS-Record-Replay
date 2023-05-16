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

#define MCAP_IMPLEMENTATION

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/logging/CustomStdLogConsumer.hpp>

#include <ddspipe_core/core/DdsPipe.hpp>
#include <ddspipe_core/efficiency/payload/FastPayloadPool.hpp>

#include <ddspipe_participants/participant/dynamic_types/DynTypesParticipant.hpp>
#include <ddspipe_participants/participant/dynamic_types/SchemaParticipant.hpp>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastrtps/types/DynamicDataPtr.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/TypeObjectFactory.h>

#include <mcap/reader.hpp>

#include "types/hello_world/HelloWorldTypeObject.h"
#include "types/hello_world/HelloWorldPubSubTypes.h"

#include <iostream>
#include <thread>
#include <chrono>

using namespace eprosima::fastdds::dds;
using namespace eprosima::ddspipe;
// using namespace eprosima::ddsrecorder;

// using McapHandlerState = eprosima::ddsrecorder::participants::McapHandlerStateCode;

enum class DataTypeKind
{
    HELLO_WORLD,
};

namespace test {

// Publisher

const unsigned int DOMAIN = 222;

std::string topic = "TypeIntrospectionTopic";
std::string data_type_name = "HelloWorld";

unsigned int n_msgs = 3;
std::string send_message = "Hello World";
unsigned int index = 6;
unsigned int downsampling = 3;

} // test

void create_subscriber()
{
    eprosima::fastdds::dds::DomainParticipantQos pqos;
    pqos.name("TypeIntrospectionExample_Participant_Subscriber");
    pqos.wire_protocol().builtin.typelookup_config.use_client = true;
    pqos.wire_protocol().builtin.typelookup_config.use_server = false;

    // Create the participant
    eprosima::fastdds::dds::DomainParticipant* participant_ =
            DomainParticipantFactory::get_instance()->create_participant(domain, pqos);

    // Create the Subscriber
    eprosima::fastdds::dds::Subscriber* subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
}

void get_information()
{
    // Create the DDS DataReader
    // test::reader_ = publisher_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, nullptr);

    / Wait for type discovery
    std::cout << "Subscriber waiting to discover type for topic < " << topic_name_
        << " >. Press CTRL+C to stop the Subscriber..." << std::endl;

    // Wait until the type is discovered and registered
    {
        std::unique_lock<std::mutex> lck(type_discovered_cv_mtx_);
        type_discovered_cv_.wait(lck, []
                {
                    return is_stopped() || (type_discovered_.load() && type_registered_.load());
                });
    }

    // Check if the application has already been stopped
    if (is_stopped())
    {
        return;
    }

    std::cout <<
        "Subscriber < " << datareader_->guid() <<
        " > listening for data in topic < " << topic_name_ <<
        " > found data type < " << dynamic_type_->get_name() <<
        " >" << std::endl;

    // Wait for expected samples or the user stops the execution
    if (samples > 0)
    {
        std::cout << "Running until " << samples <<
            " samples have been received. Press CTRL+C to stop the Subscriber at any time." << std::endl;
    }
    else
    {
        std::cout << "Press CTRL+C to stop the Subscriber." << std::endl;
    }

    {
        std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
        terminate_cv_.wait(lck, []
                {
                    return is_stopped();
                });
    }

    // Print number of data received
    std::cout <<
        "Subscriber received " << samples_ <<
        " samples." << std::endl;
}

TEST(McapFileCreationTest, trivial)
{
    create_subscriber();

    // replay(mcap_file, configuration);    TODO

    get_information();
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
