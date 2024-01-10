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
 * @file DdsRecorderStatus.cxx
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

#include "DdsRecorderStatus.h"

#if FASTCDR_VERSION_MAJOR > 1

#include <fastcdr/Cdr.h>


#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

#include <utility>




DdsRecorderStatus::DdsRecorderStatus()
{
}

DdsRecorderStatus::~DdsRecorderStatus()
{
}

DdsRecorderStatus::DdsRecorderStatus(
        const DdsRecorderStatus& x)
{
    m_previous = x.m_previous;
    m_current = x.m_current;
    m_info = x.m_info;
}

DdsRecorderStatus::DdsRecorderStatus(
        DdsRecorderStatus&& x) noexcept
{
    m_previous = std::move(x.m_previous);
    m_current = std::move(x.m_current);
    m_info = std::move(x.m_info);
}

DdsRecorderStatus& DdsRecorderStatus::operator =(
        const DdsRecorderStatus& x)
{

    m_previous = x.m_previous;
    m_current = x.m_current;
    m_info = x.m_info;
    return *this;
}

DdsRecorderStatus& DdsRecorderStatus::operator =(
        DdsRecorderStatus&& x) noexcept
{

    m_previous = std::move(x.m_previous);
    m_current = std::move(x.m_current);
    m_info = std::move(x.m_info);
    return *this;
}

bool DdsRecorderStatus::operator ==(
        const DdsRecorderStatus& x) const
{
    return (m_previous == x.m_previous &&
           m_current == x.m_current &&
           m_info == x.m_info);
}

bool DdsRecorderStatus::operator !=(
        const DdsRecorderStatus& x) const
{
    return !(*this == x);
}

/*!
 * @brief This function copies the value in member previous
 * @param _previous New value to be copied in member previous
 */
void DdsRecorderStatus::previous(
        const std::string& _previous)
{
    m_previous = _previous;
}

/*!
 * @brief This function moves the value in member previous
 * @param _previous New value to be moved in member previous
 */
void DdsRecorderStatus::previous(
        std::string&& _previous)
{
    m_previous = std::move(_previous);
}

/*!
 * @brief This function returns a constant reference to member previous
 * @return Constant reference to member previous
 */
const std::string& DdsRecorderStatus::previous() const
{
    return m_previous;
}

/*!
 * @brief This function returns a reference to member previous
 * @return Reference to member previous
 */
std::string& DdsRecorderStatus::previous()
{
    return m_previous;
}

/*!
 * @brief This function copies the value in member current
 * @param _current New value to be copied in member current
 */
void DdsRecorderStatus::current(
        const std::string& _current)
{
    m_current = _current;
}

/*!
 * @brief This function moves the value in member current
 * @param _current New value to be moved in member current
 */
void DdsRecorderStatus::current(
        std::string&& _current)
{
    m_current = std::move(_current);
}

/*!
 * @brief This function returns a constant reference to member current
 * @return Constant reference to member current
 */
const std::string& DdsRecorderStatus::current() const
{
    return m_current;
}

/*!
 * @brief This function returns a reference to member current
 * @return Reference to member current
 */
std::string& DdsRecorderStatus::current()
{
    return m_current;
}

/*!
 * @brief This function copies the value in member info
 * @param _info New value to be copied in member info
 */
void DdsRecorderStatus::info(
        const std::string& _info)
{
    m_info = _info;
}

/*!
 * @brief This function moves the value in member info
 * @param _info New value to be moved in member info
 */
void DdsRecorderStatus::info(
        std::string&& _info)
{
    m_info = std::move(_info);
}

/*!
 * @brief This function returns a constant reference to member info
 * @return Constant reference to member info
 */
const std::string& DdsRecorderStatus::info() const
{
    return m_info;
}

/*!
 * @brief This function returns a reference to member info
 * @return Reference to member info
 */
std::string& DdsRecorderStatus::info()
{
    return m_info;
}

// Include auxiliary functions like for serializing/deserializing.
#include "DdsRecorderStatusCdrAux.ipp"

#endif // FASTCDR_VERSION_MAJOR > 1
