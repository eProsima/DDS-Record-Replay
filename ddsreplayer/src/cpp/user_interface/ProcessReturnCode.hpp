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
 * @file ProcessReturnCode.hpp
 *
 */

#pragma once

namespace eprosima {
namespace ddsrecorder {
namespace replayer {

enum class ProcessReturnCode : int
{
    success = 0,
    help_argument = 1,
    version_argument = 2,
    incorrect_argument = 10,
    required_argument_failed = 11,
    execution_failed = 20,
};

} /* namespace replayer */
} /* namespace ddsrecorder */
} /* namespace eprosima */
