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
#include <cstdio>
#include <filesystem>
#include <vector>

#include <mcap/reader.hpp>

#include <yaml-cpp/yaml.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/xtypes/utils.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/common/CdrSerialization.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <fastdds/rtps/common/Types.hpp>

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/utils.hpp>
#include <cpp_utils/ros2_mangling.hpp>

#include <ddspipe_core/types/dynamic_types/schema.hpp>

#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollection.hpp>
#include <ddsrecorder_participants/common/types/dynamic_types_collection/DynamicTypesCollectionPubSubTypes.hpp>
#include <ddsrecorder_participants/constants.hpp>
#include <ddsrecorder_participants/recorder/mcap/McapHandler.hpp>
#include <ddsrecorder_participants/recorder/mcap/McapMessage.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

using namespace eprosima::ddspipe::core::types;

McapHandler::McapHandler(
        const McapHandlerConfiguration& config,
        const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
        std::shared_ptr<ddsrecorder::participants::FileTracker> file_tracker,
        const McapHandlerStateCode& init_state /* = McapHandlerStateCode::RUNNING */,
        const std::function<void()>& on_disk_full_lambda /* = nullptr */)
    : configuration_(config)
    , payload_pool_(payload_pool)
    , state_(McapHandlerStateCode::STOPPED)
    , mcap_writer_(config.output_settings, config.mcap_writer_options, file_tracker, config.record_types)
{
    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
            "MCAP_STATE | Creating MCAP handler instance.");

    if (on_disk_full_lambda != nullptr)
    {
        mcap_writer_.set_on_disk_full_callback(on_disk_full_lambda);
    }

    switch (init_state)
    {
        case McapHandlerStateCode::RUNNING:
            start();
            break;

        case McapHandlerStateCode::PAUSED:
            pause();
            break;

        default:
            break;
    }
}

McapHandler::~McapHandler()
{
    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
            "MCAP_STATE | Destroying handler.");

    // Stop handler prior to destruction
    stop(true);
}

void McapHandler::add_schema(
        const fastdds::dds::DynamicType::_ref_type& dynamic_type,
        const fastdds::dds::xtypes::TypeIdentifier& type_identifier)
{
    std::lock_guard<std::mutex> lock(mtx_);

    // NOTE: Process schemas even if in STOPPED state to avoid losing them (only sent/received once in discovery)

    assert(nullptr != dynamic_type);

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
            EPROSIMA_LOG_ERROR(DDSRECORDER_MCAP_HANDLER, "MCAP_WRITE | Failed to serialize DynamicType to idl for type wth name: " << type_name);
            return;
        }
        data = idl.str();
    }

    mcap::Schema new_schema(name, encoding, data);

    // Add schema to writer and to schemas map
    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
            "MCAP_WRITE | Adding schema with name " << type_name << " :\n" << data << "\n");

    mcap_writer_.write(new_schema);

    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
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
        // Store dynamic type in dynamic_types collection
        store_dynamic_type_(type_name, type_identifier, dynamic_types_);

        // Serialize dynamic types collection
        const auto serialized_dynamic_types = serialize_dynamic_types_(dynamic_types_);

        // Recalculate the attachment
        mcap_writer_.update_dynamic_types(*serialized_dynamic_types);
    }

    // Check if there are any pending samples for this new schema. If so, add them.
    if ((pending_samples_.find(type_name) != pending_samples_.end()) ||
            (state_ == McapHandlerStateCode::PAUSED &&
            (pending_samples_paused_.find(type_name) != pending_samples_paused_.end())))
    {
        add_pending_samples_nts_(type_name);
    }
}

void McapHandler::add_data(
        const DdsTopic& topic,
        RtpsPayloadData& data)
{
    std::unique_lock<std::mutex> lock(mtx_);

    if (state_ == McapHandlerStateCode::STOPPED)
    {
        EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
                "FAIL_MCAP_WRITE | Attempting to add sample through a stopped handler, dropping...");
        return;
    }

    EPROSIMA_LOG_INFO(
        DDSRECORDER_MCAP_HANDLER,
        "MCAP_WRITE | Adding data in topic " << topic);

    // Add data to channel
    McapMessage msg;
    msg.sequence = unique_sequence_number_++;
    msg.publishTime = fastdds_timestamp_to_mcap_timestamp(data.source_timestamp);
    if (configuration_.log_publishTime)
    {
        msg.logTime = msg.publishTime;
    }
    else
    {
        msg.logTime = now();
    }
    msg.dataSize = data.payload.length;

    if (data.payload.length > 0)
    {
        if (data.payload_owner != nullptr)
        {
            payload_pool_->get_payload(
                data.payload,
                msg.payload);

            msg.payload_owner = payload_pool_.get();
            msg.data = reinterpret_cast<std::byte*>(msg.payload.data);
        }
        else
        {
            throw utils::InconsistencyException(
                      STR_ENTRY << "Payload owner not found in data received."
                      );
        }
    }
    else
    {
        throw utils::InconsistencyException(
                  STR_ENTRY << "Received sample with no payload."
                  );
    }

    if (received_types_.count(topic.type_name) != 0)
    {
        // Schema available -> add to buffer
        add_data_nts_(msg, topic);
    }
    else
    {
        if (state_ == McapHandlerStateCode::RUNNING)
        {
            if (configuration_.max_pending_samples == 0)
            {
                if (configuration_.only_with_schema)
                {
                    // No schema available + no pending samples + only_with_schema -> Discard message
                    return;
                }
                else
                {
                    // No schema available + no pending samples -> Add to buffer with blank schema
                    add_data_nts_(msg, topic);
                }
            }
            else
            {
                EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
                        "MCAP_WRITE | Schema for topic " << topic << " not yet available. "
                        "Inserting to pending samples queue.");

                add_to_pending_nts_(msg, topic);
            }
        }
        else if (state_ == McapHandlerStateCode::PAUSED)
        {
            EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
                    "MCAP_WRITE | Schema for topic " << topic << " not yet available. "
                    "Inserting to (paused) pending samples queue.");

            pending_samples_paused_[topic.type_name].push_back({topic, msg});
        }
        else
        {
            // Should not happen, protected with mutex and state verified at beginning
            utils::tsnh(
                utils::Formatter() << "Trying to add sample from a stopped instance.");
        }
    }
}

void McapHandler::start()
{
    // Wait for completion of event routine in case event was triggered
    std::unique_lock<std::mutex> event_lock(event_cv_mutex_);
    event_cv_.wait(
        event_lock,
        [&]
        {
            return event_flag_ != EventCode::triggered;
        });

    // Protect access to state and data structures (cleared in stop_event_thread_nts)
    std::lock_guard<std::mutex> lock(mtx_);

    // Store previous state to act differently depending on its value
    McapHandlerStateCode prev_state = state_;
    state_ = McapHandlerStateCode::RUNNING;

    if (prev_state == McapHandlerStateCode::RUNNING)
    {
        EPROSIMA_LOG_WARNING(DDSRECORDER_MCAP_HANDLER,
                "MCAP_STATE | Ignoring start command, instance already started.");
    }
    else
    {
        EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
                "MCAP_STATE | Starting handler.");

        if (prev_state == McapHandlerStateCode::STOPPED)
        {
            mcap_writer_.enable();
        }
        else if (prev_state == McapHandlerStateCode::PAUSED)
        {
            // Stop event routine (cleans buffers)
            stop_event_thread_nts_(event_lock);
        }
    }
}

void McapHandler::stop(
        bool on_destruction /* false */)
{
    // Wait for completion of event routine in case event was triggered
    std::unique_lock<std::mutex> event_lock(event_cv_mutex_);
    event_cv_.wait(
        event_lock,
        [&]
        {
            return event_flag_ != EventCode::triggered;
        });

    // Protect access to state and data structures
    std::lock_guard<std::mutex> lock(mtx_);

    // Store previous state to act differently depending on its value
    McapHandlerStateCode prev_state = state_;
    state_ = McapHandlerStateCode::STOPPED;
    if (prev_state == McapHandlerStateCode::STOPPED)
    {
        if (!on_destruction)
        {
            EPROSIMA_LOG_WARNING(DDSRECORDER_MCAP_HANDLER,
                    "MCAP_STATE | Ignoring stop command, instance already stopped.");
        }

        return;
    }

    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
            "MCAP_STATE | Stopping handler.");

    if (prev_state == McapHandlerStateCode::PAUSED)
    {
        // Stop event routine (cleans buffers)
        stop_event_thread_nts_(event_lock);
    }

    if (!configuration_.only_with_schema)
    {
        // Adds to buffer samples whose schema was not received while running
        add_pending_samples_nts_();
    }
    else
    {
        // Free memory resources
        pending_samples_.clear();
    }
    dump_data_nts_();  // if prev_state == RUNNING -> writes buffer + added pending samples (if !only_with_schema)
                       // if prev_state == PAUSED  -> writes added pending samples (if !only_with_schema)

    // Ideally, the channels and schemas should be shared between the McapHandler and McapWriter.
    // Right now, the data is duplicated in both classes, which uses more memory and can lead to inconsistencies.
    // TODO: Share the channels and schemas between the McapHandler and McapWriter.

    // NOTE: disabling the McapWriter clears its channels
    mcap_writer_.disable();

    // Clear the channels after a stop so the old channels are not rewritten in every new file
    channels_.clear();
}

void McapHandler::pause()
{
    // Protect access to state and data structures
    std::lock_guard<std::mutex> lock(mtx_);

    // NOTE: no need to take event mutex as event thread does not exist at this point

    // Store previous state to act differently depending on its value
    McapHandlerStateCode prev_state = state_;
    state_ = McapHandlerStateCode::PAUSED;

    if (prev_state == McapHandlerStateCode::PAUSED)
    {
        EPROSIMA_LOG_WARNING(DDSRECORDER_MCAP_HANDLER,
                "MCAP_STATE | Ignoring pause command, instance already paused.");
    }
    else
    {
        EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
                "MCAP_STATE | Pausing handler.");

        if (prev_state == McapHandlerStateCode::STOPPED)
        {
            mcap_writer_.enable();
        }
        else if (prev_state == McapHandlerStateCode::RUNNING)
        {
            // Write data stored in buffer
            dump_data_nts_();
            // Clear buffer
            // NOTE: not really needed, dump_data_nts_ already writes and pops all samples
            samples_buffer_.clear();
        }

        // Launch event thread routine
        event_flag_ = EventCode::untriggered;  // No need to take event mutex (protected by mtx_)
        event_thread_ = std::thread(&McapHandler::event_thread_routine_, this);
    }
}

void McapHandler::trigger_event()
{
    // Wait for completion of event routine in case event was triggered
    std::unique_lock<std::mutex> event_lock(event_cv_mutex_);
    event_cv_.wait(
        event_lock,
        [&]
        {
            return event_flag_ != EventCode::triggered;
        });

    // Protect access to state
    std::lock_guard<std::mutex> lock(mtx_);

    if (state_ != McapHandlerStateCode::PAUSED)
    {
        EPROSIMA_LOG_WARNING(DDSRECORDER_MCAP_HANDLER,
                "MCAP_STATE | Ignoring trigger event command, instance is not paused.");
    }
    else
    {
        EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
                "MCAP_STATE | Triggering event.");

        // Notify event routine thread an event has been triggered
        event_flag_ = EventCode::triggered;
        event_lock.unlock(); // Unlock before notifying for efficiency purposes
        event_cv_.notify_all(); // Need to notify all as not possible to notify a specific thread
    }
}

mcap::Timestamp McapHandler::fastdds_timestamp_to_mcap_timestamp(
        const DataTime& time)
{
    std::uint64_t mcap_time = time.seconds();
    mcap_time *= 1000000000;
    return mcap_time + time.nanosec();
}

mcap::Timestamp McapHandler::std_timepoint_to_mcap_timestamp(
        const utils::Timestamp& time)
{
    return mcap::Timestamp(std::chrono::duration_cast<std::chrono::nanoseconds>(time.time_since_epoch()).count());
}

mcap::Timestamp McapHandler::now()
{
    return std_timepoint_to_mcap_timestamp(utils::now());
}

void McapHandler::add_data_nts_(
        const McapMessage& msg,
        bool direct_write /* false */)
{
    if (direct_write)
    {
        // Write to MCAP file
        mcap_writer_.write(msg);
    }
    else
    {
        samples_buffer_.push_back(msg);

        if (state_ == McapHandlerStateCode::RUNNING && samples_buffer_.size() == configuration_.buffer_size)
        {
            EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
                    "MCAP_WRITE | Full buffer, writing to disk...");
            dump_data_nts_();
        }
    }
}

void McapHandler::add_data_nts_(
        McapMessage& msg,
        const DdsTopic& topic,
        bool direct_write /* false */)
{
    try
    {
        auto channel_id = get_channel_id_nts_(topic);
        msg.channelId = channel_id;
    }
    catch (const utils::InconsistencyException& e)
    {
        EPROSIMA_LOG_WARNING(DDSRECORDER_MCAP_HANDLER,
                "MCAP_WRITE | Error adding message in topic " << topic << ". Error message:\n " << e.what());
        return;
    }

    add_data_nts_(msg, direct_write);
}

void McapHandler::add_to_pending_nts_(
        McapMessage& msg,
        const DdsTopic& topic)
{
    assert(configuration_.max_pending_samples != 0);

    if (configuration_.max_pending_samples > 0 &&
            pending_samples_[topic.type_name].size() == static_cast<unsigned int>(configuration_.max_pending_samples))
    {
        if (configuration_.only_with_schema)
        {
            // Discard oldest message in pending samples
            EPROSIMA_LOG_WARNING(DDSRECORDER_MCAP_HANDLER,
                    "MCAP_WRITE | Dropping pending sample in type " << topic.type_name << ": buffer limit (" <<
                    configuration_.max_pending_samples << ") reached.");
        }
        else
        {
            EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
                    "MCAP_WRITE | Buffer limit (" << configuration_.max_pending_samples <<  ") reached for type " <<
                    topic.type_name << ": writing oldest sample without schema.");

            // Write oldest message without schema
            auto& oldest_sample = pending_samples_[topic.type_name].front();
            add_data_nts_(oldest_sample.second, oldest_sample.first);
        }

        pending_samples_[topic.type_name].pop_front();
    }

    pending_samples_[topic.type_name].push_back({topic, msg});
}

void McapHandler::add_pending_samples_nts_(
        const std::string& schema_name)
{
    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
            "MCAP_WRITE | Adding pending samples for type: " << schema_name << ".");
    if (pending_samples_.find(schema_name) != pending_samples_.end())
    {
        // Move samples from pending_samples to buffer
        // NOTE: directly write to MCAP when PAUSED, as these correspond to samples that were previously received in
        // RUNNING state (hence to avoid being cleaned by event thread)
        add_pending_samples_nts_(pending_samples_[schema_name], state_ == McapHandlerStateCode::PAUSED);
        pending_samples_.erase(schema_name);
    }
    if (state_ == McapHandlerStateCode::PAUSED &&
            (pending_samples_paused_.find(schema_name) != pending_samples_paused_.end()))
    {
        // Move samples from pending_samples_paused to buffer, from where they will be written if event received
        add_pending_samples_nts_(pending_samples_paused_[schema_name]);
        pending_samples_paused_.erase(schema_name);
    }
}

void McapHandler::add_pending_samples_nts_(
        pending_list& pending_samples,
        bool direct_write /* false */)
{
    while (!pending_samples.empty())
    {
        // Move samples from pending list to buffer, or write them directly to MCAP file
        auto& sample = pending_samples.front();
        add_data_nts_(sample.second, sample.first, direct_write);

        pending_samples.pop_front();
    }
}

void McapHandler::add_pending_samples_nts_()
{
    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
            "MCAP_WRITE | Adding pending samples for all types.");

    auto pending_types = utils::get_keys(pending_samples_);
    for (auto& pending_type: pending_types)
    {
        add_pending_samples_nts_(pending_type);
    }
}

void McapHandler::event_thread_routine_()
{
    bool keep_going = true;
    while (keep_going)
    {
        bool timeout;
        auto exit_time = std::chrono::time_point<std::chrono::system_clock>::max();
        auto cleanup_period_ = std::chrono::seconds(configuration_.cleanup_period);
        if (cleanup_period_ < std::chrono::seconds::max())
        {
            auto now = std::chrono::system_clock::now();
            exit_time = now + cleanup_period_;
        }

        std::unique_lock<std::mutex> event_lock(event_cv_mutex_);

        if (event_flag_ != EventCode::untriggered)
        {
            // Flag set before taking mutex, no need to wait
            timeout = false;
        }
        else
        {
            timeout = !event_cv_.wait_until(
                event_lock,
                exit_time,
                [&]
                {
                    return event_flag_ != EventCode::untriggered;
                });
        }

        if (event_flag_ == EventCode::stopped)
        {
            EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
                    "MCAP_STATE | Finishing event thread routine.");
            keep_going = false;
        }
        else
        {
            // Protect access to state and data structures
            std::lock_guard<std::mutex> lock(mtx_);

            // NOTE: event mutex not released until routine completed to avoid other commands (start/stop/trigger) to interfere.

            // Delete outdated samples if timeout, and also before dumping (event triggered case)
            remove_outdated_samples_nts_();

            if (timeout)
            {
                EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
                        "MCAP_STATE | Event thread timeout.");
            }
            else
            {
                EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
                        "MCAP_STATE | Event triggered: dumping buffered data.");

                if (!(configuration_.max_pending_samples == 0 && configuration_.only_with_schema))
                {
                    // Move (paused) pending samples to buffer (or pending samples) prior to dumping
                    for (auto& pending_type : pending_samples_paused_)
                    {
                        auto type_name = pending_type.first;
                        auto& pending_list = pending_type.second;
                        while (!pending_list.empty())
                        {
                            auto& sample = pending_list.front();
                            if (configuration_.max_pending_samples == 0)
                            {
                                if (configuration_.only_with_schema)
                                {
                                    // Cannot happen (outter if)
                                }
                                else
                                {
                                    // Add to buffer with blank schema
                                    add_data_nts_(sample.second, sample.first);
                                }
                            }
                            else
                            {
                                add_to_pending_nts_(sample.second, sample.first);
                            }
                            pending_list.pop_front();
                        }
                    }
                }
                dump_data_nts_();
            }

            // Event routine iteration completed: reset and wait for next event
            event_flag_ = EventCode::untriggered;
        }

        // Notify threads waiting for this resource
        event_lock.unlock();
        event_cv_.notify_all();
    }
}

void McapHandler::remove_outdated_samples_nts_()
{
    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
            "MCAP_STATE | Removing outdated samples.");

    auto threshold = std_timepoint_to_mcap_timestamp(utils::now() - std::chrono::seconds(configuration_.event_window));
    samples_buffer_.remove_if([&](auto& sample)
            {
                return sample.logTime < threshold;
            });

    for (auto& pending_type : pending_samples_paused_)
    {
        pending_type.second.remove_if([&](auto& sample)
                {
                    return sample.second.logTime < threshold;
                });
    }
}

void McapHandler::stop_event_thread_nts_(
        std::unique_lock<std::mutex>& event_lock)
{
    // NOTE: this method assumes both mtx_ and event_cv_mutex_ (within event_lock) are locked

    // WARNING: state must have been set different to PAUSED before calling this method
    assert(state_ != McapHandlerStateCode::PAUSED);

    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
            "MCAP_STATE | Stopping event thread.");

    if (event_thread_.joinable())
    {
        event_flag_ = EventCode::stopped;
        event_lock.unlock(); // Unlock prior to notification (for efficiency) and join (to avoid deadlock)
        event_cv_.notify_all(); // Need to notify all as not possible to notify a specific thread
        event_thread_.join();
    }

    samples_buffer_.clear();
    pending_samples_paused_.clear();
}

void McapHandler::dump_data_nts_()
{
    EPROSIMA_LOG_INFO(DDSRECORDER_MCAP_HANDLER,
            "MCAP_WRITE | Writing data stored in buffer.");

    while (!samples_buffer_.empty())
    {
        auto& sample = samples_buffer_.front();

        // Write to MCAP file
        mcap_writer_.write(sample);

        // Pop written sample
        samples_buffer_.pop_front();
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
    metadata[QOS_SERIALIZATION_QOS] = serialize_qos_(topic.topic_qos);
    std::string topic_name =
            configuration_.ros2_types ? utils::demangle_if_ros_topic(topic.m_topic_name) : topic.m_topic_name;
    // Set ROS2_TYPES to "false" if the given topic_name is equal to topic.m_topic_name, otherwise set it to "true".
    metadata[ROS2_TYPES] = topic_name.compare(topic.m_topic_name) ? "true" : "false";
    mcap::Channel new_channel(topic_name, "cdr", schema_id, metadata);

    mcap_writer_.write(new_channel);

    auto channel_id = new_channel.id;
    channels_.insert({topic, std::move(new_channel)});
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
        const fastdds::dds::xtypes::TypeIdentifier& type_identifier,
        DynamicTypesCollection& dynamic_types) const
{
    fastdds::dds::xtypes::TypeIdentifierPair type_identifiers;
    type_identifiers.type_identifier1(type_identifier);

    fastdds::dds::xtypes::TypeInformation type_info;
    if (fastdds::dds::RETCODE_OK == fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_information(
            type_identifiers,
            type_info,
            true))
    {
        std::string dependency_name;
        unsigned int dependency_index = 0;
        auto type_dependencies = type_info.complete().dependent_typeids();
        for (auto dependency : type_dependencies)
        {
            fastdds::dds::xtypes::TypeIdentifier dependency_type_identifier;
            dependency_type_identifier = dependency.type_id();

            fastdds::dds::xtypes::TypeObject dependency_type_object;
            const auto ret = fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
                    dependency_type_identifier,
                    dependency_type_object);

            dependency_name = type_name + "_" + std::to_string(dependency_index);

            // Store dependency in dynamic_types collection
            store_dynamic_type_(dependency_type_identifier, dependency_type_object, dependency_name, dynamic_types);

            // Increment suffix counter
            dependency_index++;
        }

        fastdds::dds::xtypes::TypeObject type_object;
        if (fastdds::dds::RETCODE_OK == fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
                type_identifier,
                type_object))
        {
            // Store dynamic type in dynamic_types collection
            store_dynamic_type_(type_identifier, type_object, type_name, dynamic_types);
        }
    }
}

void McapHandler::store_dynamic_type_(
        const fastdds::dds::xtypes::TypeIdentifier& type_identifier,
        const fastdds::dds::xtypes::TypeObject& type_object,
        const std::string& type_name,
        DynamicTypesCollection& dynamic_types) const
{
    DynamicType dynamic_type;
    dynamic_type.type_name(type_name);

    try
    {
        dynamic_type.type_information(utils::base64_encode(serialize_type_identifier_(type_identifier)));
        dynamic_type.type_object(utils::base64_encode(serialize_type_object_(type_object)));
    }
    catch(const utils::InconsistencyException& e)
    {
        EPROSIMA_LOG_WARNING(DDSRECORDER_MCAP_HANDLER,
                "MCAP_WRITE | Error serializing DynamicType. Error message:\n " << e.what());
        return;
    }

    dynamic_types.dynamic_types().push_back(dynamic_type);

}

fastdds::rtps::SerializedPayload_t* McapHandler::serialize_dynamic_types_(
        DynamicTypesCollection& dynamic_types) const
{
    // Serialize dynamic types collection using CDR
    fastdds::dds::TypeSupport type_support(new DynamicTypesCollectionPubSubType());
    fastdds::rtps::SerializedPayload_t* serialized_payload = new fastdds::rtps::SerializedPayload_t(
        type_support.calculate_serialized_size(&dynamic_types, fastdds::dds::DEFAULT_DATA_REPRESENTATION));
    type_support.serialize(&dynamic_types, *serialized_payload, fastdds::dds::DEFAULT_DATA_REPRESENTATION);

    return serialized_payload;
}

std::string McapHandler::serialize_qos_(
        const TopicQoS& qos)
{
    // TODO: Reuse code from ddspipe_yaml

    YAML::Node qos_yaml;

    // Reliability tag
    YAML::Node reliability_tag = qos_yaml[QOS_SERIALIZATION_RELIABILITY];
    if (qos.is_reliable())
    {
        reliability_tag = true;
    }
    else
    {
        reliability_tag = false;
    }

    // Durability tag
    YAML::Node durability_tag = qos_yaml[QOS_SERIALIZATION_DURABILITY];
    if (qos.is_transient_local())
    {
        durability_tag = true;
    }
    else
    {
        durability_tag = false;
    }

    // Ownership tag
    YAML::Node ownership_tag = qos_yaml[QOS_SERIALIZATION_OWNERSHIP];
    if (qos.has_ownership())
    {
        ownership_tag = true;
    }
    else
    {
        ownership_tag = false;
    }

    // Keyed tag
    YAML::Node keyed_tag = qos_yaml[QOS_SERIALIZATION_KEYED];
    if (qos.keyed)
    {
        keyed_tag = true;
    }
    else
    {
        keyed_tag = false;
    }

    return YAML::Dump(qos_yaml);
}

template<class DynamicTypeData>
std::string McapHandler::serialize_type_data_(
        const DynamicTypeData& type_data)
{
    // Reserve payload and create buffer
    fastcdr::CdrSizeCalculator calculator(fastcdr::CdrVersion::XCDRv2);
    size_t current_alignment {0};
    size_t size = calculator.calculate_serialized_size(type_data, current_alignment) +
                            fastdds::rtps::SerializedPayload_t::representation_header_size;

    fastdds::rtps::SerializedPayload_t payload(static_cast<uint32_t>(size));
    fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

    // Create CDR serializer
    fastcdr::Cdr ser(fastbuffer, fastcdr::Cdr::DEFAULT_ENDIAN,
            fastcdr::CdrVersion::XCDRv2);

    payload.encapsulation = ser.endianness() == fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    // Serialize
    fastcdr::serialize(ser, type_data);
    payload.length = (uint32_t)ser.get_serialized_data_length();
    size = (ser.get_serialized_data_length() + 3) & ~3;

    // Create CDR message with payload
    fastdds::rtps::CDRMessage_t* cdr_message = new fastdds::rtps::CDRMessage_t(payload);

    // Add data
    if (!(cdr_message && (cdr_message->pos + payload.length <= cdr_message->max_size))|| (payload.length > 0 && !payload.data))
    {
        if (!cdr_message)
        {
            throw utils::InconsistencyException(
                    "Error adding data -> cdr_message is null.");
        }
        else if (cdr_message->pos + payload.length > cdr_message->max_size)
        {
            throw utils::InconsistencyException(
                    "Error adding data -> not enough space in cdr_message buffer.");
        }
        else if (payload.length > 0 && !payload.data)
        {
            throw utils::InconsistencyException(
                    "Error adding data -> payload length is greater than 0, but payload data is null.");
        }
    }

    memcpy(&cdr_message->buffer[cdr_message->pos], payload.data, payload.length);
    cdr_message->pos += payload.length;
    cdr_message->length += payload.length;

    fastdds::rtps::octet value = 0;
    for (uint32_t count = payload.length; count < size; ++count)
    {
        const uint32_t size_octet = sizeof(value);
        if (!(cdr_message && (cdr_message->pos + size_octet <= cdr_message->max_size)))
        {
            throw utils::InconsistencyException(
                    "Not enough space in cdr_message buffer.");
        }
        for (uint32_t i = 0; i < size_octet; i++)
        {
            cdr_message->buffer[cdr_message->pos + i] = *((fastdds::rtps::octet*)&value + size_octet - 1 - i);
        }
        cdr_message->pos += size_octet;
        cdr_message->length += size_octet;
    }

    // Copy buffer to string
    std::string typedata_str(reinterpret_cast<char const*>(cdr_message->buffer), size);

    // Delete CDR message
    // NOTE: set wraps attribute to avoid double free (buffer released by payload on destruction)
    cdr_message->wraps = true;
    delete cdr_message;

    return typedata_str;
}

std::string McapHandler::serialize_type_identifier_(
        const fastdds::dds::xtypes::TypeIdentifier& type_identifier)
{
    std::string typeid_string;
    try
    {
        typeid_string = serialize_type_data_(type_identifier);
    }
    catch(const utils::InconsistencyException& e)
    {
        throw utils::InconsistencyException(std::string("Failed to serialize TypeIdentifier: ") + e.what());
    }

    return typeid_string;
}

std::string McapHandler::serialize_type_object_(
        const fastdds::dds::xtypes::TypeObject& type_object)
{
    std::string typeobj_string;
    try
    {
        typeobj_string = serialize_type_data_(type_object);
    }
    catch(const utils::InconsistencyException& e)
    {
        throw utils::InconsistencyException(std::string("Failed to serialize TypeObject: ") + e.what());
    }

    return typeobj_string;
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
