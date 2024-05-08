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

#include <cpp_utils/event/StdinEventHandler.hpp>
#include <cpp_utils/enum/EnumBuilder.hpp>
#include <cpp_utils/wait/DBQueueWaitHandler.hpp>

namespace eprosima {
namespace utils {

/**
 * @brief Data struct that contains a command in a way of enumeration, and arguments in a string way.
 *
 * @tparam CommandEnum enumeration that represent the different commands available.
 */
template <typename CommandEnum>
struct Command
{
    //! Command in the way of a enumeration class.
    CommandEnum command;

    //! Vector with arguments of command in string way
    std::vector<std::string> arguments;
};

/**
 * Class that allows to easily read commands and their arguments from \c std::cin .
 *
 * Giving an enumeration \c CommandEnum that represent the different possible values of commands, and a
 * \c EnumBuilder that transform strings to CommandEnum values, this class read from stdin when call to
 * \c read_next_command and return a command and its arguments, in case the command is parsable to the enum.
 *
 * This class is similar to \c StdinEventHandler but this reads under request,
 * vs the event handler that is asynchronous.
 *
 * @note This class relies on \c StdinEventHandler to read from stdin and in \c EnumBuilder to interpret command.
 */
template <typename CommandEnum>
class CommandReader
{
public:

    /**
     * @brief Construct a new Command Reader object giving an \c EnumBuilder and a source from where to read.
     *
     * @param builder class that allow to convert a string to a enumeration kind.
     * @param source : source \c istream to get data from.
     *
     * @warning using a source different than std::cin is dangerous as the reference is passed forward.
     * However this is very useful for testing purpose.
     */
    CommandReader(
            const EnumBuilder<CommandEnum>& builder,
            std::istream& source = std::cin);

    //! Default dtor
    ~CommandReader() = default;

    //! Illegal copy of this class due to internal EventHandler. This remove other copy or move operators.
    CommandReader(
            const CommandReader& other) = delete;

    /**
     * @brief Read next command written in stdin.
     *
     * @param [out] command command to fill with the data read.
     *
     * @return true if the data read is parsable to a enum value of \c EnumBuilder .
     * @return false otherwise.
     */
    bool read_next_command(
            Command<CommandEnum>& command);

protected:

    /**
     * @brief
     *
     * @param command_read
     */
    void read_command_callback_(
            std::string command_read);

    //! Builder to transform string into a command enum value.
    EnumBuilder<CommandEnum> builder_;

    //! Event Handler used to read from stdin.
    event::StdinEventHandler stdin_handler_;

    /**
     * @brief Consumer where \c stdin_handler_ will produce commands read, and method \c read_next_command will
     * consume next command.
     *
     * @note Using a DBQueueWaitHandler is maybe pretty complex for what is required here, as there will be only one
     * value available at a time. But it is the only implementation of ConsumerWaitHandler so far.
     */
    event::DBQueueWaitHandler<std::string> commands_read_;
};

} /* namespace utils */
} /* namespace eprosima */

// Include implementation template file
#include <cpp_utils/user_interface/impl/CommandReader.ipp>
