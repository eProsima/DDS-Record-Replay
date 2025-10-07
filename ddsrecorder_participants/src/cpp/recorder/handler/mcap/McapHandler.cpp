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

#include <string>

#include <mcap/reader.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/ros2_mangling.hpp>

#include <ddspipe_core/types/dynamic_types/schema.hpp>

#include <ddsrecorder_participants/common/serialize/Serializer.hpp>
#include <ddsrecorder_participants/constants.hpp>
#include <ddsrecorder_participants/recorder/handler/mcap/McapHandler.hpp>
#include <ddsrecorder_participants/recorder/message/McapMessage.hpp>

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
    , configuration_(config)
    , mcap_writer_(config.output_settings, config.mcap_writer_options, file_tracker, config.record_types)
{
    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
            "MCAP_STATE | Creating MCAP handler instance.");

    // Set the BaseHandler's writer
    writer_ = &mcap_writer_;

    // Initialize the BaseHandler
    init(init_state, on_disk_full_lambda);
}

McapHandler::~McapHandler()
{
    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
            "MCAP_STATE | Destroying MCAP handler.");

    // Stop handler prior to destruction
    stop(true);
}

void McapHandler::disable()
{
    // Ideally, the channels and schemas should be shared between the McapHandler and McapWriter.
    // Right now, the data is duplicated in both classes, which uses more memory and can lead to inconsistencies.
    // TODO: Share the channels and schemas between the McapHandler and McapWriter.

    // NOTE: disabling the BaseHandler disables the McapWriter which clears its channels
    BaseHandler::disable();

    // Clear the channels after a disable so the old channels are not rewritten in every new file
    channels_.clear();
}

void McapHandler::add_schema(
        const fastdds::dds::DynamicType::_ref_type& dynamic_type,
        const fastdds::dds::xtypes::TypeIdentifier& type_identifier)
{
    // NOTE: Process schemas even if in STOPPED state to avoid losing them (only sent/received once in discovery)
    std::lock_guard<std::mutex> lock(mtx_);

    if (dynamic_type == nullptr)
    {
        EPROSIMA_LOG_WARNING(DDSRECORDER_MCAP_HANDLER, "Received nullptr dynamic type. Skipping...");
        return;
    }

    const std::string type_name = dynamic_type->get_name().to_string();

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
        name = utils::demangle_if_ros_type(type_name);
        encoding = "ros2msg";
        data = msg::generate_ros2_schema(dynamic_type);
    }
    else
    {
        name = type_name;
        encoding = "omgidl";

        std::stringstream idl;
        auto ret = fastdds::dds::idl_serialize(dynamic_type, idl);
        if (ret != fastdds::dds::RETCODE_OK)
        {
            EPROSIMA_LOG_ERROR(
                DDSRECORDER_MCAP_HANDLER,
                "MCAP_WRITE | Failed to serialize DynamicType to idl for type with name: " << type_name);
            return;
        }
        data = idl.str();
    }

    mcap::Schema new_schema(name, encoding, data);
    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER, "Schema created: " << new_schema.name << ".");

    // Add schema to writer
    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
            "MCAP_WRITE | Adding schema with name " << type_name << " :\n" << data << "\n");

    mcap_writer_.write(new_schema);

    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
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
    received_types_[type_name] = dynamic_type;

    if (configuration_.record_types)
    {
        // Store dynamic type in dynamic_types collection
        if (store_dynamic_type_(type_name, type_identifier))
        {
            // Recalculate the attachment
            std::string dynamic_types_serialized;
            Serializer::serialize(dynamic_types_, dynamic_types_serialized);
            mcap_writer_.update_dynamic_types(dynamic_types_serialized);
        }
    }

    // Check if there are any pending samples for this new type. If so, dump them.
    dump_pending_samples_nts_(type_name);
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
        EPROSIMA_LOG_WARNING(DDSRECORDER_MCAP_HANDLER,
                "MCAP_WRITE | Error adding message in topic " << topic << ". Error message:\n " << e.what());
    }

    if (state_ != BaseHandlerStateCode::STOPPED)
    {
        const auto mcap_sample = std::make_shared<const McapMessage>(
            data, payload_pool_, topic, channel_id, configuration_.log_publishTime);

        process_new_sample_nts_(mcap_sample);

        const auto it_channel = channels_by_id_.find(channel_id);
        if(it_channel != channels_by_id_.end())
        {
            mcap::Channel channel = channels_by_id_[channel_id];

            std::ostringstream guid_ss;
            guid_ss << data.source_guid;

            std::string source_guid = guid_ss.str();
            uint32_t sequence_number = mcap_sample->number_of_msgs - 1;

            mcap_writer_.add_message_sourceguid(sequence_number, guid_ss.str());
        }
    }
}

void McapHandler::write_samples_(
        std::list<std::shared_ptr<const BaseMessage>>& samples)
{
    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER, "Writing samples to MCAP file.");

    while (!samples.empty())
    {
        const auto mcap_sample = static_cast<const McapMessage*>(samples.front().get());

        if (mcap_sample == nullptr)
        {
            EPROSIMA_LOG_WARNING(DDSRECORDER_MCAP_HANDLER, "Error downcasting sample to McapMessage. Skipping...");
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
            EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
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
    Serializer::serialize(topic.topic_qos, metadata[QOS_SERIALIZATION_QOS]);

    const auto topic_name =
            configuration_.ros2_types ? utils::demangle_if_ros_topic(topic.m_topic_name) : topic.m_topic_name;
    const auto is_topic_ros2_type = configuration_.ros2_types && topic_name != topic.m_topic_name;

    metadata[ROS2_TYPES] = is_topic_ros2_type ? "true" : "false";

    std::string topic_partitions = "";
    for(const auto& pair: topic.partition_name)
    {
        topic_partitions += pair.first + ":" + pair.second + ";";
    }

    metadata[PARTITIONS] = topic_partitions;
    mcap::Channel new_channel(topic_name, "cdr", schema_id, metadata);

    mcap_writer_.write(new_channel);

    auto channel_id = new_channel.id;
    channels_.insert({topic, std::move(new_channel)});
    channels_by_id_.insert({channel_id, std::move(new_channel)});
    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
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
            EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
                    "MCAP_WRITE | Updating channel in topic " << channel.first.m_topic_name << ".");

            assert(utils::demangle_if_ros_topic(channel.first.m_topic_name) == channel.second.topic);
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
