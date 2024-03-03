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

#pragma once

#include <ddspipe_core/configuration/CommandlineArgs.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace yaml {

/*
 * Struct to parse the executable arguments
 */
struct CommandlineArgsReplayer : public ddspipe::core::CommandlineArgs
{

    /////////////////////////
    // CONSTRUCTORS
    /////////////////////////

    CommandlineArgsReplayer();

    /////////////////////////
    // METHODS
    /////////////////////////

    /**
     * @brief \c is_valid method.
     */
    bool is_valid(
            utils::Formatter& error_msg) const noexcept override;

    /////////////////////////
    // VARIABLES
    /////////////////////////

    // Input file path
    std::string input_file{""};
};

} /* namespace yaml */
} /* namespace ddsrecorder */
} /* namespace eprosima */
