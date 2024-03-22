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

#include <cstdio>
#include <mcap/reader.hpp>

#include <yaml-cpp/yaml.h>

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/utils.hpp>
#include <cpp_utils/ros2_mangling.hpp>

#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/CDRMessage_t.h>
#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/TypeObjectFactory.h>

#include <ddspipe_core/types/dynamic_types/schema.hpp>

#if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    #include <fastcdr/Cdr.h>
    #include <fastcdr/FastBuffer.h>
    #include <fastcdr/FastCdr.h>
    #include <ddsrecorder_participants/common/types/dynamic_types/v1/DynamicTypesCollection.hpp>
    #include <ddsrecorder_participants/common/types/dynamic_types/v1/DynamicTypesCollectionPubSubTypes.hpp>
#else
    #include <fastdds/rtps/common/CdrSerialization.hpp>
    #include <ddsrecorder_participants/common/types/dynamic_types/v2/DynamicTypesCollection.hpp>
    #include <ddsrecorder_participants/common/types/dynamic_types/v2/DynamicTypesCollectionPubSubTypes.hpp>
#endif // if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13

#include <ddsrecorder_participants/constants.hpp>

#include <ddsrecorder_participants/recorder/mcap/McapHandler.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

using namespace eprosima::ddspipe::core::types;

Message::Message(
        const Message& msg)
    : mcap::Message(msg)
{
    this->payload_owner = msg.payload_owner;
    auto payload_owner_ =
            const_cast<eprosima::fastrtps::rtps::IPayloadPool*>((eprosima::fastrtps::rtps::IPayloadPool*)msg.
                    payload_owner);
    this->payload_owner->get_payload(
        msg.payload,
        payload_owner_,
        this->payload);
}

Message::~Message()
{
    // If payload owner exists and payload has size, release it correctly in pool
    if (payload_owner && payload.length > 0)
    {
        payload_owner->release_payload(payload);
    }
}

McapHandler::McapHandler(
        const McapHandlerConfiguration& config,
        const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool,
        const McapHandlerStateCode& init_state /* = McapHandlerStateCode::RUNNING */)
    : configuration_(config)
    , payload_pool_(payload_pool)
    , state_(McapHandlerStateCode::STOPPED)
    , on_disk_full_lambda_set_(false)
{
    logInfo(DDSRECORDER_MCAP_HANDLER,
            "Creating MCAP handler instance.");

    if (init_state == McapHandlerStateCode::RUNNING)
    {
        start();
    }
    else if (init_state == McapHandlerStateCode::PAUSED)
    {
        pause();
    }
}

McapHandler::~McapHandler()
{
    logInfo(DDSRECORDER_MCAP_HANDLER, "Destroying handler.");

    // Stop handler prior to destruction
    stop(true);
}

void McapHandler::add_schema(
        const fastrtps::types::DynamicType_ptr& dynamic_type)
{
    try
    {
        std::lock_guard<std::mutex> lock(mtx_);

        // NOTE: Process schemas even if in STOPPED state to avoid losing them (only sent/received once in discovery)

        assert(nullptr != dynamic_type);

        std::string type_name = dynamic_type->get_name();

        // Check if it exists already
        if (received_types_.find(type_name) != received_types_.end())
        {
            return;
        }

        // Schema not found, generate from dynamic type and store
        std::string schema_text =
                configuration_.ros2_types ? msg::generate_ros2_schema(dynamic_type) : idl::generate_idl_schema(
            dynamic_type);

        logInfo(DDSRECORDER_MCAP_HANDLER, "\nAdding schema with name " << type_name << " :\n" << schema_text << "\n");

        // Create schema and add it to writer and to schemas map
        std::string encoding = configuration_.ros2_types ? "ros2msg" : "omgidl";
        mcap::Schema new_schema(configuration_.ros2_types ? utils::demangle_if_ros_type(dynamic_type->get_name()) :
                dynamic_type->get_name(), encoding, schema_text);

        if ((pending_samples_.find(type_name) != pending_samples_.end()) ||
                (state_ == McapHandlerStateCode::PAUSED &&
                (pending_samples_paused_.find(type_name) != pending_samples_paused_.end())))
        {
            // std::cout << "Deleting blank schema with name " << type_name << "... size: " << get_blank_schema_size_(type_name) << std::endl;
            mcap_size_ -= get_blank_schema_size_(type_name);
            // std::cout << "Deleting blank channel with name " << type_name << "... size: " << get_blank_channel_size_(type_name) << std::endl;
            mcap_size_ -= get_blank_channel_size_(type_name); // Fix: type_name (topic.type_name) should be topic.m_topic_name
        }

        //Check if there is enough space on disk to write the schema
        check_and_update_mcap_size_(new_schema);
        // std::cout << "Writing NEW schema Lucia1" << new_schema.name << "..." << get_schema_size_(new_schema) << std::endl;
        // WARNING: passing as non-const to MCAP library
        mcap_writer_.addSchema(new_schema);

        logInfo(DDSRECORDER_MCAP_HANDLER, "Schema created: " << type_name << ".");

        auto it = schemas_.find(type_name);
        if (it != schemas_.end())
        {
            // Update channels previously created with blank schema
            update_channels_nts_(it->second.id, new_schema.id);
        }
        schemas_[type_name] = std::move(new_schema);
        received_types_.insert(type_name);

        // Check if there are any pending samples for this new schema. If so, add them.
        if ((pending_samples_.find(type_name) != pending_samples_.end()) ||
                (state_ == McapHandlerStateCode::PAUSED &&
                (pending_samples_paused_.find(type_name) != pending_samples_paused_.end())))
        {
            add_pending_samples_nts_(type_name);
        }
        // Every time an element of schemas_ (map of dynamic types with schemas) is added the attachment is newly calculated
        save_dynamic_type_(type_name);
        serialize_dynamic_types_();
    }
    catch (const std::overflow_error& e)
    {
        logError(DDSRECORDER_MCAP_HANDLER, "FAIL_MCAP_WRITE | Failed to write on MCAP file. " << "Error message:\n " <<
            e.what());

        on_disk_full_();
    }
}

void McapHandler::add_data(
        const DdsTopic& topic,
        RtpsPayloadData& data)
{
    try
    {
        std::unique_lock<std::mutex> lock(mtx_);
        if (state_ == McapHandlerStateCode::STOPPED)
        {
            logInfo(DDSRECORDER_MCAP_HANDLER, "Attempting to add sample through a stopped handler, dropping...");
            return;
        }

        if (disk_full_)
        {
            logInfo(DDSRECORDER_MCAP_HANDLER, "Disk full");
            return;
        }

        logInfo(
            DDSRECORDER_MCAP_HANDLER,
            "Adding data in topic " << topic);

        // Add data to channel
        Message msg;
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
            auto payload_owner =
                    const_cast<eprosima::fastrtps::rtps::IPayloadPool*>((eprosima::fastrtps::rtps::IPayloadPool*)data.
                            payload_owner);

            if (payload_owner)
            {
                payload_pool_->get_payload(
                    data.payload,
                    payload_owner,
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
            // Check if there is enough space available before adding the message to buffer
            check_mcap_size_(get_message_size_(msg));
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
                        check_mcap_size_(get_message_size_(msg));
                        if ((pending_samples_.find(topic.type_name) == pending_samples_.end()) ||
                            (state_ == McapHandlerStateCode::PAUSED &&
                            (pending_samples_paused_.find(topic.type_name) == pending_samples_paused_.end())))
                        {
                            std::string encoding = configuration_.ros2_types ? "ros2msg" : "omgidl";
                            mcap::Schema blank_schema(topic.type_name, encoding, "");
                            check_and_update_mcap_size_(blank_schema);
                            // std::cout << "Writing schema Lucia1" << topic.type_name << "..." << get_blank_schema_size_(topic.type_name) << std::endl;
                        }
                        if (!received_topics_.count(topic.m_topic_name))
                        {
                            received_topics_.insert(topic.m_topic_name);
                            check_mcap_size_(get_blank_channel_size_(topic.m_topic_name));
                        }
                        add_data_nts_(msg, topic);
                    }
                }
                else
                {
                    logInfo(
                        DDSRECORDER_MCAP_HANDLER,
                        "Schema for topic " << topic << " not yet available, inserting to pending samples queue.");

                    check_mcap_size_(get_message_size_(msg));
                    if ((pending_samples_.find(topic.type_name) == pending_samples_.end()) ||
                            (state_ == McapHandlerStateCode::PAUSED &&
                            (pending_samples_paused_.find(topic.type_name) == pending_samples_paused_.end())))
                    {
                        std::string encoding = configuration_.ros2_types ? "ros2msg" : "omgidl";
                        mcap::Schema blank_schema(topic.type_name, encoding, "");
                        check_and_update_mcap_size_(blank_schema);
                        // std::cout << "Writing schema Lucia2" << topic.type_name << "..." << get_schema_size_(blank_schema) << std::endl;
                    }
                    if (!received_topics_.count(topic.m_topic_name))
                    {
                        received_topics_.insert(topic.m_topic_name);
                        check_mcap_size_(get_blank_channel_size_(topic.m_topic_name));
                        std::cout << "Writing channel Lucia2" << topic.type_name << "..." << get_blank_channel_size_(topic.m_topic_name) << std::endl;
                    }
                    add_to_pending_nts_(msg, topic);
                }
            }
            else if (state_ == McapHandlerStateCode::PAUSED)
            {
                logInfo(
                    DDSRECORDER_MCAP_HANDLER,
                    "Schema for topic " << topic << " not yet available, inserting to (paused) pending samples queue.");

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
    catch (const std::overflow_error& e)
    {
        std::cout << "Catching exception..." << std::endl;
        logError(DDSRECORDER_MCAP_HANDLER, "FAIL_MCAP_WRITE | Failed to write on MCAP file. " << "Error message:\n " <<
            e.what());

        on_disk_full_();
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
        logWarning(
            DDSRECORDER_MCAP_HANDLER,
            "Ignoring start command, instance already started.");
    }
    else
    {
        logInfo(
            DDSRECORDER_MCAP_HANDLER,
            "Starting handler.");

        if (prev_state == McapHandlerStateCode::STOPPED)
        {
            try
            {
                open_file_nts_();
            }
            catch (const utils::InitializationException& e)
            {
                logError(DDSRECORDER_MCAP_HANDLER, "FAIL_MCAP_OPEN | Failed to open MCAP file. " << "Error message:\n " <<
                    e.what());
            }
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
            logWarning(
                DDSRECORDER_MCAP_HANDLER,
                "Ignoring stop command, instance already stopped.");
        }
    }
    else
    {
        logInfo(
            DDSRECORDER_MCAP_HANDLER,
            "Stopping handler.");

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
        // Close and rename MCAP file
        close_file_nts_();

        // Reset channels
        channels_.clear();

        // Assert all attributes except schemas are cleared
        assert(channels_.size() == 0);
        assert(samples_buffer_.size() == 0);
        assert(pending_samples_.size() == 0);
        assert(pending_samples_paused_.size() == 0);
    }
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
        logWarning(
            DDSRECORDER_MCAP_HANDLER,
            "Ignoring pause command, instance already paused.");
    }
    else
    {
        logInfo(
            DDSRECORDER_MCAP_HANDLER,
            "Pausing handler.");

        if (prev_state == McapHandlerStateCode::STOPPED)
        {
            try
            {
                open_file_nts_();
            }
            catch (const utils::InitializationException& e)
            {
                logError(DDSRECORDER_MCAP_HANDLER, "FAIL_MCAP_OPEN | Failed to open MCAP file. " << "Error message:\n " <<
                    e.what());
            }
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
        logWarning(
            DDSRECORDER_MCAP_HANDLER,
            "Ignoring trigger event command, instance is not paused.");
    }
    else
    {
        logInfo(
            DDSRECORDER_MCAP_HANDLER,
            "Triggering event.");

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

void McapHandler::open_file_nts_()
{
    // Generate filename with current timestamp if applies
    if (configuration_.mcap_output_settings.prepend_timestamp)
    {
        mcap_filename_ = configuration_.mcap_output_settings.output_filepath + "/" + utils::timestamp_to_string(
            utils::now(), configuration_.mcap_output_settings.output_timestamp_format,
            configuration_.mcap_output_settings.output_local_timestamp) + "_" +
                configuration_.mcap_output_settings.output_filename  + ".mcap";
    }
    else
    {
        mcap_filename_ = configuration_.mcap_output_settings.output_filepath + "/" +
                configuration_.mcap_output_settings.output_filename;
    }

    // Append temporal suffix
    std::string tmp_filename = tmp_filename_(mcap_filename_);

    logInfo(DDSRECORDER_MCAP_HANDLER,
            "Opening file <" << tmp_filename << "> .");

    auto status = mcap_writer_.open(tmp_filename.c_str(), configuration_.mcap_writer_options);
    if (!status.ok())
    {
        throw utils::InitializationException(
                  STR_ENTRY << "Failed to open MCAP file " << tmp_filename << " for writing: " << status.message);
    }

    // Check available space in disk when opening file
    std::filesystem::space_info space = std::filesystem::space(configuration_.mcap_output_settings.output_filepath);
    space_available_when_open_ = 10000;

    // Write in new file schemas already received before
    // NOTE: This is necessary since dynamic types are only sent/received once on discovery
    if (!schemas_.empty())
    {
        rewrite_schemas_nts_();
    }
}

void McapHandler::close_file_nts_()
{
    std::cout << "Predicted MCAP file size: " << mcap_size_ << std::endl;

    std::string tmp_filename = tmp_filename_(mcap_filename_);
    logInfo(DDSRECORDER_MCAP_HANDLER,
            "Closing file <" << tmp_filename << "> .");

    // Write version metadata in MCAP file
    write_version_metadata_();

    // Serialize and store dynamic types associated to all added schemas
    if (configuration_.record_types)
    {
        write_attachment_();
    }

    // Close writer and output file
    mcap_writer_.close();

    // Rename temp file to configuration file_name
    if (std::rename(tmp_filename.c_str(), mcap_filename_.c_str()))
    {
        logError(
            DDSRECORDER_MCAP_HANDLER,
            "Failed to rename " << tmp_filename << " into " << mcap_filename_ << " on handler destruction.");
    }
}

void McapHandler::add_data_nts_(
        const Message& msg,
        bool direct_write /* false */)
{
    if (direct_write)
    {
        try
        {
            // Write to MCAP file
            write_message_nts_(msg);
        }
        catch (const utils::InconsistencyException& e)
        {
            logError(DDSRECORDER_MCAP_HANDLER, "FAIL_MCAP_WRITE | Error writting message in channel " << msg.channelId << ". Error message:\n " <<
                    e.what());
        }
    }
    else
    {
        samples_buffer_.push_back(msg);
        if (state_ == McapHandlerStateCode::RUNNING && samples_buffer_.size() == configuration_.buffer_size)
        {
            logInfo(DDSRECORDER_MCAP_HANDLER, "Full buffer, writting to disk...");
            dump_data_nts_();
        }
    }
}

void McapHandler::add_data_nts_(
        Message& msg,
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
        logWarning(DDSRECORDER_MCAP_HANDLER, "Error adding message in topic " << topic << ". Error message:\n " <<
                e.what());
        return;
    }
    add_data_nts_(msg, direct_write);
}

void McapHandler::write_message_nts_(
        const Message& msg)
{
    mcap::Status status;
    status = mcap_writer_.write(msg);
    if (!status.ok())
    {
        throw utils::InconsistencyException(
                  STR_ENTRY << "FAIL_MCAP_WRITE | Error writting in MCAP, error message: " << status.message
                  );
    }
}

void McapHandler::add_to_pending_nts_(
        Message& msg,
        const DdsTopic& topic)
{
    assert(configuration_.max_pending_samples != 0);
    if (configuration_.max_pending_samples > 0 &&
            pending_samples_[topic.type_name].size() == static_cast<unsigned int>(configuration_.max_pending_samples))
    {
        if (configuration_.only_with_schema)
        {
            // Discard oldest message in pending samples
            logWarning(DDSRECORDER_MCAP_HANDLER,
                    "Dropping pending sample in type " << topic.type_name << ": buffer limit (" << configuration_.max_pending_samples <<
                    ") reached.");
        }
        else
        {
            logInfo(DDSRECORDER_MCAP_HANDLER,
                    "Buffer limit (" << configuration_.max_pending_samples <<  ") reached for type " << topic.type_name <<
                    ": writing oldest sample without schema.");

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
    logInfo(DDSRECORDER_MCAP_HANDLER, "Adding pending samples for type: " << schema_name << ".");
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
    logInfo(DDSRECORDER_MCAP_HANDLER, "Adding pending samples for all types.");

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
            logInfo(DDSRECORDER_MCAP_HANDLER, "Finishing event thread routine.");
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
                logInfo(DDSRECORDER_MCAP_HANDLER, "Event thread timeout.");
            }
            else
            {
                logInfo(DDSRECORDER_MCAP_HANDLER, "Event triggered: dumping buffered data.");

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
    logInfo(DDSRECORDER_MCAP_HANDLER, "Removing outdated samples.");

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

    logInfo(DDSRECORDER_MCAP_HANDLER, "Stopping event thread.");
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
    logInfo(DDSRECORDER_MCAP_HANDLER, "Writing data stored in buffer.");

    while (!samples_buffer_.empty())
    {
        auto& sample = samples_buffer_.front();
        try
        {
            // Write to MCAP file
            write_message_nts_(sample);
        }
        catch (const utils::InconsistencyException& e)
        {
            logError(DDSRECORDER_MCAP_HANDLER, "Error writting message in channel " << sample.channelId << ". Error message:\n " <<
                    e.what());
        }

        // Pop written sample (even if exception thrown)
        samples_buffer_.pop_front();
    }
}

mcap::ChannelId McapHandler::create_channel_id_nts_(
        const DdsTopic& topic)
{
    // Find schema
    mcap::SchemaId schema_id;
    bool blank_schema_created = false;
    try
    {
        schema_id = get_schema_id_nts_(topic.type_name);
    }
    catch (const utils::InconsistencyException&)
    {
        if (!configuration_.only_with_schema)
        {
            logInfo(DDSRECORDER_MCAP_HANDLER,
                    "Schema not found for type: " << topic.type_name << ". Creating blank schema...");

            std::string encoding = configuration_.ros2_types ? "ros2msg" : "omgidl";
            mcap::Schema blank_schema(topic.type_name, encoding, "");
            // Add schema reserved space to write it on MCAP
            // check_and_update_mcap_size_(blank_schema); // constant already summed
            mcap_writer_.addSchema(blank_schema);
            schemas_.insert({topic.type_name, std::move(blank_schema)});

            schema_id = blank_schema.id;
            blank_schema_created = true;
        }
        else
        {
            // Propagate exception
            throw;
        }
    }

    if (blank_schema_created)
    {
        std::cout << "Deleting blank channel" << topic.m_topic_name << "... size: " << get_blank_channel_size_(topic.m_topic_name) << std::endl;
        mcap_size_ -= get_blank_channel_size_(topic.m_topic_name);
    }

    // Create new channel
    mcap::KeyValueMap metadata = {};
    metadata[QOS_SERIALIZATION_QOS] = serialize_qos_(topic.topic_qos);
    std::string topic_name =
            configuration_.ros2_types ? utils::demangle_if_ros_topic(topic.m_topic_name) : topic.m_topic_name;
    // Set ROS2_TYPES to "false" if the given topic_name is equal to topic.m_topic_name, otherwise set it to "true".
    metadata[ROS2_TYPES] = topic_name.compare(topic.m_topic_name) ? "true" : "false";
    mcap::Channel new_channel(topic_name, "cdr", schema_id, metadata);
    check_mcap_size_(get_channel_size_(new_channel));
    mcap_writer_.addChannel(new_channel);
    auto channel_id = new_channel.id;
    channels_.insert({topic, std::move(new_channel)});
    logInfo(DDSRECORDER_MCAP_HANDLER, "Channel created: " << topic << ".");

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
            logInfo(DDSRECORDER_MCAP_HANDLER, "Updating channel in topic " << channel.first.m_topic_name << ".");

            assert(channel.first.m_topic_name == channel.second.topic);
            mcap::Channel new_channel(channel.second.topic, "cdr", new_schema_id, channel.second.metadata);
            // Check if there is enough space available to write the channel
            check_mcap_size_(get_channel_size_(new_channel));
            mcap_writer_.addChannel(new_channel);
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

void McapHandler::rewrite_schemas_nts_()
{
    logInfo(DDSRECORDER_MCAP_HANDLER, "Rewriting received schemas.");

    std::map<std::string, mcap::Schema> new_schemas;
    for (const auto& schema : schemas_)
    {
        std::string type_name = schema.first;
        mcap::Schema new_schema = schema.second;

        // Check if there is enough space available to write the schema
        check_and_update_mcap_size_(new_schema);

        // WARNING: passing as non-const to MCAP library
        mcap_writer_.addSchema(new_schema);
        new_schemas[type_name] = std::move(new_schema);

        logInfo(DDSRECORDER_MCAP_HANDLER, "Schema created: " << type_name << ".");
    }

    // Overwrite schemas map
    schemas_ = new_schemas;
}

void McapHandler::save_dynamic_type_(
        const std::string& type_name)
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

            // Store dynamic type in dynamic_types_
            if (type_identifier != nullptr && type_object != nullptr)
            {
                DynamicType dynamic_type;
                dynamic_type.type_name(type_name);
                dynamic_type.type_information(utils::base64_encode(serialize_type_identifier_(type_identifier)));
                dynamic_type.type_object(utils::base64_encode(serialize_type_object_(type_object)));

                dynamic_types_.dynamic_types().push_back(dynamic_type);
            }

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

    // Store dynamic type in dynamic_types_
    if (type_identifier != nullptr && type_object != nullptr)
    {
        DynamicType dynamic_type;
        dynamic_type.type_name(type_name);
        dynamic_type.type_information(utils::base64_encode(serialize_type_identifier_(type_identifier)));
        dynamic_type.type_object(utils::base64_encode(serialize_type_object_(type_object)));

        dynamic_types_.dynamic_types().push_back(dynamic_type);
    }
}

void McapHandler::serialize_dynamic_types_()
{
    // Serialize dynamic types collection using CDR
    eprosima::fastdds::dds::TypeSupport type_support(new DynamicTypesCollectionPubSubType());
    eprosima::fastrtps::rtps::SerializedPayload_t* new_payload = new eprosima::fastrtps::rtps::SerializedPayload_t(
        type_support.get_serialized_size_provider(&dynamic_types_)());
    serialized_payload_.reset(new_payload);
    type_support.serialize(&dynamic_types_, serialized_payload_.get());

    // Recalculate attachment_size_ when serializing dynamic_types_
    mcap_size_ -= attachment_size_;
    attachment_size_ = get_attachment_size_();
    // Check if there is enough space available to write the schema
    check_mcap_size_(attachment_size_);
}

void McapHandler::write_attachment_()
{
    // Write serialized dynamic types into attachments section
    mcap::Attachment dynamic_attachment;
    dynamic_attachment.name = DYNAMIC_TYPES_ATTACHMENT_NAME;
    dynamic_attachment.data = reinterpret_cast<std::byte*>(serialized_payload_->data);
    dynamic_attachment.dataSize = serialized_payload_->length;
    dynamic_attachment.createTime = now();
    auto status = mcap_writer_.write(dynamic_attachment);

    return;
}

void McapHandler::write_version_metadata_()
{
    // Populate map with release version and commit hash
    mcap::KeyValueMap version;
    version[VERSION_METADATA_RELEASE] = DDSRECORDER_PARTICIPANTS_VERSION_STRING;
    version[VERSION_METADATA_COMMIT] = DDSRECORDER_PARTICIPANTS_COMMIT_HASH;

    // Write to MCAP file
    mcap::Metadata version_metadata;
    version_metadata.name = VERSION_METADATA_NAME;
    version_metadata.metadata = version;
    auto status = mcap_writer_.write(version_metadata);
}

std::uint64_t McapHandler::get_message_size_(
        const Message& msg)
{
    constexpr std::uint64_t NUMBER_OF_TIMES_COPIED = 1;

    std::uint64_t size = MCAP_MESSAGE_OVERHEAD;
    size += msg.dataSize;
    size *= NUMBER_OF_TIMES_COPIED;

    return size;
}

std::uint64_t McapHandler::get_schema_size_(
        const mcap::Schema& schema)
{
    constexpr std::uint64_t NUMBER_OF_TIMES_COPIED = 2;
    constexpr std::uint64_t MAGIC_LUCIA = 5;

    std::uint64_t size = MCAP_SCHEMAS_OVERHEAD;
    size += schema.name.size();
    size += schema.encoding.size();
    size += schema.data.size();
    size *= NUMBER_OF_TIMES_COPIED;

    // std::uint64_t schema_constant = size/NUMBER_OF_TIMES_COPIED - schema.name.size();
    // std::cout << "Schema size: " << schema.name.size() << ", " << schema.encoding.size() << ", " << schema.data.size() << std::endl;
    // std::cout << "Schema constant: " << schema_constant << std::endl; //29
    size -= MAGIC_LUCIA;

    return size;
}

std::uint64_t McapHandler::get_blank_schema_size_(
            const std::string& schema_name)
{
    constexpr std::uint64_t FIXED_SIZE = 29;
    constexpr int NUMBER_OF_TIMES_COPIED = 2;
    constexpr std::uint64_t MAGIC_LUCIA = 5;

    std::uint64_t size = FIXED_SIZE;
    size += schema_name.size();
    size *= NUMBER_OF_TIMES_COPIED;
    size -= MAGIC_LUCIA;

    return size;
}

std::uint64_t McapHandler::get_channel_size_(
        const mcap::Channel& channel)
{
    constexpr int NUMBER_OF_TIMES_COPIED = 2;

    std::uint64_t size = MCAP_CHANNEL_OVERHEAD;
    size += channel.topic.size();
    size += channel.messageEncoding.size();
    size += mcap::internal::KeyValueMapSize(channel.metadata);
    size *= NUMBER_OF_TIMES_COPIED;

    // std::uint64_t channel_constant = size/NUMBER_OF_TIMES_COPIED - channel.topic.size();
    // std::cout << "Channel size: " << channel.topic.size() << ", " << channel.messageEncoding.size() << ", " << mcap::internal::KeyValueMapSize(channel.metadata) << std::endl;
    // std::cout << "Channel constant: " << channel_constant << std::endl; //148

    return size;
}

std::uint64_t McapHandler::get_blank_channel_size_(
            const std::string& channel_name)
{
    // TODO: Add different constants
    constexpr std::uint64_t FIXED_SIZE = 148; // 308 / 301
    constexpr int NUMBER_OF_TIMES_COPIED = 2;

    std::uint64_t size = FIXED_SIZE;
    size += channel_name.size();
    size *= NUMBER_OF_TIMES_COPIED;

    return size;
}

std::uint64_t McapHandler::get_attachment_size_()
{
    constexpr std::uint64_t NUMBER_OF_TIMES_COPIED = 1;

    std::uint64_t size = MCAP_ATTACHMENT_OVERHEAD;
    size += serialized_payload_->length;
    size *= NUMBER_OF_TIMES_COPIED;

    return size;
}

void McapHandler::check_mcap_size_(
        const std::uint64_t size)
{
    // mcap_size_ += size;
    std::cout << "Mcap size: " << mcap_size_ << std::endl;
    if (!disk_full_)
    {
        if ((mcap_size_ + size) > space_available_when_open_)
        {
            disk_full_ = true;
            throw std::overflow_error(
                        STR_ENTRY << "Attempted to write an MCAP of size: " << mcap_size_ <<
                    ", but there is not enough space available on disk: " << space_available_when_open_);
        }
        else
        {
            mcap_size_ += size;
            std::cout << "[1] Mcap size after sum: " << mcap_size_ + size << std::endl;
        }
    }
    else
    {
        std::cout << "Disk is full, adding data previously taken into account (should always fit)" << std::endl;
        // assert(false);
        assert((mcap_size_ + size) <= space_available_when_open_);
        mcap_size_ += size;
        std::cout << "[2] Mcap size after sum: " << mcap_size_ + size << std::endl;
    }
}

void McapHandler::check_and_update_mcap_size_(
        const std::uint64_t& size)

    if ((mcap_size_ + size) > space_available_when_open_)
    {
        disk_full_ = true;
        throw std::overflow_error(
                    STR_ENTRY << "Attempted to write an MCAP of size: " << mcap_size_ <<
                ", but there is not enough space available on disk: " << space_available_when_open_);
    }
    else
    {
        mcap_size_ += size;
    }
}

void McapHandler::check_and_update_mcap_size_(
        const Message& msg)
{
    // Calculate message size
    std::uint64_t size = get_message_size_(msg);

    check_and_update_mcap_size_(size);
}

void McapHandler::check_and_update_mcap_size_(
        const mcap::Schema& schema)
{
    // Calculate schema size
    std::uint64_t size = get_schema_size_(schema);

    check_and_update_mcap_size_(size);
}

void McapHandler::check_and_update_mcap_size_(
        const mcap::Channel& channel)
{
    // Calculate channel size
    std::uint64_t size = get_message_size_(msg);

    check_and_update_mcap_size_(size);
}

void McapHandler::on_disk_full_() const noexcept
{
    if (on_disk_full_lambda_set_)
    {
        on_disk_full_lambda_();
    }
    else
    {
        logError(DDSRECORDER_MCAP_HANDLER, "Calling not set on_disk_full callback");
    }
}

void McapHandler::set_on_disk_full_callback(
        std::function<void()> on_disk_full_lambda) noexcept
{
    // std::lock_guard<std::recursive_mutex> lock(mtx_);

    if (on_disk_full_lambda_set_)
    {
        logInfo(DDSRECORDER_MCAP_HANDLER, "Changing on_disk_full callback");
    }

    on_disk_full_lambda_ = on_disk_full_lambda;
    on_disk_full_lambda_set_ = true;
}

std::string McapHandler::tmp_filename_(
        const std::string& filename)
{
    static const std::string TMP_SUFFIX = ".tmp~";
    return filename + TMP_SUFFIX;
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

std::string McapHandler::serialize_type_identifier_(
        const eprosima::fastrtps::types::TypeIdentifier* type_identifier)
{
    // Reserve payload and create buffer
    size_t size = fastrtps::types::TypeIdentifier::getCdrSerializedSize(*type_identifier) +
            eprosima::fastrtps::rtps::SerializedPayload_t::representation_header_size;
    fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(size));
    eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

    // Create CDR serializer
    #if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
    #else
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv1);
    #endif // if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13

    payload.encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    // Serialize
    type_identifier->serialize(ser);
#if FASTCDR_VERSION_MAJOR == 1
    payload.length = (uint32_t)ser.getSerializedDataLength();
    size = (ser.getSerializedDataLength() + 3) & ~3;
#else
    payload.length = (uint32_t)ser.get_serialized_data_length();
    size = (ser.get_serialized_data_length() + 3) & ~3;
#endif // if FASTCDR_VERSION_MAJOR == 1

    // Create CDR message
    // NOTE: Use 0 length to avoid allocation (memory already reserved in payload creation)
    eprosima::fastrtps::rtps::CDRMessage_t* cdr_message = new eprosima::fastrtps::rtps::CDRMessage_t(0);
    cdr_message->buffer = payload.data;
    cdr_message->max_size = payload.max_size;
    cdr_message->length = payload.length;
#if __BIG_ENDIAN__
    cdr_message->msg_endian = eprosima::fastrtps::rtps::BIGEND;
#else
    cdr_message->msg_endian = eprosima::fastrtps::rtps::LITTLEEND;
#endif // if __BIG_ENDIAN__

    // Add data
    bool valid = fastrtps::rtps::CDRMessage::addData(cdr_message, payload.data, payload.length);
    for (uint32_t count = payload.length; count < size; ++count)
    {
        valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    }
    // Copy buffer to string
    std::string typeid_str(reinterpret_cast<char const*>(cdr_message->buffer), size);

    // Delete CDR message
    // NOTE: set wraps attribute to avoid double free (buffer released by payload on destruction)
    cdr_message->wraps = true;
    delete cdr_message;

    return typeid_str;
}

std::string McapHandler::serialize_type_object_(
        const eprosima::fastrtps::types::TypeObject* type_object)
{
    // Reserve payload and create buffer
    size_t size = fastrtps::types::TypeObject::getCdrSerializedSize(*type_object) +
            eprosima::fastrtps::rtps::SerializedPayload_t::representation_header_size;
    fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(size));
    eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

    // Create CDR serializer
    #if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
    #else
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv1);
    #endif // if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    payload.encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    // Serialize
    type_object->serialize(ser);
#if FASTCDR_VERSION_MAJOR == 1
    payload.length = (uint32_t)ser.getSerializedDataLength();
    size = (ser.getSerializedDataLength() + 3) & ~3;
#else
    payload.length = (uint32_t)ser.get_serialized_data_length();
    size = (ser.get_serialized_data_length() + 3) & ~3;
#endif // if FASTCDR_VERSION_MAJOR == 1

    // Create CDR message
    // NOTE: Use 0 length to avoid allocation (memory already reserved in payload creation)
    eprosima::fastrtps::rtps::CDRMessage_t* cdr_message = new eprosima::fastrtps::rtps::CDRMessage_t(0);
    cdr_message->buffer = payload.data;
    cdr_message->max_size = payload.max_size;
    cdr_message->length = payload.length;
#if __BIG_ENDIAN__
    cdr_message->msg_endian = eprosima::fastrtps::rtps::BIGEND;
#else
    cdr_message->msg_endian = eprosima::fastrtps::rtps::LITTLEEND;
#endif // if __BIG_ENDIAN__

    // Add data
    bool valid = fastrtps::rtps::CDRMessage::addData(cdr_message, payload.data, payload.length);
    for (uint32_t count = payload.length; count < size; ++count)
    {
        valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    }
    // Copy buffer to string
    std::string typeobj_str(reinterpret_cast<char const*>(cdr_message->buffer), size);

    // Delete CDR message
    // NOTE: set wraps attribute to avoid double free (buffer released by payload on destruction)
    cdr_message->wraps = true;
    delete cdr_message;

    return typeobj_str;
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
