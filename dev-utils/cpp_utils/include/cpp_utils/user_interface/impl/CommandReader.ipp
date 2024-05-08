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

#pragma once

#include <cpp_utils/utils.hpp>

namespace eprosima {
namespace utils {

template <typename CommandEnum>
CommandReader<CommandEnum>::CommandReader(
        const EnumBuilder<CommandEnum>& builder,
        std::istream& source /* = std::cin */)
    : builder_(builder)
    , stdin_handler_(
        [this](std::string st)
        {
            this->read_command_callback_(st);
        },
        true,
        0,
        source)
    , commands_read_(0, true)
{
    // Do nothing
}

template <typename CommandEnum>
bool CommandReader<CommandEnum>::read_next_command(
        Command<CommandEnum>& command)
{
    stdin_handler_.read_one_more_line();
    std::string full_command = commands_read_.consume();

    // Divide command
    command.arguments = utils::split_string(full_command, " ");

    // Check if command exists
    // The args are already set in command, and the enum value will be set string_to_enumeration
    return builder_.string_to_enumeration(command.arguments[0], command.command);
}

template <typename CommandEnum>
void CommandReader<CommandEnum>::read_command_callback_(
        std::string command_read)
{
    commands_read_.produce(command_read);
}

} /* namespace utils */
} /* namespace eprosima */

// Include implementation template file
#include <cpp_utils/user_interface/impl/CommandReader.ipp>
