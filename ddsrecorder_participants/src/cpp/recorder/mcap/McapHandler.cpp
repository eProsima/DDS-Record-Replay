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
 * @file McapHandler.cpp
 */

#define MCAP_IMPLEMENTATION  // Define this in exactly one .cpp file

#include <algorithm>

#include <mcap/reader.hpp>
#include <yaml-cpp/yaml.h>

#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/utils.hpp>
#include <cpp_utils/ros2_mangling.hpp>

#include <ddspipe_core/types/dynamic_types/schema.hpp>

#include <ddsrecorder_participants/constants.hpp>
#include <ddsrecorder_participants/recorder/mcap/McapHandler.hpp>
#include <ddsrecorder_participants/recorder/message/McapMessage.hpp>
#include <ddsrecorder_participants/recorder/mcap/utils.hpp>
#include <ddsrecorder_participants/recorder/output/Serializer.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

using namespace eprosima::ddspipe::core::types;

McapHandler::McapHandler(
        const McapHandlerConfiguration& config,
        const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
        std::shared_ptr<ddsrecorder::participants::FileTracker> file_tracker,
        const BaseHandlerStateCode& init_state /* = BaseHandlerStateCode::RUNNING */,
        const std::function<void()>& on_disk_full_lambda /* = nullptr */)
    : BaseHandler(config, payload_pool)
    , mcap_writer_(config.output_settings, config.mcap_writer_options, file_tracker, config.record_types)
{
    logInfo(DDSRECORDER_MCAP_HANDLER,
            "MCAP_STATE | Creating MCAP handler instance.");

    if (on_disk_full_lambda != nullptr)
    {
        mcap_writer_.set_on_disk_full_callback(on_disk_full_lambda);
    }

    init(init_state);
}

McapHandler::~McapHandler()
{
    logInfo(DDSRECORDER_MCAP_HANDLER,
            "MCAP_STATE | Destroying MCAP handler.");

    // Stop handler prior to destruction
    stop(true);
}

void McapHandler::enable()
{
    logInfo(DDSRECORDER_MCAP_HANDLER, "Enabling MCAP handler.");

    mcap_writer_.enable();
}

void McapHandler::disable()
{
    logInfo(DDSRECORDER_MCAP_HANDLER, "Disabling MCAP handler.");

    // Ideally, the channels and schemas should be shared between the McapHandler and McapWriter.
    // Right now, the data is duplicated in both classes, which uses more memory and can lead to inconsistencies.
    // TODO: Share the channels and schemas between the McapHandler and McapWriter.

    // NOTE: disabling the McapWriter clears its channels
    mcap_writer_.disable();

    // Clear the channels after a disable so the old channels are not rewritten in every new file
    channels_.clear();
}

void McapHandler::add_schema(
        const fastdds::dds::DynamicType::_ref_type& dynamic_type,
        const std::string& type_name,
        const fastdds::dds::xtypes::TypeIdentifier& type_id)
{
    // NOTE: Process schemas even if in STOPPED state to avoid losing them (only sent/received once in discovery)
    std::lock_guard<std::mutex> lock(mtx_);

    if (dynamic_type == nullptr)
    {
        logWarning(DDSRECORDER_MCAP_HANDLER, "Received nullptr dynamic type. Skipping...");
        return;
    }

    // const std::string type_name = dynamic_type->get_name();

    // Check if it exists already
    if (received_types_.find(type_name) != received_types_.end())
    {
        return;
    }

    // Create the MCAP schema
    std::string name;
    std::string encoding;
    std::string data;

    if (configuration_.ros2_types)
    {
        // NOTE: Currently ROS2 types are not supported, change when supported
        name = utils::demangle_if_ros_type(type_name);
        encoding = "ros2msg";
        data = msg::generate_ros2_schema(dynamic_type);
    }
    else
    {
        name = type_name;
        encoding = "omgidl";

        std::stringstream idl;
        auto ret = idl_serialize(dynamic_type, idl);
        if (ret != fastdds::dds::RETCODE_OK)
        {
            logError(DDSRECORDER_MCAP_HANDLER, "MCAP_WRITE | Failed to serialize DynamicType to idl for type wth name: " << dynamic_type->get_name().to_string());
            return;
        }
        data = idl.str();
    }

    mcap::Schema new_schema(name, encoding, data);
    logInfo(DDSRECORDER_MCAP_HANDLER, "Schema created: " << new_schema.name << ".");

    // Add schema to writer
    logInfo(DDSRECORDER_MCAP_HANDLER,
            "MCAP_WRITE | Adding schema with name " << dynamic_type->get_name().to_string() << " :\n" << data << "\n");

    mcap_writer_.write(new_schema);

    logInfo(DDSRECORDER_MCAP_HANDLER,
            "MCAP_WRITE | Schema created: " << new_schema.name << ".");

    // Update channels previously created with blank schema
    const auto it = schemas_.find(type_name);
    if (it != schemas_.end())
    {
        update_channels_nts_(it->second.id, new_schema.id);
    }

    // Store schema
    schemas_[type_name] = std::move(new_schema);

    // Add type to the list of received types
    received_types_.insert(type_name);

    if (configuration_.record_types)
    {
        mcap_writer_.update_dynamic_types(*Serializer::serialize(&dynamic_types_));
    }

    // Check if there are any pending samples for this new type. If so, dump them.
    if (pending_samples_.find(type_name) != pending_samples_.end() ||
            (state_ == BaseHandlerStateCode::PAUSED &&
            pending_samples_paused_.find(type_name) != pending_samples_paused_.end()))
    {
        dump_pending_samples_nts_(type_name);
    }
}

void McapHandler::add_data(
        const DdsTopic& topic,
        RtpsPayloadData& data)
{
    std::unique_lock<std::mutex> lock(mtx_);

    // Add channel to data
    mcap::ChannelId channel_id;

    try
    {
        channel_id = get_channel_id_nts_(topic);
    }
    catch (const utils::InconsistencyException& e)
    {
        logWarning(DDSRECORDER_MCAP_HANDLER,
                "MCAP_WRITE | Error adding message in topic " << topic << ". Error message:\n " << e.what());
    }

    process_new_sample_nts_(std::make_shared<const McapMessage>(
            data, payload_pool_, topic, channel_id, configuration_.log_publishTime));
}

void McapHandler::write_samples_(
        std::list<std::shared_ptr<const BaseMessage>>& samples)
{
    logInfo(DDSRECORDER_MCAP_HANDLER, "Writing samples to MCAP file.");

    while (!samples.empty())
    {
        const auto mcap_sample = static_cast<const McapMessage*>(samples.front().get());

        if (mcap_sample == nullptr)
        {
            logWarning(DDSRECORDER_MCAP_HANDLER, "Error downcasting sample to McapMessage. Skipping...");
            continue;
        }

        mcap_writer_.write(*mcap_sample);

        samples.pop_front();
    }
}

mcap::ChannelId McapHandler::create_channel_id_nts_(
        const DdsTopic& topic)
{
    // Find schema
    mcap::SchemaId schema_id;
    try
    {
        schema_id = get_schema_id_nts_(topic.type_name);
    }
    catch (const utils::InconsistencyException&)
    {
        if (!configuration_.only_with_schema)
        {
            logInfo(DDSRECORDER_MCAP_HANDLER,
                    "MCAP_WRITE | Schema not found for type: " << topic.type_name << ". Creating blank schema...");

            std::string encoding = configuration_.ros2_types ? "ros2msg" : "omgidl";
            mcap::Schema blank_schema(topic.type_name, encoding, "");

            mcap_writer_.write(blank_schema);

            schemas_.insert({topic.type_name, std::move(blank_schema)});

            schema_id = blank_schema.id;
        }
        else
        {
            // Propagate exception
            throw;
        }
    }

    // Create new channel
    mcap::KeyValueMap metadata = {};
    metadata[QOS_SERIALIZATION_QOS] = Serializer::serialize(topic.topic_qos);
    std::string topic_name =
            configuration_.ros2_types ? utils::demangle_if_ros_topic(topic.m_topic_name) : topic.m_topic_name;
    // Set ROS2_TYPES to "false" if the given topic_name is equal to topic.m_topic_name, otherwise set it to "true".
    metadata[ROS2_TYPES] = topic_name.compare(topic.m_topic_name) ? "true" : "false";
    mcap::Channel new_channel(topic_name, "cdr", schema_id, metadata);

    mcap_writer_.write(new_channel);

    auto channel_id = new_channel.id;
    channels_.insert({topic, std::move(new_channel)});
    logInfo(DDSRECORDER_MCAP_HANDLER,
            "MCAP_WRITE | Channel created: " << topic << ".");

    return channel_id;
}

mcap::ChannelId McapHandler::get_channel_id_nts_(
        const DdsTopic& topic)
{
    auto it = channels_.find(topic);
    if (it != channels_.end())
    {
        return it->second.id;
    }

    // If it does not exist yet, create it (call it with mutex taken)
    return create_channel_id_nts_(topic);
}

void McapHandler::update_channels_nts_(
        const mcap::SchemaId& old_schema_id,
        const mcap::SchemaId& new_schema_id)
{
    for (auto& channel : channels_)
    {
        if (channel.second.schemaId == old_schema_id)
        {
            logInfo(DDSRECORDER_MCAP_HANDLER,
                    "MCAP_WRITE | Updating channel in topic " << channel.first.m_topic_name << ".");

            assert(channel.first.m_topic_name == channel.second.topic);
            mcap::Channel new_channel(channel.second.topic, "cdr", new_schema_id, channel.second.metadata);

            mcap_writer_.write(new_channel);

            channel.second = std::move(new_channel);
        }
    }
}

mcap::SchemaId McapHandler::get_schema_id_nts_(
        const std::string& schema_name)
{
    auto it = schemas_.find(schema_name);
    if (it != schemas_.end())
    {
        return it->second.id;
    }
    else
    {
        throw utils::InconsistencyException(
                  STR_ENTRY << "Schema " << schema_name << " is not registered.");
    }
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
