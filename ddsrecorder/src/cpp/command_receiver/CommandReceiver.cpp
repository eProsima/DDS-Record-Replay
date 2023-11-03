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
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>

#include <ddspipe_participants/participant/rtps/CommonParticipant.hpp>

#include "CommandReceiver.hpp"

namespace eprosima {
namespace ddsrecorder {
namespace recorder {
namespace receiver {

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

CommandReceiver::CommandReceiver(
        uint32_t domain,
        const std::string& command_topic_name,
        const std::string& status_topic_name,
        std::shared_ptr<eprosima::utils::event::MultipleEventHandler> event_handler,
        std::shared_ptr<eprosima::ddspipe::participants::SimpleParticipantConfiguration> participant_configuration)
    : domain_(domain)
    , participant_(nullptr)
    , command_topic_name_(command_topic_name)
    , command_subscriber_(nullptr)
    , command_topic_(nullptr)
    , command_reader_(nullptr)
    , command_type_(new DdsRecorderCommandPubSubType())
    , status_topic_name_(status_topic_name)
    , status_publisher_(nullptr)
    , status_topic_(nullptr)
    , status_writer_(nullptr)
    , status_type_(new DdsRecorderStatusPubSubType())
    , event_handler_(event_handler)
    , participant_configuration_(participant_configuration)
{
    registerDdsRecorderStatusTypes();
}

bool CommandReceiver::init()
{
    // CONFIGURE TRANSPORT
    // TODO: Create a utils method that returns a participant QoS object (or DomainParticipant directly) given a configuration structure.
    // This could be somewhere in dev-utils repo, or be a static method of DDS-Pipe's (RTPS/DDS) CommonParticipant class.
    DomainParticipantQos pqos;
    if (participant_configuration_->transport == ddspipe::core::types::TransportDescriptors::builtin)
    {
        if (!participant_configuration_->whitelist.empty())
        {
            // Disable builtin
            pqos.transport().use_builtin_transports = false;

            // Add Shared Memory Transport
            std::shared_ptr<eprosima::fastdds::rtps::SharedMemTransportDescriptor> shm_transport =
                    std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
            pqos.transport().user_transports.push_back(shm_transport);

            // Add UDP Transport
            std::shared_ptr<eprosima::fastdds::rtps::UDPv4TransportDescriptor> udp_transport =
                    ddspipe::participants::rtps::CommonParticipant::create_descriptor<eprosima::fastdds::rtps::UDPv4TransportDescriptor>(
                participant_configuration_->whitelist);
            pqos.transport().user_transports.push_back(udp_transport);
        }
    }
    else if (participant_configuration_->transport == ddspipe::core::types::TransportDescriptors::shm_only)
    {
        // Disable builtin
        pqos.transport().use_builtin_transports = false;

        // Add Shared Memory Transport
        std::shared_ptr<eprosima::fastdds::rtps::SharedMemTransportDescriptor> shm_transport =
                std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
        pqos.transport().user_transports.push_back(shm_transport);
    }
    else if (participant_configuration_->transport == ddspipe::core::types::TransportDescriptors::udp_only)
    {
        // Disable builtin
        pqos.transport().use_builtin_transports = false;

        // Add UDP Transport
        std::shared_ptr<eprosima::fastdds::rtps::UDPv4TransportDescriptor> udp_transport =
                ddspipe::participants::rtps::CommonParticipant::create_descriptor<eprosima::fastdds::rtps::UDPv4TransportDescriptor>(
            participant_configuration_->whitelist);
        pqos.transport().user_transports.push_back(udp_transport);
    }

    // Participant discovery filter configuration
    switch (participant_configuration_->ignore_participant_flags)
    {
        case ddspipe::core::types::IgnoreParticipantFlags::no_filter:
            pqos.wire_protocol().builtin.discovery_config.ignoreParticipantFlags =
                    eprosima::fastrtps::rtps::ParticipantFilteringFlags_t::NO_FILTER;
            break;
        case ddspipe::core::types::IgnoreParticipantFlags::filter_different_host:
            pqos.wire_protocol().builtin.discovery_config.ignoreParticipantFlags =
                    eprosima::fastrtps::rtps::ParticipantFilteringFlags_t::FILTER_DIFFERENT_HOST;
            break;
        case ddspipe::core::types::IgnoreParticipantFlags::filter_different_process:
            pqos.wire_protocol().builtin.discovery_config.ignoreParticipantFlags =
                    eprosima::fastrtps::rtps::ParticipantFilteringFlags_t::FILTER_DIFFERENT_PROCESS;
            break;
        case ddspipe::core::types::IgnoreParticipantFlags::filter_same_process:
            pqos.wire_protocol().builtin.discovery_config.ignoreParticipantFlags =
                    eprosima::fastrtps::rtps::ParticipantFilteringFlags_t::FILTER_SAME_PROCESS;
            break;
        case ddspipe::core::types::IgnoreParticipantFlags::filter_different_and_same_process:
            pqos.wire_protocol().builtin.discovery_config.ignoreParticipantFlags =
                    static_cast<eprosima::fastrtps::rtps::ParticipantFilteringFlags_t>(
                eprosima::fastrtps::rtps::ParticipantFilteringFlags_t::FILTER_DIFFERENT_PROCESS |
                eprosima::fastrtps::rtps::ParticipantFilteringFlags_t::FILTER_SAME_PROCESS);
            break;
        default:
            break;
    }

    // CREATE THE PARTICIPANT
    pqos.name("DdsRecorderCommandReceiver");
    
    // Set app properties
    pqos.properties().properties().emplace_back(
        "fastdds.application.id",
        participant_configuration_->app_id,
        "true");
    pqos.properties().properties().emplace_back(
        "fastdds.application.metadata",
        participant_configuration_->app_metadata,
        "true");

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
        command_topic_name_,
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
        status_topic_name_,
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

        case CommandCode::suspend:
            return "SUSPENDED";

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
} /* namespace recorder */
} /* namespace ddsrecorder */
} /* namespace eprosima */
