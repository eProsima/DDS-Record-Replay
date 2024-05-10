// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BaseHandler.cpp
 */

#include <algorithm>
#include <chrono>

#include <fastrtps/types/TypeObjectFactory.h>

#include <cpp_utils/time/time_utils.hpp>

#include <ddspipe_core/types/dynamic_types/schema.hpp>

#if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v1/DynamicTypesCollection.hpp>
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v1/DynamicTypesCollectionPubSubTypes.hpp>
#else
    #include <fastdds/rtps/common/CdrSerialization.hpp>
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v2/DynamicTypesCollection.hpp>
    #include <ddsrecorder_participants/common/types/dynamic_types_collection/v2/DynamicTypesCollectionPubSubTypes.hpp>
#endif // if FASTRTPS_VERSION_MAJOR <= 2 && FASTRTPS_VERSION_MINOR < 13

#include <ddsrecorder_participants/constants.hpp>
#include <ddsrecorder_participants/recorder/mcap/utils.hpp>
#include <ddsrecorder_participants/recorder/message/BaseMessage.hpp>
#include <ddsrecorder_participants/recorder/output/BaseHandler.hpp>
#include <ddsrecorder_participants/recorder/output/Serializer.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

using namespace eprosima::ddspipe::core::types;

BaseHandler::BaseHandler(
        const McapHandlerConfiguration& config,
        const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool)
    : configuration_(config)
    , payload_pool_(payload_pool)
    , state_(BaseHandlerStateCode::STOPPED)
{
    logInfo(DDSRECORDER_DATA_HANDLER, "Creating handler instance.");
}

BaseHandler::~BaseHandler()
{
    logInfo(DDSRECORDER_DATA_HANDLER, "Destroying handler.");
}

void BaseHandler::init(
        const BaseHandlerStateCode& init_state /* = BaseHandlerStateCode::RUNNING */)
{
    switch (init_state)
    {
        case BaseHandlerStateCode::RUNNING:
            start();
            break;

        case BaseHandlerStateCode::PAUSED:
            pause();
            break;

        default:
            break;
    }
}

void BaseHandler::start()
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
    BaseHandlerStateCode prev_state = state_;
    state_ = BaseHandlerStateCode::RUNNING;

    if (prev_state == BaseHandlerStateCode::RUNNING)
    {
        logWarning(
            DDSRECORDER_DATA_HANDLER,
            "Ignoring start command, instance already started.");
    }
    else
    {
        logInfo(
            DDSRECORDER_DATA_HANDLER,
            "Starting handler.");

        if (prev_state == BaseHandlerStateCode::STOPPED)
        {
            enable();
        }
        else if (prev_state == BaseHandlerStateCode::PAUSED)
        {
            // Stop event routine (cleans buffers)
            stop_event_thread_nts_(event_lock);
        }
    }
}

void BaseHandler::stop(
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
    BaseHandlerStateCode prev_state = state_;
    state_ = BaseHandlerStateCode::STOPPED;

    switch (prev_state)
    {
        case BaseHandlerStateCode::STOPPED:

            if (!on_destruction)
            {
                logWarning(
                    DDSRECORDER_DATA_HANDLER,
                    "Ignoring stop command, instance already stopped.");
            }

            break;

        case BaseHandlerStateCode::PAUSED:

            // Stop event routine (cleans buffers)
            stop_event_thread_nts_(event_lock);

            [[fallthrough]];

        default:

            logInfo(
                DDSRECORDER_DATA_HANDLER,
                "Stopping handler.");

            if (!configuration_.only_with_schema)
            {
                // Adds to buffer samples whose schema was not received while running
                // NOTE: Loop this way since dump_pending_samples_nts_ removes the entry from the map
                while (!pending_samples_.empty())
                {
                    const auto type_name = pending_samples_.begin()->first;
                    dump_pending_samples_nts_(type_name);
                }
            }
            else
            {
                // Free memory resources
                pending_samples_.clear();
            }

            // if prev_state == RUNNING -> writes buffer + added pending samples (if !only_with_schema)
            // if prev_state == PAUSED  -> writes added pending samples (if !only_with_schema)
            write_samples_(samples_buffer_);

            disable();
            break;
    }
}

void BaseHandler::pause()
{
    // Protect access to state and data structures
    std::lock_guard<std::mutex> lock(mtx_);

    // NOTE: no need to take event mutex as event thread does not exist at this point

    // Store previous state to act differently depending on its value
    BaseHandlerStateCode prev_state = state_;
    state_ = BaseHandlerStateCode::PAUSED;

    if (prev_state == BaseHandlerStateCode::PAUSED)
    {
        logWarning(
            DDSRECORDER_DATA_HANDLER,
            "Ignoring pause command, instance already paused.");
    }
    else
    {
        logInfo(
            DDSRECORDER_DATA_HANDLER,
            "Pausing handler.");

        if (prev_state == BaseHandlerStateCode::STOPPED)
        {
            enable();
        }
        else if (prev_state == BaseHandlerStateCode::RUNNING)
        {
            // Write data stored in buffer
            write_samples_(samples_buffer_);
        }

        // Launch event thread routine
        event_flag_ = EventCode::untriggered;  // No need to take event mutex (protected by mtx_)
        event_thread_ = std::thread(&BaseHandler::event_thread_routine_, this);
    }
}

void BaseHandler::trigger_event()
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

    if (state_ != BaseHandlerStateCode::PAUSED)
    {
        logWarning(
            DDSRECORDER_DATA_HANDLER,
            "Ignoring trigger event command, instance is not paused.");
    }
    else
    {
        logInfo(
            DDSRECORDER_DATA_HANDLER,
            "Triggering event.");

        // Notify event routine thread an event has been triggered
        event_flag_ = EventCode::triggered;
        event_lock.unlock(); // Unlock before notifying for efficiency purposes
        event_cv_.notify_all(); // Need to notify all as not possible to notify a specific thread
    }
}

void BaseHandler::event_thread_routine_()
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
            logInfo(DDSRECORDER_DATA_HANDLER, "Finishing event thread routine.");
            keep_going = false;
        }
        else
        {
            // Protect access to state and data structures
            std::lock_guard<std::mutex> lock(mtx_);

            // NOTE: event mutex not released until routine completed to avoid other commands (start/stop/trigger) to interfere.

            // Delete outdated samples if timeout, and also before writing the buffer (event triggered case)
            remove_outdated_samples_nts_();

            if (timeout)
            {
                logInfo(DDSRECORDER_DATA_HANDLER, "Event thread timeout.");
            }
            else
            {
                logInfo(DDSRECORDER_DATA_HANDLER, "Event triggered: writing buffered data.");

                if (!(configuration_.max_pending_samples == 0 && configuration_.only_with_schema))
                {
                    // Move (paused) pending samples to buffer (or pending samples) before writing the buffer
                    for (auto& [_, pending] : pending_samples_paused_)
                    {
                        while (!pending.empty())
                        {
                            const auto& sample = pending.front();

                            if (configuration_.max_pending_samples != 0)
                            {
                                add_sample_to_pending_nts_(sample);
                            }
                            else if (!configuration_.only_with_schema)
                            {
                                // Add to buffer with blank schema
                                add_sample_to_buffer_nts_(sample);
                            }

                            pending.pop_front();
                        }
                    }
                }

                write_samples_(samples_buffer_);
            }

            // Event routine iteration completed: reset and wait for next event
            event_flag_ = EventCode::untriggered;
        }

        // Notify threads waiting for this resource
        event_lock.unlock();
        event_cv_.notify_all();
    }
}

void BaseHandler::stop_event_thread_nts_(
        std::unique_lock<std::mutex>& event_lock)
{
    // NOTE: this method assumes both mtx_ and event_cv_mutex_ (within event_lock) are locked

    // WARNING: state must have been set different to PAUSED before calling this method
    assert(state_ != BaseHandlerStateCode::PAUSED);

    logInfo(DDSRECORDER_DATA_HANDLER, "Stopping event thread.");

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

void BaseHandler::add_sample_to_buffer_nts_(
        const BaseMessage* sample)
{
    samples_buffer_.push_back(sample);

    if (state_ != BaseHandlerStateCode::RUNNING || samples_buffer_.size() < configuration_.buffer_size)
    {
        // Don't write samples to disk
        return;
    }

    if (samples_buffer_.size() == configuration_.buffer_size)
    {
        logInfo(DDSRECORDER_BASE_HANDLER,
                "The buffer is full. Writing to disk...");
    }
    else
    {
        logWarning(DDSRECORDER_BASE_HANDLER,
                   "The buffer's size (" << samples_buffer_.size() << ") exceeds its limit (" <<
                   configuration_.buffer_size << "). Writing to disk...");
    }

    write_samples_(samples_buffer_);
}

void BaseHandler::add_samples_to_buffer_nts_(
        std::list<const BaseMessage*>& samples)
{
    while (!samples.empty())
    {
        add_sample_to_buffer_nts_(samples.front());
        samples.pop_front();
    }
}

void BaseHandler::add_sample_to_pending_nts_(
        const BaseMessage* sample)
{
    assert(configuration_.max_pending_samples != 0);

    auto& pending_samples = pending_samples_[sample->topic.type_name];

    while (pending_samples.size() >= configuration_.max_pending_samples)
    {
        // The pending samples buffer is full. Discard the oldest sample.
        const auto oldest_sample = pending_samples.front();
        pending_samples.pop_front();

        if (configuration_.only_with_schema)
        {
            logWarning(DDSRECORDER_BASE_HANDLER,
                      "Dropping pending sample in type " << sample->topic.type_name << ": buffer limit (" <<
                      configuration_.max_pending_samples << ") reached.");
        }
        else
        {
            logInfo(DDSRECORDER_BASE_HANDLER,
                    "Buffer limit (" << configuration_.max_pending_samples <<  ") reached for type " <<
                    sample->topic.type_name << ": writing oldest sample without schema.");

            add_sample_to_buffer_nts_(oldest_sample);
        }
    }

    pending_samples.push_back(sample);
}

void BaseHandler::dump_pending_samples_nts_(
        const std::string& type_name)
{
    logInfo(DDSRECORDER_BASE_HANDLER, "Adding pending samples for type: " << type_name << ".");

    if (pending_samples_.find(type_name) != pending_samples_.end())
    {
        if (state_ == BaseHandlerStateCode::PAUSED)
        {
            // The samples were received previously in the RUNNING state.
            // To avoid them being cleaned by the event thread, we write them directly.
            write_samples_(pending_samples_[type_name]);
        }
        else
        {
            // Move samples from pending_samples to buffer
            add_samples_to_buffer_nts_(pending_samples_[type_name]);
        }

        pending_samples_.erase(type_name);
    }

    if (state_ == BaseHandlerStateCode::PAUSED &&
            (pending_samples_paused_.find(type_name) != pending_samples_paused_.end()))
    {
        // Move samples from pending_samples_paused to buffer
        add_samples_to_buffer_nts_(pending_samples_paused_[type_name]);
        pending_samples_paused_.erase(type_name);
    }
}

void BaseHandler::remove_outdated_samples_nts_()
{
    logInfo(DDSRECORDER_BASE_HANDLER, "Removing outdated samples.");

    ddspipe::core::types::DataTime threshold;
    ddspipe::core::types::DataTime::now(threshold);
    threshold = threshold - ddspipe::core::types::DataTime(configuration_.event_window);

    // NOTE: the outdated pending samples are not removed since they must be written as soon as they receive their type.

    // Buffer
    samples_buffer_.remove_if([&](const auto sample)
            {
                return sample->log_time < threshold;
            });

    // Pending samples paused
    for (auto& [_, pending] : pending_samples_paused_)
    {
        pending.remove_if([&](const auto sample)
                {
                    return sample->log_time < threshold;
                });
    }
}

void BaseHandler::store_dynamic_type_(
        const std::string& type_name)
{
    auto type_object_factory = fastrtps::types::TypeObjectFactory::get_instance();
    const auto type_information = type_object_factory->get_type_information(type_name);

    if (type_information != nullptr)
    {
        // Store dependencies as dynamic types
        auto dependencies = type_information->complete().dependent_typeids();
        unsigned int dependency_index = 0;

        for (auto dependency : dependencies)
        {
            const auto type_identifier = &dependency.type_id();
            const auto type_object = type_object_factory->get_type_object(type_identifier);
            const auto dependency_name = type_name + "_" + std::to_string(dependency_index);

            // Store dependency in dynamic_types collection
            store_dynamic_type_(type_identifier, type_object, dependency_name);

            // Increment suffix counter
            dependency_index++;
        }
    }

    const fastrtps::types::TypeObject* type_object = nullptr;
    const fastrtps::types::TypeIdentifier* type_identifier = type_object_factory->get_type_identifier(type_name, true);

    if (type_identifier)
    {
        type_object = type_object_factory->get_type_object(type_name, true);

        // If complete not found, try with minimal
        if (!type_object)
        {
            type_identifier = type_object_factory->get_type_identifier(type_name, false);

            if (type_identifier)
            {
                type_object = type_object_factory->get_type_object(type_name, false);
            }
        }
    }

    // Store dynamic type in dynamic_types collection
    store_dynamic_type_(type_identifier, type_object, type_name);
}

void BaseHandler::store_dynamic_type_(
        const fastrtps::types::TypeIdentifier* type_identifier,
        const fastrtps::types::TypeObject* type_object,
        const std::string& type_name)
{
    if (type_identifier == nullptr)
    {
        logWarning(DDSRECORDER_MCAP_HANDLER, "Attempting to store a DynamicType without a type identifier. Exiting.");
        return;
    }

    if (type_object == nullptr)
    {
        logWarning(DDSRECORDER_MCAP_HANDLER, "Attempting to store a DynamicType without a type object. Exiting.");
        return;
    }

    DynamicType dynamic_type;
    dynamic_type.type_name(type_name);
    dynamic_type.type_information(utils::base64_encode(Serializer::serialize(*type_identifier)));
    dynamic_type.type_object(utils::base64_encode(Serializer::serialize(*type_object)));
    dynamic_types_.dynamic_types().push_back(dynamic_type);
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
