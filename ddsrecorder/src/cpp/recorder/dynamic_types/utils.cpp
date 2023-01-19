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
 * @file utils.cpp
 */

#include <fastrtps/types/TypeObjectFactory.h>

#include <ddsrecorder/types/dds/Data.hpp>

#include <recorder/dynamic_types/types.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {
namespace recorder {

const fastrtps::types::TypeObject* type_object_from_name(
        const std::string& type_name)
{
    auto type_obj_factory = eprosima::fastrtps::types::TypeObjectFactory::get_instance();
    auto type_id = type_obj_factory->get_type_identifier(type_name, true);

    if (type_id == nullptr)
    {
        return nullptr;
    }

    return type_obj_factory->get_type_object(type_id);
}

const fastrtps::types::DynamicType_ptr dynamic_type_from_name(
        const std::string& type_name)
{
    auto type_obj_factory = eprosima::fastrtps::types::TypeObjectFactory::get_instance();

    auto type_id = type_obj_factory->get_type_identifier(type_name, true);
    if (type_id == nullptr)
    {
        return fastrtps::types::DynamicType_ptr();
    }

    auto type_obj = type_obj_factory->get_type_object(type_id);
    if (type_obj == nullptr)
    {
        return fastrtps::types::DynamicType_ptr();
    }

    return type_obj_factory->build_dynamic_type(type_name, type_id, type_obj);
}

} /* namespace recorder */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
