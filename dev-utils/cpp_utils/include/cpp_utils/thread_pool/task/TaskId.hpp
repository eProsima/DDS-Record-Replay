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
 * @file TaskId.hpp
 *
 * This file contains class Task definition.
 */

#pragma once

#include <cpp_utils/library/library_dll.h>

#include <functional>

namespace eprosima {
namespace utils {

//! Type of the task ID.
using TaskId = unsigned int;

/**
 * @brief Get a new unique task ID.
 *
 * It uses a random number to generate a new ID.
 *
 * @return new unique TaskId
 */
CPP_UTILS_DllAPI TaskId new_unique_task_id();

} /* namespace utils */
} /* namespace eprosima */


