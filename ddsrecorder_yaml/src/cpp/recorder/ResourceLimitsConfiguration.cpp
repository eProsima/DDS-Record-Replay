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
 * @file ResourceLimitsConfiguration.cpp
 */

#include <cstdint>
#include <string>
#include <iostream>

#include <cpp_utils/Log.hpp>

#include <ddspipe_yaml/yaml_configuration_tags.hpp>
#include <ddspipe_yaml/YamlManager.hpp>
#include <ddspipe_yaml/YamlReader.hpp>

#include <ddsrecorder_yaml/recorder/yaml_configuration_tags.hpp>

#include <ddsrecorder_yaml/recorder/ResourceLimitsConfiguration.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace yaml {

using namespace eprosima::ddspipe::yaml;



ResourceLimitsConfiguration::ResourceLimitsConfiguration(
            const eprosima::Yaml& yml,
            const YamlReaderVersion& version)
{
    /////
    // Get optional file rotation
    if (YamlReader::is_tag_present(yml, RECORDER_RESOURCE_LIMITS_LOG_ROTATION_TAG))
    {
        resource_limits_struct.file_rotation_ = YamlReader::get<bool>(yml,
                        RECORDER_RESOURCE_LIMITS_LOG_ROTATION_TAG, version);
    }

    /////
    // Get optional max size
    if (YamlReader::is_tag_present(yml, RECORDER_RESOURCE_LIMITS_MAX_SIZE_TAG))
    {
        const auto& max_size_str = YamlReader::get<std::string>(yml,
                        RECORDER_RESOURCE_LIMITS_MAX_SIZE_TAG,
                        version);
        resource_limits_struct.max_size_ = eprosima::utils::to_bytes(max_size_str);
        
        resource_limits_struct.max_file_size_ = resource_limits_struct.max_size_;
    }
    

    /////
    // Get optional max file size
    if (YamlReader::is_tag_present(yml, RECORDER_RESOURCE_LIMITS_MAX_FILE_SIZE_TAG))
    {
        const auto& max_file_size_str = YamlReader::get<std::string>(yml,
                        RECORDER_RESOURCE_LIMITS_MAX_FILE_SIZE_TAG,
                        version);
        resource_limits_struct.max_file_size_ = eprosima::utils::to_bytes(max_file_size_str);
    }
    // TODO: In SQL configuration, max file size is not used. It is only used in MCAP configuration. This is a temporary solution.

    /////
    // Get optional size tolerance
    if (YamlReader::is_tag_present(yml, RECORDER_RESOURCE_LIMITS_SIZE_TOLERANCE_TAG))
    {
        const auto& size_tolerance_str = YamlReader::get<std::string>(yml,
                        RECORDER_RESOURCE_LIMITS_SIZE_TOLERANCE_TAG,
                        version);
        if(eprosima::utils::to_bytes(size_tolerance_str) < resource_limits_struct.size_tolerance_)
            EPROSIMA_LOG_ERROR(YAML_READER_CONFIGURATION, "NOT VALID VALUE | SQL " << RECORDER_RESOURCE_LIMITS_SIZE_TOLERANCE_TAG << " must be greater than the minimum value accepted. Defaulting to (Mb): " << resource_limits_struct.size_tolerance_ / (1024 * 1024));
        else
            resource_limits_struct.size_tolerance_ = eprosima::utils::to_bytes(size_tolerance_str);
    }
}



bool ResourceLimitsConfiguration::are_limits_valid(
    utils::Formatter& error_msg,
    bool safety_margin)
{
    if (resource_limits_struct.max_size_ > 0)
    {
        if (resource_limits_struct.max_file_size_ == 0)
        {
            error_msg << "The max file size cannot be unlimited when the max size is limited.";
            return false;
        }

        if (resource_limits_struct.max_size_ < resource_limits_struct.max_file_size_)
        {
            error_msg << "The max size cannot be lower than the max file size.";
            return false;
        }
    }

    if (resource_limits_struct.file_rotation_)
    {
        if (resource_limits_struct.max_file_size_ == 0)
        {
            error_msg << "The max file size cannot be unlimited when file rotation is enabled.";
            return false;
        }

        if (resource_limits_struct.max_size_ == 0 && !safety_margin)
        {
            error_msg << "Both max size and safety_margin cannot be unlimited when file rotation is enabled.";
            return false;
        }
    }

    return true;
}

} /* namespace yaml */
} /* namespace ddsrecorder */
} /* namespace eprosima */
