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
 * @file CommandReceiver.cpp
 *
 */

#include <cpp_utils/Log.hpp>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

#include "CommandReceiver.hpp"

namespace eprosima {
namespace ddsrecorder {
namespace receiver {

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

CommandReceiver::CommandReceiver(
        uint32_t domain,
        std::shared_ptr<eprosima::utils::event::MultipleEventHandler> event_handler)
    : domain_(domain)
    , participant_(nullptr)
    , command_subscriber_(nullptr)
    , command_topic_(nullptr)
    , command_reader_(nullptr)
    , command_type_(new DdsRecorderCommandPubSubType())
    , status_publisher_(nullptr)
    , status_topic_(nullptr)
    , status_writer_(nullptr)
    , status_type_(new DdsRecorderStatusPubSubType())
    , event_handler_(event_handler)
{
}

bool CommandReceiver::init()
{
    // CREATE THE PARTICIPANT
    DomainParticipantQos pqos;
    pqos.name("CommandReceiver");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(domain_, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    /////////////////////////////////
    // CREATE COMMAND DDS ENTITIES //
    /////////////////////////////////

    // REGISTER THE TYPE
    command_type_.register_type(participant_);

    // CREATE THE SUBSCRIBER
    command_subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

    if (command_subscriber_ == nullptr)
    {
        return false;
    }

    // CREATE THE TOPIC
    command_topic_ = participant_->create_topic(
        "/ddsrecorder/command",
        command_type_->getName(),
        TOPIC_QOS_DEFAULT);

    if (command_topic_ == nullptr)
    {
        return false;
    }

    // CREATE THE READER
    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    rqos.durability().kind = VOLATILE_DURABILITY_QOS;
    rqos.history().kind = KEEP_LAST_HISTORY_QOS;
    rqos.history().depth = 1; //TODO: increase?

    command_reader_ = command_subscriber_->create_datareader(command_topic_, rqos, this);

    if (command_reader_ == nullptr)
    {
        return false;
    }

    /////////////////////////////////
    // CREATE STATUS DDS ENTITIES //
    /////////////////////////////////

    // REGISTER THE TYPE
    status_type_.register_type(participant_);

    // CREATE THE PUBLISHER
    status_publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

    if (status_publisher_ == nullptr)
    {
        return false;
    }

    // CREATE THE TOPIC
    status_topic_ = participant_->create_topic(
        "/ddsrecorder/status",
        status_type_->getName(),
        TOPIC_QOS_DEFAULT);

    if (status_topic_ == nullptr)
    {
        return false;
    }

    // CREATE THE WRITER
    DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;
    wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    wqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    wqos.history().kind = KEEP_LAST_HISTORY_QOS;
    wqos.history().depth = 1;

    status_writer_ = status_publisher_->create_datawriter(status_topic_, wqos);

    if (status_writer_ == nullptr)
    {
        return false;
    }

    return true;
}

CommandReceiver::~CommandReceiver()
{
    if (participant_ != nullptr)
    {
        if (command_subscriber_ != nullptr)
        {
            if (command_reader_ != nullptr)
            {
                command_subscriber_->delete_datareader(command_reader_);
            }
            participant_->delete_subscriber(command_subscriber_);
        }
        if (command_topic_ != nullptr)
        {
            participant_->delete_topic(command_topic_);
        }
        if (status_publisher_ != nullptr)
        {
            if (status_writer_ != nullptr)
            {
                status_publisher_->delete_datawriter(status_writer_);
            }
            participant_->delete_publisher(status_publisher_);
        }
        if (status_topic_ != nullptr)
        {
            participant_->delete_topic(status_topic_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

DdsRecorderCommand CommandReceiver::wait_for_command()
{
    DdsRecorderCommand ret;
    event_handler_->wait_for_event();

    std::lock_guard<std::mutex> lock(mtx_);
    if (event_handler_->event_count() > commands_received_.size())
    {
        // If the events count is greater than the num of commands received, it's because a signal was received -> EXIT
        ret.command("close");
    }
    else  /* = event_count == commands_received_.size */
    {
        event_handler_->decrement_event_count();
        ret = commands_received_.front();
        commands_received_.pop();
    }

    return ret;
}

void CommandReceiver::publish_status(
        CommandCode current,
        CommandCode previous,
        std::string info)
{
    DdsRecorderStatus status;
    status.current(command_to_status_string_(current));
    status.previous(command_to_status_string_(previous));
    if (!info.empty())
    {
        status.info(info);
    }
    logInfo(
        DDSRECORDER_COMMAND_RECEIVER,
        "Publishing status: " << status.previous() << " ---> " << status.current() <<  " with info [" << status.info() <<
            " ].");
    status_writer_->write(&status);
}

void CommandReceiver::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        logInfo(
            DDSRECORDER_COMMAND_RECEIVER,
            "Subscriber matched [ " << iHandle2GUID(info.last_publication_handle) << " ].");
    }
    else if (info.current_count_change == -1)
    {
        logInfo(
            DDSRECORDER_COMMAND_RECEIVER,
            "Subscriber unmatched [ " << iHandle2GUID(info.last_publication_handle) << " ].");
    }
    else
    {
        logWarning(
            DDSRECORDER_COMMAND_RECEIVER,
            info.current_count_change << " is not a valid value for SubscriptionMatchedStatus current count change");
    }
}

void CommandReceiver::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    DdsRecorderCommand controller_command;
    while ((reader->take_next_sample(&controller_command,
            &info)) == (ReturnCode_t::RETCODE_OK && info.instance_state == ALIVE_INSTANCE_STATE))
    {
        logInfo(
            DDSRECORDER_COMMAND_RECEIVER,
            "New command received: " << controller_command.command() << " [" << controller_command.args() << "]");
        {
            std::lock_guard<std::mutex> lock(mtx_);
            commands_received_.push(controller_command);
        }
        event_handler_->simulate_event_occurred();
    }
}

std::string CommandReceiver::command_to_status_string_(
        const CommandCode& command)
{
    switch (command)
    {
        case CommandCode::start:
            return "RUNNING";

        case CommandCode::pause:
        case CommandCode::event:
            return "PAUSED";

        case CommandCode::stop:
            return "STOPPED";

        case CommandCode::close:
            return "CLOSED";

        case CommandCode::unknown:
        default:
            return "UNKNOWN";
    }
}

} /* namespace receiver */
} /* namespace ddsrecorder */
} /* namespace eprosima */
