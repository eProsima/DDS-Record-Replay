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
 * @file all_types.hpp
 */

/*
 * USEFUL COMMAND
 *
 * for TYPE in hello_world numeric_array char_sequence basic_struct do; ${FASTDDSGEN_WS}/scripts/fastddsgen -replace -d ${WS}/src/recorder/ddsrecorder/test/unittest/dynamic_types/types/type_objects/ -typeobject -cs ${WS}/src/recorder/ddsrecorder/test/unittest/dynamic_types/types/idls/${TYPE}.idl; done
 */

#pragma once

#include <cpp_utils/macros/custom_enumeration.hpp>
#include <cpp_utils/utils.hpp>

#include <fastrtps/types/DynamicTypePtr.h>

#include <recorder/dynamic_types/utils.hpp>

#include "type_objects/basic_structTypeObject.h"
#include "type_objects/char_sequenceTypeObject.h"
#include "type_objects/hello_worldTypeObject.h"
#include "type_objects/numeric_arrayTypeObject.h"

namespace test {

ENUMERATION_BUILDER(
    SupportedType,
    hello_world,
    numeric_array,
    char_sequence,
    basic_struct
);

eprosima::fastrtps::types::DynamicType_ptr get_dynamic_type(SupportedType type)
{
    registerbasic_structTypes();
    registerchar_sequenceTypes();
    registerhello_worldTypes();
    registernumeric_arrayTypes();

    return eprosima::ddsrecorder::core::recorder::dynamic_type_from_name(to_string(type));
}

} /* namespace test */
