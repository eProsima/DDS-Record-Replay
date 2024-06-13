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
 * @file constants.hpp
 */

#pragma once

#include <cstdint>


namespace test {

constexpr std::uint32_t DOMAIN = 110;

const auto DDS_TOPIC_NAME = "/dds/topic";
const auto DDS_TYPE_NAME = "HelloWorld";

const auto ROS2_TOPIC_NAME = "rt/topic";
const auto ROS2_TYPE_NAME = "std_msgs::msg::dds_::String_";

} // namespace test
