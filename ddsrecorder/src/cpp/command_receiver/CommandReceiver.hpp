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

#include <mutex>
#include <queue>
#include <string>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>

#include <cpp_utils/event/MultipleEventHandler.hpp>
#include <cpp_utils/macros/custom_enumeration.hpp>

#include <ddspipe_participants/configuration/SimpleParticipantConfiguration.hpp>

#include "types/DdsRecorderCommand/DdsRecorderCommandPubSubTypes.hpp"
#include "types/DdsRecorderCommand/DdsRecorderCommandTypeObjectSupport.hpp"
#include "types/DdsRecorderStatus/DdsRecorderStatusPubSubTypes.hpp"
#include "types/DdsRecorderStatus/DdsRecorderStatusTypeObjectSupport.hpp"

namespace eprosima {
namespace ddsrecorder {
namespace recorder {
namespace receiver {

ENUMERATION_BUILDER(
    CommandCode,
    start,
    pause,
    event,
    suspend,
    stop,
    close,
    unknown
    );

class CommandReceiver : public eprosima::fastdds::dds::DataReaderListener
{
public:

    CommandReceiver(
            uint32_t domain,
            const std::string& command_topic_name,
            const std::string& status_topic_name,
            std::shared_ptr<utils::event::MultipleEventHandler> event_handler,
            std::shared_ptr<ddspipe::participants::SimpleParticipantConfiguration> participant_configuration);

    virtual ~CommandReceiver();

    bool init();

    DdsRecorderCommand wait_for_command();

    void publish_status(
            CommandCode current,
            CommandCode previous,
            std::string info = "");

    void on_data_available(
            fastdds::dds::DataReader* reader) override;

    void on_subscription_matched(
            fastdds::dds::DataReader* reader,
            const fastdds::dds::SubscriptionMatchedStatus& info) override;

private:

    static std::string command_to_status_string_(
            const CommandCode& command);

    std::mutex mtx_;
    std::queue<DdsRecorderCommand> commands_received_;

    // DDS related attributes
    uint32_t domain_;
    fastdds::dds::DomainParticipant* participant_;

    // Command attributes
    std::string command_topic_name_;
    fastdds::dds::Subscriber* command_subscriber_;
    fastdds::dds::Topic* command_topic_;
    fastdds::dds::DataReader* command_reader_;
    fastdds::dds::TypeSupport command_type_;

    // Status attributes
    std::string status_topic_name_;
    fastdds::dds::Publisher* status_publisher_;
    fastdds::dds::Topic* status_topic_;
    fastdds::dds::DataWriter* status_writer_;
    fastdds::dds::TypeSupport status_type_;

    std::shared_ptr<utils::event::MultipleEventHandler> event_handler_;

    std::shared_ptr<ddspipe::participants::SimpleParticipantConfiguration> participant_configuration_;
};

} /* namespace receiver */
} /* namespace recorder */
} /* namespace ddsrecorder */
} /* namespace eprosima */
