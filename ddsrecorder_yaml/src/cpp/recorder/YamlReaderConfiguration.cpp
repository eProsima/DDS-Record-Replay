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
 * @file YamlReaderConfiguration.cpp
 *
 */

#include <cstdint>
#include <string>

#include <cpp_utils/Log.hpp>
#include <cpp_utils/utils.hpp>

#include <ddspipe_core/configuration/DdsPipeLogConfiguration.hpp>
#include <ddspipe_core/types/dynamic_types/types.hpp>
#include <ddspipe_core/types/topic/filter/ManualTopic.hpp>
#include <ddspipe_core/types/topic/filter/WildcardDdsFilterTopic.hpp>

#include <ddspipe_participants/xml/XmlHandlerConfiguration.hpp>
#include <ddspipe_participants/types/address/Address.hpp>

#include <ddspipe_yaml/yaml_configuration_tags.hpp>
#include <ddspipe_yaml/Yaml.hpp>
#include <ddspipe_yaml/YamlManager.hpp>
#include <ddspipe_yaml/YamlReader.hpp>

#include <ddsrecorder_participants/recorder/output/OutputSettings.hpp>
#include <ddsrecorder_participants/recorder/handler/sql/SqlHandlerConfiguration.hpp>

#include <ddsrecorder_yaml/recorder/yaml_configuration_tags.hpp>
#include <ddsrecorder_yaml/recorder/YamlReaderConfiguration.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace yaml {

using namespace eprosima::ddspipe::core;
using namespace eprosima::ddspipe::core::types;
using namespace eprosima::ddspipe::participants;
using namespace eprosima::ddspipe::participants::types;
using namespace eprosima::ddspipe::yaml;

RecorderConfiguration::RecorderConfiguration(
        const Yaml& yml,
        const CommandlineArgsRecorder* args /*= nullptr*/)
{
    load_ddsrecorder_configuration_(yml, args);
}

RecorderConfiguration::RecorderConfiguration(
        const std::string& file_path,
        const CommandlineArgsRecorder* args /*= nullptr*/)
{
    load_ddsrecorder_configuration_from_file_(file_path, args);
}

bool RecorderConfiguration::is_valid(
        utils::Formatter& error_msg) noexcept
{
    if (!mcap_enabled && !sql_enabled)
    {
        error_msg << "At least one of MCAP or SQL libraries must be enabled.";
        return false;
    }

    if (mcap_enabled &&
            !mcap_resource_limits.are_limits_valid(error_msg,
            output_safety_margin > OUTPUT_SAFETY_MARGIN_MIN))
    {
        return false;
    }

    if (sql_enabled &&
            !sql_resource_limits.are_limits_valid(error_msg, output_safety_margin > OUTPUT_SAFETY_MARGIN_MIN))
    {
        return false;
    }

    if (sql_enabled &&
            sql_resource_limits.resource_limits_struct.max_file_size_ !=
            sql_resource_limits.resource_limits_struct.max_size_)
    {
        EPROSIMA_LOG_ERROR(DDSRECORDER,
                "SQL max file size is not used as SQL records everything in just one file. It is only used in MCAP configuration.");
        error_msg <<
            "SQL max file size is not used as SQL records everything in just one file. It is only used in MCAP configuration.";
        return false;
    }

    return true;
}

void RecorderConfiguration::load_ddsrecorder_configuration_(
        const Yaml& yml,
        const CommandlineArgsRecorder* args)
{
    try
    {
        YamlReaderVersion version = LATEST;

        ////////////////////////////////////////
        // Create participants configurations //
        ////////////////////////////////////////

        /////
        // Create Simple Participant Configuration
        dds_configuration = std::make_shared<XmlParticipantConfiguration>();
        dds_configuration->id = "SimpleRecorderParticipant";
        dds_configuration->app_id = "DDS_RECORDER";
        dds_configuration->app_metadata = "";
        dds_configuration->is_repeater = false;

        /////
        // Create MCAP Recorder Participant Configuration
        mcap_recorder_configuration = std::make_shared<ParticipantConfiguration>();
        mcap_recorder_configuration->id = "MCAPRecorderRecorderParticipant";
        mcap_recorder_configuration->app_id = "DDS_RECORDER";
        // TODO: fill metadata field once its content has been defined.
        mcap_recorder_configuration->app_metadata = "";
        mcap_recorder_configuration->is_repeater = false;

        /////
        // Create SQL Recorder Participant Configuration
        sql_recorder_configuration = std::make_shared<ParticipantConfiguration>();
        sql_recorder_configuration->id = "SQLRecorderRecorderParticipant";
        sql_recorder_configuration->app_id = "DDS_RECORDER";
        // TODO: fill metadata field once its content has been defined.
        sql_recorder_configuration->app_metadata = "";
        sql_recorder_configuration->is_repeater = false;

        /////
        // Get optional Recorder configuration options
        if (YamlReader::is_tag_present(yml, RECORDER_RECORDER_TAG))
        {
            auto recorder_yml = YamlReader::get_value_in_tag(yml, RECORDER_RECORDER_TAG);
            load_recorder_configuration_(recorder_yml, version);
        }

        // Initialize cleanup_period with twice the value of event_window
        // WARNING: event_window tag (under recorder tag) must have been parsed beforehand
        cleanup_period = 2 * event_window;

        /////
        // Get optional specs configuration
        // WARNING: Parse builtin topics (dds tag) AFTER specs, as some topic-specific default values are set there
        if (YamlReader::is_tag_present(yml, SPECS_TAG))
        {
            auto specs_yml = YamlReader::get_value_in_tag(yml, SPECS_TAG);
            load_specs_configuration_(specs_yml, version);
        }

        /////
        // Get optional DDS configuration options
        if (YamlReader::is_tag_present(yml, RECORDER_DDS_TAG))
        {
            auto dds_yml = YamlReader::get_value_in_tag(yml, RECORDER_DDS_TAG);
            load_dds_configuration_(dds_yml, version);
        }

        // POTENTIAL TODO: Show loaded configuration in log
        // POTENTIAL TODO: Show warning if tag present but not used

        // Block ROS 2 services (RPC) topics
        // RATIONALE:
        // At the time of this writing, services in ROS 2 behave in the following manner: a ROS 2 service
        // client awaits to discover a server, and it is then when a request is sent to this (and only this) server,
        // from which a response is expected.
        // Hence, if these topics are not blocked, the client would wrongly believe DDS-Recorder is a server, thus
        // sending a request for which a response will not be received.
        WildcardDdsFilterTopic rpc_request_topic, rpc_response_topic;
        rpc_request_topic.topic_name.set_value("rq/*");
        rpc_response_topic.topic_name.set_value("rr/*");

        ddspipe_configuration.blocklist.insert(
            utils::Heritable<WildcardDdsFilterTopic>::make_heritable(rpc_request_topic));

        ddspipe_configuration.blocklist.insert(
            utils::Heritable<WildcardDdsFilterTopic>::make_heritable(rpc_response_topic));

        ddspipe_configuration.init_enabled = true;

        // Only trigger the DdsPipe's callbacks when discovering or removing writers
        ddspipe_configuration.discovery_trigger = DiscoveryTrigger::WRITER;

        // Initialize controller domain with the same as the one being recorded
        // WARNING: dds tag must have been parsed beforehand
        controller_domain = dds_configuration->domain;

        /////
        // Get optional remote controller configuration
        if (YamlReader::is_tag_present(yml, RECORDER_REMOTE_CONTROLLER_TAG))
        {
            auto controller_yml = YamlReader::get_value_in_tag(yml, RECORDER_REMOTE_CONTROLLER_TAG);
            load_controller_configuration_(controller_yml, version);
        }

        /////
        // Log Configuration's set methods: Depending on where Log Configuration has been configured
        // (Yaml, Command-Line and/or by default) these methods will set DdsPipeConfiguration's log_configuration
        // taking into account this precedence:
        //  1. Log Configuration set on Command-line.
        //  2. Log Configuration set by YAML.
        //  3. Log Configuration set by default.
        if (args != nullptr)
        {
            ddspipe_configuration.log_configuration.set(args->log_verbosity);
            ddspipe_configuration.log_configuration.set(args->log_filter);
        }
    }
    catch (const std::exception& e)
    {
        throw eprosima::utils::ConfigurationException(
                  utils::Formatter() << "Error loading DDS Recorder configuration from yaml:\n " << e.what());
    }

}

void RecorderConfiguration::load_recorder_configuration_(
        const Yaml& yml,
        const YamlReaderVersion& version)
{
    /////
    // Get optional buffer size
    if (YamlReader::is_tag_present(yml, RECORDER_BUFFER_SIZE_TAG))
    {
        buffer_size = YamlReader::get_positive_int(yml, RECORDER_BUFFER_SIZE_TAG);
    }

    // Get cleanup period
    if (YamlReader::is_tag_present(yml, RECORDER_CLEANUP_PERIOD_TAG))
    {
        cleanup_period = YamlReader::get_positive_int(yml, RECORDER_CLEANUP_PERIOD_TAG);
    }

    /////
    // Get optional event window length
    if (YamlReader::is_tag_present(yml, RECORDER_EVENT_WINDOW_TAG))
    {
        event_window = YamlReader::get_positive_int(yml, RECORDER_EVENT_WINDOW_TAG);
    }

    // Get max pending samples
    if (YamlReader::is_tag_present(yml, RECORDER_MAX_PENDING_SAMPLES_TAG))
    {
        max_pending_samples = YamlReader::get<int>(yml, RECORDER_MAX_PENDING_SAMPLES_TAG, version);
        if (max_pending_samples < -1)
        {
            throw eprosima::utils::ConfigurationException(
                      utils::Formatter() << "Error reading value under tag <" << RECORDER_MAX_PENDING_SAMPLES_TAG <<
                          "> : value cannot be lower than -1.");
        }
    }

    /////
    // Get optional only_with_type
    if (YamlReader::is_tag_present(yml, RECORDER_ONLY_WITH_TYPE_TAG))
    {
        only_with_type = YamlReader::get<bool>(yml, RECORDER_ONLY_WITH_TYPE_TAG, version);
    }

    /////
    // Get optional record_types
    if (YamlReader::is_tag_present(yml, RECORDER_RECORD_TYPES_TAG))
    {
        record_types = YamlReader::get<bool>(yml, RECORDER_RECORD_TYPES_TAG, version);
    }

    /////
    // Get optional ros2_types
    if (YamlReader::is_tag_present(yml, RECORDER_ROS2_TYPES_TAG))
    {
        ros2_types = YamlReader::get<bool>(yml, RECORDER_ROS2_TYPES_TAG, version);
    }

    /////
    // Get optional output configuration
    if (YamlReader::is_tag_present(yml, RECORDER_OUTPUT_TAG))
    {
        const auto output_yml = YamlReader::get_value_in_tag(yml, RECORDER_OUTPUT_TAG);
        load_recorder_output_configuration_(output_yml, version);
    }

    /////
    // Get optional sql configuration
    if (YamlReader::is_tag_present(yml, RECORDER_SQL_TAG))
    {
        const auto sql_yml = YamlReader::get_value_in_tag(yml, RECORDER_SQL_TAG);
        load_recorder_sql_configuration_(sql_yml, version);
        // Disable default MCAP if SQL is enabled
        if (sql_enabled)
        {
            mcap_enabled = false;
        }
    }

    /////
    // Get optional mcap configuration
    if (YamlReader::is_tag_present(yml, RECORDER_MCAP_TAG))
    {
        const auto mcap_yml = YamlReader::get_value_in_tag(yml, RECORDER_MCAP_TAG);
        load_recorder_mcap_configuration_(mcap_yml, version);
    }
}

void RecorderConfiguration::load_recorder_output_configuration_(
        const Yaml& yml,
        const YamlReaderVersion& version)
{
    /////
    // Get optional file path
    if (YamlReader::is_tag_present(yml, RECORDER_OUTPUT_PATH_FILE_TAG))
    {
        output_filepath = YamlReader::get<std::string>(yml, RECORDER_OUTPUT_PATH_FILE_TAG, version);
    }

    /////
    // Get optional file name
    if (YamlReader::is_tag_present(yml, RECORDER_OUTPUT_FILE_NAME_TAG))
    {
        output_filename = YamlReader::get<std::string>(yml, RECORDER_OUTPUT_FILE_NAME_TAG, version);
    }

    /////
    // Get optional timestamp format
    if (YamlReader::is_tag_present(yml, RECORDER_OUTPUT_TIMESTAMP_FORMAT_TAG))
    {
        output_timestamp_format = YamlReader::get<std::string>(yml, RECORDER_OUTPUT_TIMESTAMP_FORMAT_TAG,
                        version);
    }

    /////
    // Get optional timestamp format
    if (YamlReader::is_tag_present(yml, RECORDER_OUTPUT_LOCAL_TIMESTAMP_TAG))
    {
        output_local_timestamp = YamlReader::get<bool>(yml, RECORDER_OUTPUT_LOCAL_TIMESTAMP_TAG, version);
    }

    /////
    // Get optional size tolerance
    if (YamlReader::is_tag_present(yml, RECORDER_OUTPUT_TAG))
    {
        const auto& output_safety_margin_tmp = YamlReader::get<std::string>(yml,
                        RECORDER_OUTPUT_TAG,
                        version);
        output_safety_margin = eprosima::utils::to_bytes(output_safety_margin_tmp);
        if (output_safety_margin < OUTPUT_SAFETY_MARGIN_MIN)
        {
            output_safety_margin = OUTPUT_SAFETY_MARGIN_MIN;
            EPROSIMA_LOG_ERROR(YAML_READER_CONFIGURATION,
                    "NOT VALID VALUE | SQL " << RECORDER_OUTPUT_TAG << " must be greater than the minimum value accepted. Defaulting to (Kb): " << output_safety_margin /
                    1024);
        }
    }
}

void RecorderConfiguration::load_recorder_mcap_configuration_(
        const Yaml& yml,
        const YamlReaderVersion& version)
{
    /////
    // Get mandatory enable
    mcap_enabled = YamlReader::get<bool>(yml, RECORDER_MCAP_ENABLE_TAG, version);

    if (!mcap_enabled)
    {
        return;
    }

    /////
    // Get optional log publishTime
    if (YamlReader::is_tag_present(yml, RECORDER_MCAP_LOG_PUBLISH_TIME_TAG))
    {
        mcap_log_publish_time = YamlReader::get<bool>(yml, RECORDER_MCAP_LOG_PUBLISH_TIME_TAG, version);
    }

    /////
    // Get optional compression settings
    if (YamlReader::is_tag_present(yml, RECORDER_MCAP_COMPRESSION_SETTINGS_TAG))
    {
        mcap_writer_options = YamlReader::get<mcap::McapWriterOptions>(yml, RECORDER_MCAP_COMPRESSION_SETTINGS_TAG,
                        version);
    }

    /////
    // Get optional resource limits
    if (YamlReader::is_tag_present(yml, RECORDER_RESOURCE_LIMITS_TAG))
    {
        auto mcap_resource_limits_yml = YamlReader::get_value_in_tag(yml, RECORDER_RESOURCE_LIMITS_TAG);
        mcap_resource_limits_enabled = true;
        mcap_resource_limits = ResourceLimitsConfiguration(mcap_resource_limits_yml, version);
    }
}

void RecorderConfiguration::load_recorder_sql_configuration_(
        const Yaml& yml,
        const YamlReaderVersion& version)
{
    /////
    // Get mandatory enable
    sql_enabled = YamlReader::get<bool>(yml, RECORDER_SQL_ENABLE_TAG, version);

    if (!sql_enabled)
    {
        return;
    }

    /////
    // Get optional log publishTime
    if (YamlReader::is_tag_present(yml, RECORDER_SQL_DATA_FORMAT_TAG))
    {
        const auto data_format_yml = YamlReader::get_value_in_tag(yml, RECORDER_SQL_DATA_FORMAT_TAG);
        sql_data_format = YamlReader::get_enumeration<ddsrecorder::participants::DataFormat>(data_format_yml,
                    {
                        {RECORDER_SQL_DATA_FORMAT_CDR_TAG,  ddsrecorder::participants::DataFormat::cdr},
                        {RECORDER_SQL_DATA_FORMAT_JSON_TAG, ddsrecorder::participants::DataFormat::json},
                        {RECORDER_SQL_DATA_FORMAT_BOTH_TAG, ddsrecorder::participants::DataFormat::both}
                    });
    }

    /////
    // Get optional resource limits
    if (YamlReader::is_tag_present(yml, RECORDER_RESOURCE_LIMITS_TAG))
    {
        auto sql_resource_limits_yml = YamlReader::get_value_in_tag(yml, RECORDER_RESOURCE_LIMITS_TAG);
        sql_resource_limits_enabled = true;
        sql_resource_limits = ResourceLimitsConfiguration(sql_resource_limits_yml, version);
        // As max_file_size is not used in SQL configuration, if only any of the two is set, both must coincide.
        // If both are set and different, an error will be thrown in is_valid()
        if (sql_resource_limits.resource_limits_struct.max_file_size_ ==
                0 ^ sql_resource_limits.resource_limits_struct.max_size_ == 0)
        {
            if (sql_resource_limits.resource_limits_struct.max_file_size_ == 0)
            {
                sql_resource_limits.resource_limits_struct.max_file_size_ =
                        sql_resource_limits.resource_limits_struct.max_size_;
            }
            else
            {
                sql_resource_limits.resource_limits_struct.max_size_ =
                        sql_resource_limits.resource_limits_struct.max_file_size_;
            }
        }
    }
}

void RecorderConfiguration::load_controller_configuration_(
        const Yaml& yml,
        const YamlReaderVersion& version)
{
    // Get optional enable remote controller
    if (YamlReader::is_tag_present(yml, RECORDER_REMOTE_CONTROLLER_ENABLE_TAG))
    {
        enable_remote_controller = YamlReader::get<bool>(yml, RECORDER_REMOTE_CONTROLLER_ENABLE_TAG,
                        version);
    }

    // Get optional DDS domain
    if (YamlReader::is_tag_present(yml, DOMAIN_ID_TAG))
    {
        controller_domain = YamlReader::get<DomainId>(yml, DOMAIN_ID_TAG, version);
    }

    // Get optional initial state
    if (YamlReader::is_tag_present(yml, RECORDER_REMOTE_CONTROLLER_INITIAL_STATE_TAG))
    {
        // Convert to enum and check valid wherever used to avoid mcap library dependency in YAML module
        initial_state = YamlReader::get<std::string>(yml,
                        RECORDER_REMOTE_CONTROLLER_INITIAL_STATE_TAG, version);
        // Case insensitive
        eprosima::utils::to_uppercase(initial_state);
    }

    // Get optional command topic name
    if (YamlReader::is_tag_present(yml, RECORDER_REMOTE_CONTROLLER_COMMAND_TOPIC_NAME_TAG))
    {
        command_topic_name = YamlReader::get<std::string>(yml,
                        RECORDER_REMOTE_CONTROLLER_COMMAND_TOPIC_NAME_TAG, version);
    }

    // Get optional status topic name
    if (YamlReader::is_tag_present(yml, RECORDER_REMOTE_CONTROLLER_STATUS_TOPIC_NAME_TAG))
    {
        status_topic_name = YamlReader::get<std::string>(yml,
                        RECORDER_REMOTE_CONTROLLER_STATUS_TOPIC_NAME_TAG, version);
    }
}

void RecorderConfiguration::load_specs_configuration_(
        const Yaml& yml,
        const YamlReaderVersion& version)
{
    // Get number of threads
    if (YamlReader::is_tag_present(yml, NUMBER_THREADS_TAG))
    {
        n_threads = YamlReader::get_positive_int(yml, NUMBER_THREADS_TAG);
    }

    /////
    // Get optional Topic QoS
    if (YamlReader::is_tag_present(yml, SPECS_QOS_TAG))
    {
        YamlReader::fill<TopicQoS>(topic_qos, YamlReader::get_value_in_tag(yml, SPECS_QOS_TAG), version);
        TopicQoS::default_topic_qos.set_value(topic_qos);
    }

    /////
    // Get optional Log Configuration
    if (YamlReader::is_tag_present(yml, LOG_CONFIGURATION_TAG))
    {
        ddspipe_configuration.log_configuration = YamlReader::get<DdsPipeLogConfiguration>(yml, LOG_CONFIGURATION_TAG,
                        version);
    }

    // Get optional monitor tag
    if (YamlReader::is_tag_present(yml, MONITOR_TAG))
    {
        monitor_configuration = YamlReader::get<MonitorConfiguration>(yml, MONITOR_TAG, version);
    }
}

void RecorderConfiguration::load_dds_configuration_(
        const Yaml& yml,
        const YamlReaderVersion& version)
{
    // Get optional xml configuration
    if (YamlReader::is_tag_present(yml, XML_TAG))
    {
        YamlReader::fill<XmlHandlerConfiguration>(
            xml_configuration,
            YamlReader::get_value_in_tag(yml, XML_TAG),
            version);
    }

    // Check if RECORDER_PROFILE_TAG exists
    if (YamlReader::is_tag_present(yml, RECORDER_PROFILE_TAG))
    {
        dds_configuration->participant_profile = YamlReader::get<std::string>(yml, RECORDER_PROFILE_TAG, version);
        xml_enabled = true;
    }

    // Get optional DDS domain
    if (YamlReader::is_tag_present(yml, DOMAIN_ID_TAG))
    {
        dds_configuration->domain = YamlReader::get<DomainId>(yml, DOMAIN_ID_TAG, version);
    }

    /////
    // Get optional whitelist interfaces
    if (YamlReader::is_tag_present(yml, WHITELIST_INTERFACES_TAG))
    {
        dds_configuration->whitelist = YamlReader::get_set<WhitelistType>(yml, WHITELIST_INTERFACES_TAG,
                        version);
    }

    /////
    // Get optional partitions
    if (YamlReader::is_tag_present(yml, PARTITIONLIST_TAG))
    {
        simple_configuration->partitionlist = YamlReader::get_set<std::string>(yml, PARTITIONLIST_TAG,
                        version);

        // check if the wildcard partition is in the partitionlist
        bool wildcard = false;
        for(std::string partition: simple_configuration->partitionlist)
        {
            if(partition == "*")
            {
                wildcard = true;
                break;
            }
        }

        if(wildcard)
        {
            // the partitionslist contains "*" -> clear the list,
            // all the partitions are allowed in the filter
            simple_configuration->partitionlist.clear();
        }
    }

    // Optional get Transport protocol
    if (YamlReader::is_tag_present(yml, TRANSPORT_DESCRIPTORS_TRANSPORT_TAG))
    {
        dds_configuration->transport = YamlReader::get<TransportDescriptors>(yml,
                        TRANSPORT_DESCRIPTORS_TRANSPORT_TAG,
                        version);
    }
    else
    {
        dds_configuration->transport = TransportDescriptors::builtin;
    }

    // Optional get ROS 2 easy mode IP
    if (YamlReader::is_tag_present(yml, EASY_MODE_TAG))
    {
        dds_configuration->easy_mode_ip = YamlReader::get<IpType>(yml, EASY_MODE_TAG, version);
    }

    // Optional get ignore participant flags
    if (YamlReader::is_tag_present(yml, IGNORE_PARTICIPANT_FLAGS_TAG))
    {
        dds_configuration->ignore_participant_flags = YamlReader::get<IgnoreParticipantFlags>(yml,
                        IGNORE_PARTICIPANT_FLAGS_TAG,
                        version);
    }
    else
    {
        dds_configuration->ignore_participant_flags = IgnoreParticipantFlags::no_filter;
    }

    /////
    // Get optional allowlist
    if (YamlReader::is_tag_present(yml, ALLOWLIST_TAG))
    {
        ddspipe_configuration.allowlist = YamlReader::get_set<utils::Heritable<IFilterTopic>>(yml, ALLOWLIST_TAG,
                        version);
    }

    /////
    // Get optional blocklist
    if (YamlReader::is_tag_present(yml, BLOCKLIST_TAG))
    {
        ddspipe_configuration.blocklist = YamlReader::get_set<utils::Heritable<IFilterTopic>>(yml, BLOCKLIST_TAG,
                        version);
    }

    /////
    // Get optional topics
    if (YamlReader::is_tag_present(yml, TOPICS_TAG))
    {
        const auto& manual_topics = YamlReader::get_list<ManualTopic>(yml, TOPICS_TAG, version);
        ddspipe_configuration.manual_topics =
                std::vector<ManualTopic>(manual_topics.begin(), manual_topics.end());
    }

    /////
    // Get optional builtin topics
    if (YamlReader::is_tag_present(yml, BUILTIN_TAG))
    {
        // WARNING: Parse builtin topics AFTER specs and recorder, as some topic-specific default values are set there
        ddspipe_configuration.builtin_topics = YamlReader::get_set<utils::Heritable<DistributedTopic>>(yml, BUILTIN_TAG,
                        version);
    }
}

void RecorderConfiguration::load_ddsrecorder_configuration_from_file_(
        const std::string& file_path,
        const CommandlineArgsRecorder* args)
{
    Yaml yml;

    // Load file
    try
    {
        if (!file_path.empty())
        {
            yml = YamlManager::load_file(file_path);
        }
    }
    catch (const std::exception& e)
    {
        throw eprosima::utils::ConfigurationException(
                  utils::Formatter() << "Error loading DDS Recorder configuration from file: <" << file_path <<
                      "> :\n " << e.what());
    }

    RecorderConfiguration::load_ddsrecorder_configuration_(yml, args);
}

} /* namespace yaml */
} /* namespace ddsrecorder */
} /* namespace eprosima */
