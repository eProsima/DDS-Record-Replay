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
 * @file constants.hpp
 */

#pragma once

namespace eprosima {
namespace ddsrecorder {
namespace participants {

// QoS serialization
constexpr const char* QOS_SERIALIZATION_QOS("qos");
constexpr const char* QOS_SERIALIZATION_RELIABILITY("reliability");
constexpr const char* QOS_SERIALIZATION_DURABILITY("durability");
constexpr const char* QOS_SERIALIZATION_OWNERSHIP("ownership");
constexpr const char* QOS_SERIALIZATION_KEYED("keyed");

// Dynamic types serialization
constexpr const char* DYNAMIC_TYPES_ATTACHMENT_NAME("dynamic_types");

// ROS 2 Types metadata
constexpr const char* ROS2_TYPES("ros2-types");



// Version metadata
constexpr const char* VERSION_METADATA_NAME("version");
constexpr const char* VERSION_METADATA_RELEASE("release");
constexpr const char* VERSION_METADATA_COMMIT("commit");
// Partitions metadata
constexpr const char* PARTITIONS("partitions"); // partitions of the channel
constexpr const char* VERSION_METADATA_MESSAGE_NAME("messages_guid"); // the guid associated with a message
constexpr const char* VERSION_METADATA_MESSAGE_INDX_NAME("messages_guid_indx"); // the guid associated with a message

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
