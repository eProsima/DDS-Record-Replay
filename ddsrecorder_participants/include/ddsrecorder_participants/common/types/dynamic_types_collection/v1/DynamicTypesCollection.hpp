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
 * @file DynamicTypesCollection.h
 * This header file contains the declaration of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#pragma once

#include <fastrtps/utils/fixed_size_string.hpp>

#include <stdint.h>
#include <array>
#include <string>
#include <vector>
#include <map>
#include <bitset>

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec( dllexport )
#else
#define eProsima_user_DllExport
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define eProsima_user_DllExport
#endif  // _WIN32

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#if defined(DYNAMICTYPESCOLLECTION_SOURCE)
#define DYNAMICTYPESCOLLECTION_DllAPI __declspec( dllexport )
#else
#define DYNAMICTYPESCOLLECTION_DllAPI __declspec( dllimport )
#endif // DYNAMICTYPESCOLLECTION_SOURCE
#else
#define DYNAMICTYPESCOLLECTION_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define DYNAMICTYPESCOLLECTION_DllAPI
#endif // _WIN32

namespace eprosima {
namespace fastcdr {
class Cdr;
} // namespace fastcdr
} // namespace eprosima

namespace eprosima {
namespace ddsrecorder {
namespace participants {

/*!
 * @brief This class represents the structure DynamicType defined by the user in the IDL file.
 * @ingroup DynamicTypesCollection
 */
class DynamicType
{
public:

    /*!
     * @brief Default constructor.
     */
    eProsima_user_DllExport DynamicType();

    /*!
     * @brief Default destructor.
     */
    eProsima_user_DllExport ~DynamicType();

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object DynamicType that will be copied.
     */
    eProsima_user_DllExport DynamicType(
            const DynamicType& x);

    /*!
     * @brief Move constructor.
     * @param x Reference to the object DynamicType that will be copied.
     */
    eProsima_user_DllExport DynamicType(
            DynamicType&& x) noexcept;

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object DynamicType that will be copied.
     */
    eProsima_user_DllExport DynamicType& operator =(
            const DynamicType& x);

    /*!
     * @brief Move assignment.
     * @param x Reference to the object DynamicType that will be copied.
     */
    eProsima_user_DllExport DynamicType& operator =(
            DynamicType&& x) noexcept;

    /*!
     * @brief Comparison operator.
     * @param x DynamicType object to compare.
     */
    eProsima_user_DllExport bool operator ==(
            const DynamicType& x) const;

    /*!
     * @brief Comparison operator.
     * @param x DynamicType object to compare.
     */
    eProsima_user_DllExport bool operator !=(
            const DynamicType& x) const;

    /*!
     * @brief This function copies the value in member type_name
     * @param _type_name New value to be copied in member type_name
     */
    eProsima_user_DllExport void type_name(
            const std::string& _type_name);

    /*!
     * @brief This function moves the value in member type_name
     * @param _type_name New value to be moved in member type_name
     */
    eProsima_user_DllExport void type_name(
            std::string&& _type_name);

    /*!
     * @brief This function returns a constant reference to member type_name
     * @return Constant reference to member type_name
     */
    eProsima_user_DllExport const std::string& type_name() const;

    /*!
     * @brief This function returns a reference to member type_name
     * @return Reference to member type_name
     */
    eProsima_user_DllExport std::string& type_name();
    /*!
     * @brief This function copies the value in member type_information
     * @param _type_information New value to be copied in member type_information
     */
    eProsima_user_DllExport void type_information(
            const std::string& _type_information);

    /*!
     * @brief This function moves the value in member type_information
     * @param _type_information New value to be moved in member type_information
     */
    eProsima_user_DllExport void type_information(
            std::string&& _type_information);

    /*!
     * @brief This function returns a constant reference to member type_information
     * @return Constant reference to member type_information
     */
    eProsima_user_DllExport const std::string& type_information() const;

    /*!
     * @brief This function returns a reference to member type_information
     * @return Reference to member type_information
     */
    eProsima_user_DllExport std::string& type_information();
    /*!
     * @brief This function copies the value in member type_object
     * @param _type_object New value to be copied in member type_object
     */
    eProsima_user_DllExport void type_object(
            const std::string& _type_object);

    /*!
     * @brief This function moves the value in member type_object
     * @param _type_object New value to be moved in member type_object
     */
    eProsima_user_DllExport void type_object(
            std::string&& _type_object);

    /*!
     * @brief This function returns a constant reference to member type_object
     * @return Constant reference to member type_object
     */
    eProsima_user_DllExport const std::string& type_object() const;

    /*!
     * @brief This function returns a reference to member type_object
     * @return Reference to member type_object
     */
    eProsima_user_DllExport std::string& type_object();

    /*!
     * @brief This function returns the maximum serialized size of an object
     * depending on the buffer alignment.
     * @param current_alignment Buffer alignment.
     * @return Maximum serialized size.
     */
    eProsima_user_DllExport static size_t getMaxCdrSerializedSize(
            size_t current_alignment = 0);

    /*!
     * @brief This function returns the serialized size of a data depending on the buffer alignment.
     * @param data Data which is calculated its serialized size.
     * @param current_alignment Buffer alignment.
     * @return Serialized size.
     */
    eProsima_user_DllExport static size_t getCdrSerializedSize(
            const DynamicType& data,
            size_t current_alignment = 0);


    /*!
     * @brief This function serializes an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    eProsima_user_DllExport void serialize(
            eprosima::fastcdr::Cdr& cdr) const;

    /*!
     * @brief This function deserializes an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    eProsima_user_DllExport void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    /*!
     * @brief This function returns the maximum serialized size of the Key of an object
     * depending on the buffer alignment.
     * @param current_alignment Buffer alignment.
     * @return Maximum serialized size.
     */
    eProsima_user_DllExport static size_t getKeyMaxCdrSerializedSize(
            size_t current_alignment = 0);

    /*!
     * @brief This function tells you if the Key has been defined for this type
     */
    eProsima_user_DllExport static bool isKeyDefined();

    /*!
     * @brief This function serializes the key members of an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    eProsima_user_DllExport void serializeKey(
            eprosima::fastcdr::Cdr& cdr) const;

private:

    std::string m_type_name;
    std::string m_type_information;
    std::string m_type_object;

};
/*!
 * @brief This class represents the structure DynamicTypesCollection defined by the user in the IDL file.
 * @ingroup DynamicTypesCollection
 */
class DynamicTypesCollection
{
public:

    /*!
     * @brief Default constructor.
     */
    eProsima_user_DllExport DynamicTypesCollection();

    /*!
     * @brief Default destructor.
     */
    eProsima_user_DllExport ~DynamicTypesCollection();

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object DynamicTypesCollection that will be copied.
     */
    eProsima_user_DllExport DynamicTypesCollection(
            const DynamicTypesCollection& x);

    /*!
     * @brief Move constructor.
     * @param x Reference to the object DynamicTypesCollection that will be copied.
     */
    eProsima_user_DllExport DynamicTypesCollection(
            DynamicTypesCollection&& x) noexcept;

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object DynamicTypesCollection that will be copied.
     */
    eProsima_user_DllExport DynamicTypesCollection& operator =(
            const DynamicTypesCollection& x);

    /*!
     * @brief Move assignment.
     * @param x Reference to the object DynamicTypesCollection that will be copied.
     */
    eProsima_user_DllExport DynamicTypesCollection& operator =(
            DynamicTypesCollection&& x) noexcept;

    /*!
     * @brief Comparison operator.
     * @param x DynamicTypesCollection object to compare.
     */
    eProsima_user_DllExport bool operator ==(
            const DynamicTypesCollection& x) const;

    /*!
     * @brief Comparison operator.
     * @param x DynamicTypesCollection object to compare.
     */
    eProsima_user_DllExport bool operator !=(
            const DynamicTypesCollection& x) const;

    /*!
     * @brief This function copies the value in member dynamic_types
     * @param _dynamic_types New value to be copied in member dynamic_types
     */
    eProsima_user_DllExport void dynamic_types(
            const std::vector<DynamicType>& _dynamic_types);

    /*!
     * @brief This function moves the value in member dynamic_types
     * @param _dynamic_types New value to be moved in member dynamic_types
     */
    eProsima_user_DllExport void dynamic_types(
            std::vector<DynamicType>&& _dynamic_types);

    /*!
     * @brief This function returns a constant reference to member dynamic_types
     * @return Constant reference to member dynamic_types
     */
    eProsima_user_DllExport const std::vector<DynamicType>& dynamic_types() const;

    /*!
     * @brief This function returns a reference to member dynamic_types
     * @return Reference to member dynamic_types
     */
    eProsima_user_DllExport std::vector<DynamicType>& dynamic_types();

    /*!
     * @brief This function returns the maximum serialized size of an object
     * depending on the buffer alignment.
     * @param current_alignment Buffer alignment.
     * @return Maximum serialized size.
     */
    eProsima_user_DllExport static size_t getMaxCdrSerializedSize(
            size_t current_alignment = 0);

    /*!
     * @brief This function returns the serialized size of a data depending on the buffer alignment.
     * @param data Data which is calculated its serialized size.
     * @param current_alignment Buffer alignment.
     * @return Serialized size.
     */
    eProsima_user_DllExport static size_t getCdrSerializedSize(
            const DynamicTypesCollection& data,
            size_t current_alignment = 0);


    /*!
     * @brief This function serializes an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    eProsima_user_DllExport void serialize(
            eprosima::fastcdr::Cdr& cdr) const;

    /*!
     * @brief This function deserializes an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    eProsima_user_DllExport void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    /*!
     * @brief This function returns the maximum serialized size of the Key of an object
     * depending on the buffer alignment.
     * @param current_alignment Buffer alignment.
     * @return Maximum serialized size.
     */
    eProsima_user_DllExport static size_t getKeyMaxCdrSerializedSize(
            size_t current_alignment = 0);

    /*!
     * @brief This function tells you if the Key has been defined for this type
     */
    eProsima_user_DllExport static bool isKeyDefined();

    /*!
     * @brief This function serializes the key members of an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    eProsima_user_DllExport void serializeKey(
            eprosima::fastcdr::Cdr& cdr) const;

private:

    std::vector<DynamicType> m_dynamic_types;

};

} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
