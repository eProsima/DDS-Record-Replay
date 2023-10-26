// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*!
 * @file DdsRecorderCommand.cpp
 * This source file contains the implementation of the described types in the IDL file.
 *
 * This file was generated by the tool fastddsgen.
 */

#ifdef _WIN32
// Remove linker warning LNK4221 on Visual Studio
namespace {
char dummy;
}  // namespace
#endif  // _WIN32

#include "DdsRecorderCommand.h"
#include <fastdds/rtps/common/CdrSerialization.hpp>

#include <utility>

// Include auxiliary functions like for serializing/deserializing.
#include "DdsRecorderCommandCdrAux.ipp"

using namespace eprosima::fastcdr::exception;


DdsRecorderCommand::DdsRecorderCommand()
{

}

DdsRecorderCommand::~DdsRecorderCommand()
{
}

DdsRecorderCommand::DdsRecorderCommand(
        const DdsRecorderCommand& x)
{
    m_command = x.m_command;
    m_args = x.m_args;
}

DdsRecorderCommand::DdsRecorderCommand(
        DdsRecorderCommand&& x) noexcept
{
    m_command = std::move(x.m_command);
    m_args = std::move(x.m_args);
}

DdsRecorderCommand& DdsRecorderCommand::operator =(
        const DdsRecorderCommand& x)
{

    m_command = x.m_command;
    m_args = x.m_args;
    return *this;
}

DdsRecorderCommand& DdsRecorderCommand::operator =(
        DdsRecorderCommand&& x) noexcept
{

    m_command = std::move(x.m_command);
    m_args = std::move(x.m_args);
    return *this;
}

bool DdsRecorderCommand::operator ==(
        const DdsRecorderCommand& x) const
{
    return (m_command == x.m_command &&
           m_args == x.m_args);
}

bool DdsRecorderCommand::operator !=(
        const DdsRecorderCommand& x) const
{
    return !(*this == x);
}

void DdsRecorderCommand::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{
    eprosima::fastcdr::serialize(scdr, *this);
}

void DdsRecorderCommand::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{
    eprosima::fastcdr::deserialize(dcdr, *this);
}


/*!
 * @brief This function copies the value in member command
 * @param _command New value to be copied in member command
 */
void DdsRecorderCommand::command(
        const std::string& _command)
{
    m_command = _command;
}

/*!
 * @brief This function moves the value in member command
 * @param _command New value to be moved in member command
 */
void DdsRecorderCommand::command(
        std::string&& _command)
{
    m_command = std::move(_command);
}

/*!
 * @brief This function returns a constant reference to member command
 * @return Constant reference to member command
 */
const std::string& DdsRecorderCommand::command() const
{
    return m_command;
}

/*!
 * @brief This function returns a reference to member command
 * @return Reference to member command
 */
std::string& DdsRecorderCommand::command()
{
    return m_command;
}


/*!
 * @brief This function copies the value in member args
 * @param _args New value to be copied in member args
 */
void DdsRecorderCommand::args(
        const std::string& _args)
{
    m_args = _args;
}

/*!
 * @brief This function moves the value in member args
 * @param _args New value to be moved in member args
 */
void DdsRecorderCommand::args(
        std::string&& _args)
{
    m_args = std::move(_args);
}

/*!
 * @brief This function returns a constant reference to member args
 * @return Constant reference to member args
 */
const std::string& DdsRecorderCommand::args() const
{
    return m_args;
}

/*!
 * @brief This function returns a reference to member args
 * @return Reference to member args
 */
std::string& DdsRecorderCommand::args()
{
    return m_args;
}

