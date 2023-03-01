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

/**
 * @file CommandReceiver.h
 *
 */

#pragma once

#include <atomic>

#include <cpp_utils/enum/EnumBuilder.hpp>
#include <cpp_utils/event/MultipleEventHandler.hpp>
#include <cpp_utils/macros/custom_enumeration.hpp>
#include <cpp_utils/macros/macros.hpp>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>

#include "types/controller_command/ControllerCommandPubSubTypes.h"
#include "types/status/StatusPubSubTypes.h"

namespace eprosima {
namespace ddsrecorder {
namespace receiver {

ENUMERATION_BUILDER(
    CommandCode,
    NONE,
    START,
    PAUSE,
    EVENT,
    STOP,
    CLOSE,
    UNKNOWN
    );

eProsima_ENUMERATION_BUILDER(
    CommandCodeBuilder,
    CommandCode,
            {
                { CommandCode::NONE COMMA { "NONE" } } COMMA
                { CommandCode::START COMMA { "START" } } COMMA
                { CommandCode::PAUSE COMMA { "PAUSE" } } COMMA
                { CommandCode::EVENT COMMA { "EVENT" } } COMMA
                { CommandCode::STOP COMMA { "STOP" } } COMMA
                { CommandCode::CLOSE COMMA { "CLOSE" } } COMMA
                { CommandCode::UNKNOWN COMMA { "UNKNOWN" } }
            }
    );

class CommandReceiver : public eprosima::fastdds::dds::DataReaderListener
{
public:

    CommandReceiver(
            uint32_t domain,
            eprosima::utils::event::MultipleEventHandler* event_handler);

    virtual ~CommandReceiver();

    bool init();

    void wait_for_command();

    CommandCode command_received();

    void publish_status(
            CommandCode current,
            CommandCode previous,
            std::string info = "");

    void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override;

    void on_subscription_matched(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

private:

    static std::string command_to_status_string_(
            CommandCode command);

    std::atomic<CommandCode> command_received_;

    // DDS related attributes
    uint32_t domain_;
    eprosima::fastdds::dds::DomainParticipant* participant_;

    // Command attributes
    eprosima::fastdds::dds::Subscriber* command_subscriber_;
    eprosima::fastdds::dds::Topic* command_topic_;
    eprosima::fastdds::dds::DataReader* command_reader_;
    eprosima::fastdds::dds::TypeSupport command_type_;

    // Status attributes
    eprosima::fastdds::dds::Publisher* status_publisher_;
    eprosima::fastdds::dds::Topic* status_topic_;
    eprosima::fastdds::dds::DataWriter* status_writer_;
    eprosima::fastdds::dds::TypeSupport status_type_;

    eprosima::utils::event::MultipleEventHandler* event_handler_;
};

} /* namespace receiver */
} /* namespace ddsrecorder */
} /* namespace eprosima */
