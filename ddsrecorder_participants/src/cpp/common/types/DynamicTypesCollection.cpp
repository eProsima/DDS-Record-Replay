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

/*!
 * @file DynamicTypesCollection.cpp
 * This source file contains the definition of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifdef _WIN32
// Remove linker warning LNK4221 on Visual Studio
namespace {
char dummy;
}  // namespace
#endif  // _WIN32

#include <ddsrecorder_participants/common/types/DynamicTypesCollection.hpp>
#include <fastcdr/Cdr.h>

#include <fastcdr/exceptions/BadParamException.h>

#include <utility>

namespace eprosima {
namespace ddsrecorder {
namespace participants {

using namespace eprosima::fastcdr::exception;

#define DynamicTypesCollection_max_cdr_typesize 78004ULL;
#define DynamicType_max_cdr_typesize 780ULL;
#define DynamicTypesCollection_max_key_cdr_typesize 0ULL;
#define DynamicType_max_key_cdr_typesize 0ULL;

DynamicType::DynamicType()
{
    // string m_type_name
    m_type_name = "";

    // string m_type_information
    m_type_information = "";

    // string m_type_object
    m_type_object = "";
}

DynamicType::~DynamicType()
{
}

DynamicType::DynamicType(
        const DynamicType& x)
{
    m_type_name = x.m_type_name;
    m_type_information = x.m_type_information;
    m_type_object = x.m_type_object;
}

DynamicType::DynamicType(
        DynamicType&& x) noexcept
{
    m_type_name = std::move(x.m_type_name);
    m_type_information = std::move(x.m_type_information);
    m_type_object = std::move(x.m_type_object);
}

DynamicType& DynamicType::operator =(
        const DynamicType& x)
{
    m_type_name = x.m_type_name;
    m_type_information = x.m_type_information;
    m_type_object = x.m_type_object;

    return *this;
}

DynamicType& DynamicType::operator =(
        DynamicType&& x) noexcept
{
    m_type_name = std::move(x.m_type_name);
    m_type_information = std::move(x.m_type_information);
    m_type_object = std::move(x.m_type_object);

    return *this;
}

bool DynamicType::operator ==(
        const DynamicType& x) const
{
    return (m_type_name == x.m_type_name && m_type_information == x.m_type_information &&
           m_type_object == x.m_type_object);
}

bool DynamicType::operator !=(
        const DynamicType& x) const
{
    return !(*this == x);
}

size_t DynamicType::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);
    return DynamicType_max_cdr_typesize;
}

size_t DynamicType::getCdrSerializedSize(
        const DynamicType& data,
        size_t current_alignment)
{
    static_cast<void>(data);
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.type_name().size() + 1;
    current_alignment += 4 +
            eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.type_information().size() + 1;
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.type_object().size() + 1;

    return current_alignment - initial_alignment;
}

void DynamicType::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{
    scdr << m_type_name.c_str(); scdr << m_type_information.c_str(); scdr << m_type_object.c_str();
}

void DynamicType::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{
    dcdr >> m_type_name;
    dcdr >> m_type_information;
    dcdr >> m_type_object;
}

/*!
 * @brief This function copies the value in member type_name
 * @param _type_name New value to be copied in member type_name
 */
void DynamicType::type_name(
        const std::string& _type_name)
{
    m_type_name = _type_name;
}

/*!
 * @brief This function moves the value in member type_name
 * @param _type_name New value to be moved in member type_name
 */
void DynamicType::type_name(
        std::string&& _type_name)
{
    m_type_name = std::move(_type_name);
}

/*!
 * @brief This function returns a constant reference to member type_name
 * @return Constant reference to member type_name
 */
const std::string& DynamicType::type_name() const
{
    return m_type_name;
}

/*!
 * @brief This function returns a reference to member type_name
 * @return Reference to member type_name
 */
std::string& DynamicType::type_name()
{
    return m_type_name;
}

/*!
 * @brief This function copies the value in member type_information
 * @param _type_information New value to be copied in member type_information
 */
void DynamicType::type_information(
        const std::string& _type_information)
{
    m_type_information = _type_information;
}

/*!
 * @brief This function moves the value in member type_information
 * @param _type_information New value to be moved in member type_information
 */
void DynamicType::type_information(
        std::string&& _type_information)
{
    m_type_information = std::move(_type_information);
}

/*!
 * @brief This function returns a constant reference to member type_information
 * @return Constant reference to member type_information
 */
const std::string& DynamicType::type_information() const
{
    return m_type_information;
}

/*!
 * @brief This function returns a reference to member type_information
 * @return Reference to member type_information
 */
std::string& DynamicType::type_information()
{
    return m_type_information;
}

/*!
 * @brief This function copies the value in member type_object
 * @param _type_object New value to be copied in member type_object
 */
void DynamicType::type_object(
        const std::string& _type_object)
{
    m_type_object = _type_object;
}

/*!
 * @brief This function moves the value in member type_object
 * @param _type_object New value to be moved in member type_object
 */
void DynamicType::type_object(
        std::string&& _type_object)
{
    m_type_object = std::move(_type_object);
}

/*!
 * @brief This function returns a constant reference to member type_object
 * @return Constant reference to member type_object
 */
const std::string& DynamicType::type_object() const
{
    return m_type_object;
}

/*!
 * @brief This function returns a reference to member type_object
 * @return Reference to member type_object
 */
std::string& DynamicType::type_object()
{
    return m_type_object;
}

size_t DynamicType::getKeyMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);
    return DynamicType_max_key_cdr_typesize;
}

bool DynamicType::isKeyDefined()
{
    return false;
}

void DynamicType::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    static_cast<void>(scdr);
}

DynamicTypesCollection::DynamicTypesCollection()
{
}

DynamicTypesCollection::~DynamicTypesCollection()
{
}

DynamicTypesCollection::DynamicTypesCollection(
        const DynamicTypesCollection& x)
{
    m_dynamic_types = x.m_dynamic_types;
}

DynamicTypesCollection::DynamicTypesCollection(
        DynamicTypesCollection&& x) noexcept
{
    m_dynamic_types = std::move(x.m_dynamic_types);
}

DynamicTypesCollection& DynamicTypesCollection::operator =(
        const DynamicTypesCollection& x)
{
    m_dynamic_types = x.m_dynamic_types;

    return *this;
}

DynamicTypesCollection& DynamicTypesCollection::operator =(
        DynamicTypesCollection&& x) noexcept
{
    m_dynamic_types = std::move(x.m_dynamic_types);

    return *this;
}

bool DynamicTypesCollection::operator ==(
        const DynamicTypesCollection& x) const
{
    return (m_dynamic_types == x.m_dynamic_types);
}

bool DynamicTypesCollection::operator !=(
        const DynamicTypesCollection& x) const
{
    return !(*this == x);
}

size_t DynamicTypesCollection::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);
    return DynamicTypesCollection_max_cdr_typesize;
}

size_t DynamicTypesCollection::getCdrSerializedSize(
        const DynamicTypesCollection& data,
        size_t current_alignment)
{
    static_cast<void>(data);
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    for (size_t a = 0; a < data.dynamic_types().size(); ++a)
    {
        current_alignment += DynamicType::getCdrSerializedSize(data.dynamic_types().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void DynamicTypesCollection::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{
    scdr << m_dynamic_types;
}

void DynamicTypesCollection::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{
    dcdr >> m_dynamic_types;
}

/*!
 * @brief This function copies the value in member dynamic_types
 * @param _dynamic_types New value to be copied in member dynamic_types
 */
void DynamicTypesCollection::dynamic_types(
        const std::vector<DynamicType>& _dynamic_types)
{
    m_dynamic_types = _dynamic_types;
}

/*!
 * @brief This function moves the value in member dynamic_types
 * @param _dynamic_types New value to be moved in member dynamic_types
 */
void DynamicTypesCollection::dynamic_types(
        std::vector<DynamicType>&& _dynamic_types)
{
    m_dynamic_types = std::move(_dynamic_types);
}

/*!
 * @brief This function returns a constant reference to member dynamic_types
 * @return Constant reference to member dynamic_types
 */
const std::vector<DynamicType>& DynamicTypesCollection::dynamic_types() const
{
    return m_dynamic_types;
}

/*!
 * @brief This function returns a reference to member dynamic_types
 * @return Reference to member dynamic_types
 */
std::vector<DynamicType>& DynamicTypesCollection::dynamic_types()
{
    return m_dynamic_types;
}

size_t DynamicTypesCollection::getKeyMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);
    return DynamicTypesCollection_max_key_cdr_typesize;
}

bool DynamicTypesCollection::isKeyDefined()
{
    return false;
}

void DynamicTypesCollection::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    static_cast<void>(scdr);
}

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
