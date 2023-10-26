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
 * @file DynamicTypesCollection.cpp
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

#include <ddsrecorder_participants/common/types/DynamicTypesCollection.hpp>
#include <fastdds/rtps/common/CdrSerialization.hpp>

#include <utility>

// Include auxiliary functions like for serializing/deserializing.
#include "DynamicTypesCollectionCdrAux.ipp"

using namespace eprosima::fastcdr::exception;


namespace eprosima {

namespace ddsrecorder {

namespace participants {

eprosima::ddsrecorder::participants::DynamicType::DynamicType()
{

}

eprosima::ddsrecorder::participants::DynamicType::~DynamicType()
{
}

eprosima::ddsrecorder::participants::DynamicType::DynamicType(
        const DynamicType& x)
{
    m_type_name = x.m_type_name;
    m_type_information = x.m_type_information;
    m_type_object = x.m_type_object;
}

eprosima::ddsrecorder::participants::DynamicType::DynamicType(
        DynamicType&& x) noexcept
{
    m_type_name = std::move(x.m_type_name);
    m_type_information = std::move(x.m_type_information);
    m_type_object = std::move(x.m_type_object);
}

eprosima::ddsrecorder::participants::DynamicType& eprosima::ddsrecorder::participants::DynamicType::operator =(
        const DynamicType& x)
{

    m_type_name = x.m_type_name;
    m_type_information = x.m_type_information;
    m_type_object = x.m_type_object;
    return *this;
}

eprosima::ddsrecorder::participants::DynamicType& eprosima::ddsrecorder::participants::DynamicType::operator =(
        DynamicType&& x) noexcept
{

    m_type_name = std::move(x.m_type_name);
    m_type_information = std::move(x.m_type_information);
    m_type_object = std::move(x.m_type_object);
    return *this;
}

bool eprosima::ddsrecorder::participants::DynamicType::operator ==(
        const DynamicType& x) const
{
    return (m_type_name == x.m_type_name &&
           m_type_information == x.m_type_information &&
           m_type_object == x.m_type_object);
}

bool eprosima::ddsrecorder::participants::DynamicType::operator !=(
        const DynamicType& x) const
{
    return !(*this == x);
}

void eprosima::ddsrecorder::participants::DynamicType::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{
    eprosima::fastcdr::serialize(scdr, *this);
}

void eprosima::ddsrecorder::participants::DynamicType::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{
    eprosima::fastcdr::deserialize(dcdr, *this);
}

/*!
 * @brief This function copies the value in member type_name
 * @param _type_name New value to be copied in member type_name
 */
void eprosima::ddsrecorder::participants::DynamicType::type_name(
        const std::string& _type_name)
{
    m_type_name = _type_name;
}

/*!
 * @brief This function moves the value in member type_name
 * @param _type_name New value to be moved in member type_name
 */
void eprosima::ddsrecorder::participants::DynamicType::type_name(
        std::string&& _type_name)
{
    m_type_name = std::move(_type_name);
}

/*!
 * @brief This function returns a constant reference to member type_name
 * @return Constant reference to member type_name
 */
const std::string& eprosima::ddsrecorder::participants::DynamicType::type_name() const
{
    return m_type_name;
}

/*!
 * @brief This function returns a reference to member type_name
 * @return Reference to member type_name
 */
std::string& eprosima::ddsrecorder::participants::DynamicType::type_name()
{
    return m_type_name;
}

/*!
 * @brief This function copies the value in member type_information
 * @param _type_information New value to be copied in member type_information
 */
void eprosima::ddsrecorder::participants::DynamicType::type_information(
        const std::string& _type_information)
{
    m_type_information = _type_information;
}

/*!
 * @brief This function moves the value in member type_information
 * @param _type_information New value to be moved in member type_information
 */
void eprosima::ddsrecorder::participants::DynamicType::type_information(
        std::string&& _type_information)
{
    m_type_information = std::move(_type_information);
}

/*!
 * @brief This function returns a constant reference to member type_information
 * @return Constant reference to member type_information
 */
const std::string& eprosima::ddsrecorder::participants::DynamicType::type_information() const
{
    return m_type_information;
}

/*!
 * @brief This function returns a reference to member type_information
 * @return Reference to member type_information
 */
std::string& eprosima::ddsrecorder::participants::DynamicType::type_information()
{
    return m_type_information;
}

/*!
 * @brief This function copies the value in member type_object
 * @param _type_object New value to be copied in member type_object
 */
void eprosima::ddsrecorder::participants::DynamicType::type_object(
        const std::string& _type_object)
{
    m_type_object = _type_object;
}

/*!
 * @brief This function moves the value in member type_object
 * @param _type_object New value to be moved in member type_object
 */
void eprosima::ddsrecorder::participants::DynamicType::type_object(
        std::string&& _type_object)
{
    m_type_object = std::move(_type_object);
}

/*!
 * @brief This function returns a constant reference to member type_object
 * @return Constant reference to member type_object
 */
const std::string& eprosima::ddsrecorder::participants::DynamicType::type_object() const
{
    return m_type_object;
}

/*!
 * @brief This function returns a reference to member type_object
 * @return Reference to member type_object
 */
std::string& eprosima::ddsrecorder::participants::DynamicType::type_object()
{
    return m_type_object;
}

eprosima::ddsrecorder::participants::DynamicTypesCollection::DynamicTypesCollection()
{

}

eprosima::ddsrecorder::participants::DynamicTypesCollection::~DynamicTypesCollection()
{
}

eprosima::ddsrecorder::participants::DynamicTypesCollection::DynamicTypesCollection(
        const DynamicTypesCollection& x)
{
    m_dynamic_types = x.m_dynamic_types;
}

eprosima::ddsrecorder::participants::DynamicTypesCollection::DynamicTypesCollection(
        DynamicTypesCollection&& x) noexcept
{
    m_dynamic_types = std::move(x.m_dynamic_types);
}

eprosima::ddsrecorder::participants::DynamicTypesCollection& eprosima::ddsrecorder::participants::DynamicTypesCollection
        ::operator =(
        const DynamicTypesCollection& x)
{

    m_dynamic_types = x.m_dynamic_types;
    return *this;
}

eprosima::ddsrecorder::participants::DynamicTypesCollection& eprosima::ddsrecorder::participants::DynamicTypesCollection
        ::operator =(
        DynamicTypesCollection&& x) noexcept
{

    m_dynamic_types = std::move(x.m_dynamic_types);
    return *this;
}

bool eprosima::ddsrecorder::participants::DynamicTypesCollection::operator ==(
        const DynamicTypesCollection& x) const
{
    return (m_dynamic_types == x.m_dynamic_types);
}

bool eprosima::ddsrecorder::participants::DynamicTypesCollection::operator !=(
        const DynamicTypesCollection& x) const
{
    return !(*this == x);
}

void eprosima::ddsrecorder::participants::DynamicTypesCollection::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{
    eprosima::fastcdr::serialize(scdr, *this);
}

void eprosima::ddsrecorder::participants::DynamicTypesCollection::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{
    eprosima::fastcdr::deserialize(dcdr, *this);
}

/*!
 * @brief This function copies the value in member dynamic_types
 * @param _dynamic_types New value to be copied in member dynamic_types
 */
void eprosima::ddsrecorder::participants::DynamicTypesCollection::dynamic_types(
        const std::vector<eprosima::ddsrecorder::participants::DynamicType>& _dynamic_types)
{
    m_dynamic_types = _dynamic_types;
}

/*!
 * @brief This function moves the value in member dynamic_types
 * @param _dynamic_types New value to be moved in member dynamic_types
 */
void eprosima::ddsrecorder::participants::DynamicTypesCollection::dynamic_types(
        std::vector<eprosima::ddsrecorder::participants::DynamicType>&& _dynamic_types)
{
    m_dynamic_types = std::move(_dynamic_types);
}

/*!
 * @brief This function returns a constant reference to member dynamic_types
 * @return Constant reference to member dynamic_types
 */
const std::vector<eprosima::ddsrecorder::participants::DynamicType>& eprosima::ddsrecorder::participants::
        DynamicTypesCollection::dynamic_types() const
{
    return m_dynamic_types;
}

/*!
 * @brief This function returns a reference to member dynamic_types
 * @return Reference to member dynamic_types
 */
std::vector<eprosima::ddsrecorder::participants::DynamicType>& eprosima::ddsrecorder::participants::
        DynamicTypesCollection::dynamic_types()
{
    return m_dynamic_types;
}

} // namespace participants


} // namespace ddsrecorder


} // namespace eprosima
