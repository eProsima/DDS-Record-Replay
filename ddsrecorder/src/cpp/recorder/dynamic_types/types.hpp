// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file types.hpp
 */

#pragma once

#include <fastrtps/types/DynamicTypePtr.h>

#include <ddsrecorder/types/dds/Data.hpp>
#include <ddsrecorder/types/dds/Guid.hpp>
#include <ddsrecorder/types/topic/dds/DdsTopic.hpp>

#include <efficiency/payload/PayloadPool.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {
namespace recorder {

constexpr const char* TYPE_OBJECT_TOPIC_NAME = "__internal__/type_object";
constexpr const char* TYPE_OBJECT_DATA_TYPE_NAME = "__internal__::type_object";

types::DdsTopic type_object_topic();

bool is_type_object_topic(const types::DdsTopic& topic);

types::Guid new_unique_guid();

std::unique_ptr<types::DataReceived> string_serialization(
        std::shared_ptr<PayloadPool> payload_pool,
        const std::string& str);

std::string string_deserialization(
        const std::unique_ptr<types::DataReceived>& data);

} /* namespace recorder */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
