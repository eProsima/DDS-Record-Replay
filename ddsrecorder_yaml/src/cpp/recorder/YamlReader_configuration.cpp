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
 * @file YamlReader_configuration.cpp
 *
 * Use this file to define tailored validation methods for the DDS Recorder.
 * The methods are already implemented in the DDS Pipe, but they can be overridden here to provide specific validation.
 *
 * The namespace must be the same as the DDS Pipe's YamlReader class.
 *
 * NOTE: Overriding the same validate method in the DDS Recorder and the DDS Replayer would give conflicts. The
 * functions that need to be overridden in both are left commented in both until the DDS Recorder and the DDS Replayer
 * are split up into different products.
 */

#include <set>

#include <ddspipe_core/types/dds/TopicQoS.hpp>

#include <ddspipe_yaml/YamlValidator.hpp>
#include <ddspipe_yaml/yaml_configuration_tags.hpp>

namespace eprosima {
namespace ddspipe {
namespace yaml {

// template <>
// bool YamlValidator::validate<core::types::TopicQoS>(
//         const Yaml& yml,
//         const YamlReaderVersion& /* version */)
// {
//     // The DDS Pipe's QOS_MAX_TX_RATE is invalid in the DDS Recorder.
//     static const std::set<TagType> tags{
//         QOS_TRANSIENT_TAG,
//         QOS_RELIABLE_TAG,
//         QOS_OWNERSHIP_TAG,
//         QOS_PARTITION_TAG,
//         QOS_HISTORY_DEPTH_TAG,
//         QOS_KEYED_TAG,
//         QOS_MAX_RX_RATE_TAG,
//         QOS_DOWNSAMPLING_TAG};

//     return YamlValidator::validate_tags(yml, tags);
// }

} /* namespace yaml */
} /* namespace ddspipe */
} /* namespace eprosima */
