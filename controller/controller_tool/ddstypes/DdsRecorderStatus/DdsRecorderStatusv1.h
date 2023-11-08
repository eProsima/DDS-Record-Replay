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
 * @file DdsRecorderStatus.h
 * This header file contains the declaration of the described types in the IDL file.
 *
 * This file was generated by the tool fastddsgen.
 */

#include <fastcdr/config.h>

#if FASTCDR_VERSION_MAJOR == 1

#ifndef _FAST_DDS_GENERATED_DDSRECORDERSTATUS_H_
#define _FAST_DDS_GENERATED_DDSRECORDERSTATUS_H_


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
#if defined(DDSRECORDERSTATUS_SOURCE)
#define DDSRECORDERSTATUS_DllAPI __declspec( dllexport )
#else
#define DDSRECORDERSTATUS_DllAPI __declspec( dllimport )
#endif // DDSRECORDERSTATUS_SOURCE
#else
#define DDSRECORDERSTATUS_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define DDSRECORDERSTATUS_DllAPI
#endif // _WIN32

namespace eprosima {
namespace fastcdr {
class Cdr;
} // namespace fastcdr
} // namespace eprosima





/*!
 * @brief This class represents the structure DdsRecorderStatus defined by the user in the IDL file.
 * @ingroup DdsRecorderStatus
 */
class DdsRecorderStatus
{
public:

    /*!
     * @brief Default constructor.
     */
    eProsima_user_DllExport DdsRecorderStatus();

    /*!
     * @brief Default destructor.
     */
    eProsima_user_DllExport ~DdsRecorderStatus();

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object DdsRecorderStatus that will be copied.
     */
    eProsima_user_DllExport DdsRecorderStatus(
            const DdsRecorderStatus& x);

    /*!
     * @brief Move constructor.
     * @param x Reference to the object DdsRecorderStatus that will be copied.
     */
    eProsima_user_DllExport DdsRecorderStatus(
            DdsRecorderStatus&& x) noexcept;

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object DdsRecorderStatus that will be copied.
     */
    eProsima_user_DllExport DdsRecorderStatus& operator =(
            const DdsRecorderStatus& x);

    /*!
     * @brief Move assignment.
     * @param x Reference to the object DdsRecorderStatus that will be copied.
     */
    eProsima_user_DllExport DdsRecorderStatus& operator =(
            DdsRecorderStatus&& x) noexcept;

    /*!
     * @brief Comparison operator.
     * @param x DdsRecorderStatus object to compare.
     */
    eProsima_user_DllExport bool operator ==(
            const DdsRecorderStatus& x) const;

    /*!
     * @brief Comparison operator.
     * @param x DdsRecorderStatus object to compare.
     */
    eProsima_user_DllExport bool operator !=(
            const DdsRecorderStatus& x) const;

    /*!
     * @brief This function copies the value in member previous
     * @param _previous New value to be copied in member previous
     */
    eProsima_user_DllExport void previous(
            const std::string& _previous);

    /*!
     * @brief This function moves the value in member previous
     * @param _previous New value to be moved in member previous
     */
    eProsima_user_DllExport void previous(
            std::string&& _previous);

    /*!
     * @brief This function returns a constant reference to member previous
     * @return Constant reference to member previous
     */
    eProsima_user_DllExport const std::string& previous() const;

    /*!
     * @brief This function returns a reference to member previous
     * @return Reference to member previous
     */
    eProsima_user_DllExport std::string& previous();


    /*!
     * @brief This function copies the value in member current
     * @param _current New value to be copied in member current
     */
    eProsima_user_DllExport void current(
            const std::string& _current);

    /*!
     * @brief This function moves the value in member current
     * @param _current New value to be moved in member current
     */
    eProsima_user_DllExport void current(
            std::string&& _current);

    /*!
     * @brief This function returns a constant reference to member current
     * @return Constant reference to member current
     */
    eProsima_user_DllExport const std::string& current() const;

    /*!
     * @brief This function returns a reference to member current
     * @return Reference to member current
     */
    eProsima_user_DllExport std::string& current();


    /*!
     * @brief This function copies the value in member info
     * @param _info New value to be copied in member info
     */
    eProsima_user_DllExport void info(
            const std::string& _info);

    /*!
     * @brief This function moves the value in member info
     * @param _info New value to be moved in member info
     */
    eProsima_user_DllExport void info(
            std::string&& _info);

    /*!
     * @brief This function returns a constant reference to member info
     * @return Constant reference to member info
     */
    eProsima_user_DllExport const std::string& info() const;

    /*!
     * @brief This function returns a reference to member info
     * @return Reference to member info
     */
    eProsima_user_DllExport std::string& info();


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
            const DdsRecorderStatus& data,
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

    std::string m_previous;
    std::string m_current;
    std::string m_info;

};


#endif // _FAST_DDS_GENERATED_DDSRECORDERSTATUS_H_



#endif // FASTCDR_VERSION_MAJOR == 1
