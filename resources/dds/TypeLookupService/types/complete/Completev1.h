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
 * @file Complete.h
 * This header file contains the declaration of the described types in the IDL file.
 *
 * This file was generated by the tool fastddsgen.
 */

#include <fastcdr/config.h>

#if FASTCDR_VERSION_MAJOR == 1

#ifndef _FAST_DDS_GENERATED_COMPLETE_H_
#define _FAST_DDS_GENERATED_COMPLETE_H_


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
#if defined(COMPLETE_SOURCE)
#define COMPLETE_DllAPI __declspec( dllexport )
#else
#define COMPLETE_DllAPI __declspec( dllimport )
#endif // COMPLETE_SOURCE
#else
#define COMPLETE_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define COMPLETE_DllAPI
#endif // _WIN32

namespace eprosima {
namespace fastcdr {
class Cdr;
} // namespace fastcdr
} // namespace eprosima





/*!
 * @brief This class represents the structure Timestamp defined by the user in the IDL file.
 * @ingroup Complete
 */
class Timestamp
{
public:

    /*!
     * @brief Default constructor.
     */
    eProsima_user_DllExport Timestamp();

    /*!
     * @brief Default destructor.
     */
    eProsima_user_DllExport ~Timestamp();

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object Timestamp that will be copied.
     */
    eProsima_user_DllExport Timestamp(
            const Timestamp& x);

    /*!
     * @brief Move constructor.
     * @param x Reference to the object Timestamp that will be copied.
     */
    eProsima_user_DllExport Timestamp(
            Timestamp&& x) noexcept;

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object Timestamp that will be copied.
     */
    eProsima_user_DllExport Timestamp& operator =(
            const Timestamp& x);

    /*!
     * @brief Move assignment.
     * @param x Reference to the object Timestamp that will be copied.
     */
    eProsima_user_DllExport Timestamp& operator =(
            Timestamp&& x) noexcept;

    /*!
     * @brief Comparison operator.
     * @param x Timestamp object to compare.
     */
    eProsima_user_DllExport bool operator ==(
            const Timestamp& x) const;

    /*!
     * @brief Comparison operator.
     * @param x Timestamp object to compare.
     */
    eProsima_user_DllExport bool operator !=(
            const Timestamp& x) const;

    /*!
     * @brief This function sets a value in member seconds
     * @param _seconds New value for member seconds
     */
    eProsima_user_DllExport void seconds(
            int32_t _seconds);

    /*!
     * @brief This function returns the value of member seconds
     * @return Value of member seconds
     */
    eProsima_user_DllExport int32_t seconds() const;

    /*!
     * @brief This function returns a reference to member seconds
     * @return Reference to member seconds
     */
    eProsima_user_DllExport int32_t& seconds();


    /*!
     * @brief This function sets a value in member milliseconds
     * @param _milliseconds New value for member milliseconds
     */
    eProsima_user_DllExport void milliseconds(
            int32_t _milliseconds);

    /*!
     * @brief This function returns the value of member milliseconds
     * @return Value of member milliseconds
     */
    eProsima_user_DllExport int32_t milliseconds() const;

    /*!
     * @brief This function returns a reference to member milliseconds
     * @return Reference to member milliseconds
     */
    eProsima_user_DllExport int32_t& milliseconds();


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
            const Timestamp& data,
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

    int32_t m_seconds;
    int32_t m_milliseconds;

};



/*!
 * @brief This class represents the structure Point defined by the user in the IDL file.
 * @ingroup Complete
 */
class Point
{
public:

    /*!
     * @brief Default constructor.
     */
    eProsima_user_DllExport Point();

    /*!
     * @brief Default destructor.
     */
    eProsima_user_DllExport ~Point();

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object Point that will be copied.
     */
    eProsima_user_DllExport Point(
            const Point& x);

    /*!
     * @brief Move constructor.
     * @param x Reference to the object Point that will be copied.
     */
    eProsima_user_DllExport Point(
            Point&& x) noexcept;

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object Point that will be copied.
     */
    eProsima_user_DllExport Point& operator =(
            const Point& x);

    /*!
     * @brief Move assignment.
     * @param x Reference to the object Point that will be copied.
     */
    eProsima_user_DllExport Point& operator =(
            Point&& x) noexcept;

    /*!
     * @brief Comparison operator.
     * @param x Point object to compare.
     */
    eProsima_user_DllExport bool operator ==(
            const Point& x) const;

    /*!
     * @brief Comparison operator.
     * @param x Point object to compare.
     */
    eProsima_user_DllExport bool operator !=(
            const Point& x) const;

    /*!
     * @brief This function sets a value in member x
     * @param _x New value for member x
     */
    eProsima_user_DllExport void x(
            int32_t _x);

    /*!
     * @brief This function returns the value of member x
     * @return Value of member x
     */
    eProsima_user_DllExport int32_t x() const;

    /*!
     * @brief This function returns a reference to member x
     * @return Reference to member x
     */
    eProsima_user_DllExport int32_t& x();


    /*!
     * @brief This function sets a value in member y
     * @param _y New value for member y
     */
    eProsima_user_DllExport void y(
            int32_t _y);

    /*!
     * @brief This function returns the value of member y
     * @return Value of member y
     */
    eProsima_user_DllExport int32_t y() const;

    /*!
     * @brief This function returns a reference to member y
     * @return Reference to member y
     */
    eProsima_user_DllExport int32_t& y();


    /*!
     * @brief This function sets a value in member z
     * @param _z New value for member z
     */
    eProsima_user_DllExport void z(
            int32_t _z);

    /*!
     * @brief This function returns the value of member z
     * @return Value of member z
     */
    eProsima_user_DllExport int32_t z() const;

    /*!
     * @brief This function returns a reference to member z
     * @return Reference to member z
     */
    eProsima_user_DllExport int32_t& z();


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
            const Point& data,
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

    int32_t m_x;
    int32_t m_y;
    int32_t m_z;

};



/*!
 * @brief This class represents the structure MessageDescriptor defined by the user in the IDL file.
 * @ingroup Complete
 */
class MessageDescriptor
{
public:

    /*!
     * @brief Default constructor.
     */
    eProsima_user_DllExport MessageDescriptor();

    /*!
     * @brief Default destructor.
     */
    eProsima_user_DllExport ~MessageDescriptor();

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object MessageDescriptor that will be copied.
     */
    eProsima_user_DllExport MessageDescriptor(
            const MessageDescriptor& x);

    /*!
     * @brief Move constructor.
     * @param x Reference to the object MessageDescriptor that will be copied.
     */
    eProsima_user_DllExport MessageDescriptor(
            MessageDescriptor&& x) noexcept;

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object MessageDescriptor that will be copied.
     */
    eProsima_user_DllExport MessageDescriptor& operator =(
            const MessageDescriptor& x);

    /*!
     * @brief Move assignment.
     * @param x Reference to the object MessageDescriptor that will be copied.
     */
    eProsima_user_DllExport MessageDescriptor& operator =(
            MessageDescriptor&& x) noexcept;

    /*!
     * @brief Comparison operator.
     * @param x MessageDescriptor object to compare.
     */
    eProsima_user_DllExport bool operator ==(
            const MessageDescriptor& x) const;

    /*!
     * @brief Comparison operator.
     * @param x MessageDescriptor object to compare.
     */
    eProsima_user_DllExport bool operator !=(
            const MessageDescriptor& x) const;

    /*!
     * @brief This function sets a value in member id
     * @param _id New value for member id
     */
    eProsima_user_DllExport void id(
            uint32_t _id);

    /*!
     * @brief This function returns the value of member id
     * @return Value of member id
     */
    eProsima_user_DllExport uint32_t id() const;

    /*!
     * @brief This function returns a reference to member id
     * @return Reference to member id
     */
    eProsima_user_DllExport uint32_t& id();


    /*!
     * @brief This function copies the value in member topic
     * @param _topic New value to be copied in member topic
     */
    eProsima_user_DllExport void topic(
            const std::string& _topic);

    /*!
     * @brief This function moves the value in member topic
     * @param _topic New value to be moved in member topic
     */
    eProsima_user_DllExport void topic(
            std::string&& _topic);

    /*!
     * @brief This function returns a constant reference to member topic
     * @return Constant reference to member topic
     */
    eProsima_user_DllExport const std::string& topic() const;

    /*!
     * @brief This function returns a reference to member topic
     * @return Reference to member topic
     */
    eProsima_user_DllExport std::string& topic();


    /*!
     * @brief This function copies the value in member time
     * @param _time New value to be copied in member time
     */
    eProsima_user_DllExport void time(
            const Timestamp& _time);

    /*!
     * @brief This function moves the value in member time
     * @param _time New value to be moved in member time
     */
    eProsima_user_DllExport void time(
            Timestamp&& _time);

    /*!
     * @brief This function returns a constant reference to member time
     * @return Constant reference to member time
     */
    eProsima_user_DllExport const Timestamp& time() const;

    /*!
     * @brief This function returns a reference to member time
     * @return Reference to member time
     */
    eProsima_user_DllExport Timestamp& time();


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
            const MessageDescriptor& data,
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

    uint32_t m_id;
    std::string m_topic;
    Timestamp m_time;

};



/*!
 * @brief This class represents the structure Message defined by the user in the IDL file.
 * @ingroup Complete
 */
class Message
{
public:

    /*!
     * @brief Default constructor.
     */
    eProsima_user_DllExport Message();

    /*!
     * @brief Default destructor.
     */
    eProsima_user_DllExport ~Message();

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object Message that will be copied.
     */
    eProsima_user_DllExport Message(
            const Message& x);

    /*!
     * @brief Move constructor.
     * @param x Reference to the object Message that will be copied.
     */
    eProsima_user_DllExport Message(
            Message&& x) noexcept;

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object Message that will be copied.
     */
    eProsima_user_DllExport Message& operator =(
            const Message& x);

    /*!
     * @brief Move assignment.
     * @param x Reference to the object Message that will be copied.
     */
    eProsima_user_DllExport Message& operator =(
            Message&& x) noexcept;

    /*!
     * @brief Comparison operator.
     * @param x Message object to compare.
     */
    eProsima_user_DllExport bool operator ==(
            const Message& x) const;

    /*!
     * @brief Comparison operator.
     * @param x Message object to compare.
     */
    eProsima_user_DllExport bool operator !=(
            const Message& x) const;

    /*!
     * @brief This function copies the value in member descriptor
     * @param _descriptor New value to be copied in member descriptor
     */
    eProsima_user_DllExport void descriptor(
            const MessageDescriptor& _descriptor);

    /*!
     * @brief This function moves the value in member descriptor
     * @param _descriptor New value to be moved in member descriptor
     */
    eProsima_user_DllExport void descriptor(
            MessageDescriptor&& _descriptor);

    /*!
     * @brief This function returns a constant reference to member descriptor
     * @return Constant reference to member descriptor
     */
    eProsima_user_DllExport const MessageDescriptor& descriptor() const;

    /*!
     * @brief This function returns a reference to member descriptor
     * @return Reference to member descriptor
     */
    eProsima_user_DllExport MessageDescriptor& descriptor();


    /*!
     * @brief This function copies the value in member message
     * @param _message New value to be copied in member message
     */
    eProsima_user_DllExport void message(
            const std::string& _message);

    /*!
     * @brief This function moves the value in member message
     * @param _message New value to be moved in member message
     */
    eProsima_user_DllExport void message(
            std::string&& _message);

    /*!
     * @brief This function returns a constant reference to member message
     * @return Constant reference to member message
     */
    eProsima_user_DllExport const std::string& message() const;

    /*!
     * @brief This function returns a reference to member message
     * @return Reference to member message
     */
    eProsima_user_DllExport std::string& message();


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
            const Message& data,
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

    MessageDescriptor m_descriptor;
    std::string m_message;

};



/*!
 * @brief This class represents the structure CompleteData defined by the user in the IDL file.
 * @ingroup Complete
 */
class CompleteData
{
public:

    /*!
     * @brief Default constructor.
     */
    eProsima_user_DllExport CompleteData();

    /*!
     * @brief Default destructor.
     */
    eProsima_user_DllExport ~CompleteData();

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object CompleteData that will be copied.
     */
    eProsima_user_DllExport CompleteData(
            const CompleteData& x);

    /*!
     * @brief Move constructor.
     * @param x Reference to the object CompleteData that will be copied.
     */
    eProsima_user_DllExport CompleteData(
            CompleteData&& x) noexcept;

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object CompleteData that will be copied.
     */
    eProsima_user_DllExport CompleteData& operator =(
            const CompleteData& x);

    /*!
     * @brief Move assignment.
     * @param x Reference to the object CompleteData that will be copied.
     */
    eProsima_user_DllExport CompleteData& operator =(
            CompleteData&& x) noexcept;

    /*!
     * @brief Comparison operator.
     * @param x CompleteData object to compare.
     */
    eProsima_user_DllExport bool operator ==(
            const CompleteData& x) const;

    /*!
     * @brief Comparison operator.
     * @param x CompleteData object to compare.
     */
    eProsima_user_DllExport bool operator !=(
            const CompleteData& x) const;

    /*!
     * @brief This function sets a value in member index
     * @param _index New value for member index
     */
    eProsima_user_DllExport void index(
            uint32_t _index);

    /*!
     * @brief This function returns the value of member index
     * @return Value of member index
     */
    eProsima_user_DllExport uint32_t index() const;

    /*!
     * @brief This function returns a reference to member index
     * @return Reference to member index
     */
    eProsima_user_DllExport uint32_t& index();


    /*!
     * @brief This function copies the value in member main_point
     * @param _main_point New value to be copied in member main_point
     */
    eProsima_user_DllExport void main_point(
            const Point& _main_point);

    /*!
     * @brief This function moves the value in member main_point
     * @param _main_point New value to be moved in member main_point
     */
    eProsima_user_DllExport void main_point(
            Point&& _main_point);

    /*!
     * @brief This function returns a constant reference to member main_point
     * @return Constant reference to member main_point
     */
    eProsima_user_DllExport const Point& main_point() const;

    /*!
     * @brief This function returns a reference to member main_point
     * @return Reference to member main_point
     */
    eProsima_user_DllExport Point& main_point();


    /*!
     * @brief This function copies the value in member internal_data
     * @param _internal_data New value to be copied in member internal_data
     */
    eProsima_user_DllExport void internal_data(
            const std::vector<Point>& _internal_data);

    /*!
     * @brief This function moves the value in member internal_data
     * @param _internal_data New value to be moved in member internal_data
     */
    eProsima_user_DllExport void internal_data(
            std::vector<Point>&& _internal_data);

    /*!
     * @brief This function returns a constant reference to member internal_data
     * @return Constant reference to member internal_data
     */
    eProsima_user_DllExport const std::vector<Point>& internal_data() const;

    /*!
     * @brief This function returns a reference to member internal_data
     * @return Reference to member internal_data
     */
    eProsima_user_DllExport std::vector<Point>& internal_data();


    /*!
     * @brief This function copies the value in member messages
     * @param _messages New value to be copied in member messages
     */
    eProsima_user_DllExport void messages(
            const std::array<Message, 2>& _messages);

    /*!
     * @brief This function moves the value in member messages
     * @param _messages New value to be moved in member messages
     */
    eProsima_user_DllExport void messages(
            std::array<Message, 2>&& _messages);

    /*!
     * @brief This function returns a constant reference to member messages
     * @return Constant reference to member messages
     */
    eProsima_user_DllExport const std::array<Message, 2>& messages() const;

    /*!
     * @brief This function returns a reference to member messages
     * @return Reference to member messages
     */
    eProsima_user_DllExport std::array<Message, 2>& messages();


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
            const CompleteData& data,
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

    uint32_t m_index;
    Point m_main_point;
    std::vector<Point> m_internal_data;
    std::array<Message, 2> m_messages;

};


#endif // _FAST_DDS_GENERATED_COMPLETE_H_



#endif // FASTCDR_VERSION_MAJOR == 1
