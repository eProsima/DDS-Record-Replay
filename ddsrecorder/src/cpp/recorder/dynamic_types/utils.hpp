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
 * @file utils.hpp
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

const fastrtps::types::TypeObject* type_object_from_name(
        const std::string& type_name);

const fastrtps::types::DynamicType_ptr dynamic_type_from_name(
        const std::string& type_name);

} /* namespace recorder */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
