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

#include <fastrtps/types/TypeObjectFactory.h>

#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/utils.hpp>
#include <cpp_utils/ros2_mangling.hpp>

#include <ddspipe_core/types/dynamic_types/schema.hpp>

#if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    #include <fastcdr/Cdr.h>
    #include <fastcdr/FastBuffer.h>
    #include <fastcdr/FastCdr.h>
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v1/DynamicTypesCollection.hpp>
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v1/DynamicTypesCollectionPubSubTypes.hpp>
#else
    #include <fastdds/rtps/common/CdrSerialization.hpp>
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v2/DynamicTypesCollection.hpp>
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v2/DynamicTypesCollectionPubSubTypes.hpp>
#endif // if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13

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

    mcap_writer_.disable();
}

void McapHandler::add_schema(
        const fastrtps::types::DynamicType_ptr& dynamic_type)
{
    std::lock_guard<std::mutex> lock(mtx_);

    // NOTE: Process schemas even if in STOPPED state to avoid losing them (only sent/received once in discovery)

    assert(nullptr != dynamic_type);

    const std::string type_name = dynamic_type->get_name();

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
        data = idl::generate_idl_schema(dynamic_type);
    }

    mcap::Schema new_schema(name, encoding, data);

    // Add schema to writer and to schemas map
    logInfo(DDSRECORDER_MCAP_HANDLER,
            "MCAP_WRITE | Adding schema with name " << type_name << " :\n" << data << "\n");

    mcap_writer_.write(new_schema);

    logInfo(DDSRECORDER_MCAP_HANDLER,
            "MCAP_WRITE | Schema created: " << new_schema.name << ".");

    auto it = schemas_.find(type_name);
    if (it != schemas_.end())
    {
        // Update channels previously created with blank schema
        update_channels_nts_(it->second.id, new_schema.id);
    }
    schemas_[type_name] = std::move(new_schema);
    received_types_.insert(type_name);

    if (configuration_.record_types)
    {
        mcap_writer_.update_dynamic_types(*Serializer::serialize(&dynamic_types_));
    }

    // Check if there are any pending samples for this new schema. If so, dump them.
    if ((pending_samples_.find(type_name) != pending_samples_.end()) ||
            (state_ == BaseHandlerStateCode::PAUSED &&
            (pending_samples_paused_.find(type_name) != pending_samples_paused_.end())))
    {
        dump_pending_samples_nts_(type_name);
    }
}

void McapHandler::add_data(
        const DdsTopic& topic,
        RtpsPayloadData& data)
{
    std::unique_lock<std::mutex> lock(mtx_);

    if (state_ == BaseHandlerStateCode::STOPPED)
    {
        logInfo(DDSRECORDER_MCAP_HANDLER,
                "FAIL_MCAP_WRITE | Attempting to add sample through a stopped handler, dropping...");
        return;
    }

    logInfo(
        DDSRECORDER_MCAP_HANDLER,
        "MCAP_WRITE | Adding data in topic " << topic);

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

    const auto sample = new McapMessage(data, payload_pool_, topic, channel_id, configuration_.log_publishTime);

    if (received_types_.find(topic.type_name) != received_types_.end())
    {
        add_sample_to_buffer_nts_(sample);
        return;
    }

    switch (state_)
    {
        case BaseHandlerStateCode::RUNNING:

            if (configuration_.max_pending_samples != 0)
            {
                logInfo(
                    DDSRECORDER_MCAP_HANDLER,
                    "MCAP_WRITE | Schema for topic " << topic <<
                    " not yet available, inserting to pending samples queue.");

                add_sample_to_pending_nts_(sample);
            }
            else if (!configuration_.only_with_schema)
            {
                // No schema available + no pending samples -> Add to buffer with blank schema
                add_sample_to_buffer_nts_(sample);
            }
            break;

        case BaseHandlerStateCode::PAUSED:

            logInfo(
                DDSRECORDER_MCAP_HANDLER,
                "MCAP_WRITE | Schema for topic " << topic <<
                " not yet available, inserting to (paused) pending samples queue.");

            pending_samples_paused_[topic.type_name].push_back(sample);
            break;

        default:

            // Should not happen, protected with mutex and state verified at beginning
            utils::tsnh(utils::Formatter() << "Trying to add sample to a stopped instance.");
            break;
    }
}

void McapHandler::write_samples_(
        std::list<const BaseMessage*>& samples)
{
    logInfo(DDSRECORDER_MCAP_HANDLER, "Writing samples to MCAP file.");

    while (!samples.empty())
    {
        const auto mcap_sample = static_cast<const McapMessage*>(samples.front());

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

void McapHandler::store_dynamic_type_(
        const std::string& type_name,
        DynamicTypesCollection& dynamic_types) const
{
    const eprosima::fastrtps::types::TypeIdentifier* type_identifier = nullptr;
    const eprosima::fastrtps::types::TypeObject* type_object = nullptr;
    const eprosima::fastrtps::types::TypeInformation* type_information = nullptr;

    type_information =
            eprosima::fastrtps::types::TypeObjectFactory::get_instance()->get_type_information(type_name);
    if (type_information != nullptr)
    {
        auto dependencies = type_information->complete().dependent_typeids();
        std::string dependency_name;
        unsigned int dependency_index = 0;
        for (auto dependency: dependencies)
        {
            type_identifier = &dependency.type_id();
            type_object = eprosima::fastrtps::types::TypeObjectFactory::get_instance()->get_type_object(
                type_identifier);
            dependency_name = type_name + "_" + std::to_string(dependency_index);

            // Store dependency in dynamic_types collection
            store_dynamic_type_(type_identifier, type_object, dependency_name, dynamic_types);

            // Increment suffix counter
            dependency_index++;
        }
    }

    type_identifier = nullptr;
    type_object = nullptr;

    type_identifier = eprosima::fastrtps::types::TypeObjectFactory::get_instance()->get_type_identifier(type_name,
                    true);
    if (type_identifier)
    {
        type_object =
                eprosima::fastrtps::types::TypeObjectFactory::get_instance()->get_type_object(type_name, true);
    }

    // If complete not found, try with minimal
    if (!type_object)
    {
        type_identifier = eprosima::fastrtps::types::TypeObjectFactory::get_instance()->get_type_identifier(
            type_name, false);
        if (type_identifier)
        {
            type_object = eprosima::fastrtps::types::TypeObjectFactory::get_instance()->get_type_object(type_name,
                            false);
        }
    }

    // Store dynamic type in dynamic_types collection
    store_dynamic_type_(type_identifier, type_object, type_name, dynamic_types);
}

void McapHandler::store_dynamic_type_(
        const eprosima::fastrtps::types::TypeIdentifier* type_identifier,
        const eprosima::fastrtps::types::TypeObject* type_object,
        const std::string& type_name,
        DynamicTypesCollection& dynamic_types) const
{
    if (type_identifier != nullptr && type_object != nullptr)
    {
        DynamicType dynamic_type;
        dynamic_type.type_name(type_name);
        dynamic_type.type_information(utils::base64_encode(Serializer::serialize(*type_identifier)));
        dynamic_type.type_object(utils::base64_encode(Serializer::serialize(*type_object)));

        dynamic_types.dynamic_types().push_back(dynamic_type);
    }
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
