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
 * @file Complete.cpp
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

#include "Complete.h"

#if FASTCDR_VERSION_MAJOR == 1

#include "CompleteTypeObject.h"

#include <fastcdr/Cdr.h>


#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

#include <utility>

namespace helper {
namespace internal {

enum class Size
{
    UInt8,
    UInt16,
    UInt32,
    UInt64,
};

constexpr Size get_size(
        int s)
{
    return (s <= 8 ) ? Size::UInt8:
           (s <= 16) ? Size::UInt16:
           (s <= 32) ? Size::UInt32: Size::UInt64;
}

template<Size s>
struct FindTypeH;

template<>
struct FindTypeH<Size::UInt8>
{
    using type = std::uint8_t;
};

template<>
struct FindTypeH<Size::UInt16>
{
    using type = std::uint16_t;
};

template<>
struct FindTypeH<Size::UInt32>
{
    using type = std::uint32_t;
};

template<>
struct FindTypeH<Size::UInt64>
{
    using type = std::uint64_t;
};
} // namespace internal

template<int S>
struct FindType
{
    using type = typename internal::FindTypeH<internal::get_size(S)>::type;
};
} // namespace helper

#define MessageDescriptor_max_cdr_typesize 280ULL;
#define CompleteData_max_cdr_typesize 2724ULL;
#define Message_max_cdr_typesize 544ULL;
#define Point_max_cdr_typesize 16ULL;
#define Timestamp_max_cdr_typesize 12ULL;




Timestamp::Timestamp()
{
    // long m_seconds
    m_seconds = 0;
    // long m_milliseconds
    m_milliseconds = 0;

    // Just to register all known types
    registerCompleteTypes();
}

Timestamp::~Timestamp()
{
}

Timestamp::Timestamp(
        const Timestamp& x)
{
    m_seconds = x.m_seconds;


    m_milliseconds = x.m_milliseconds;

}

Timestamp::Timestamp(
        Timestamp&& x) noexcept
{
    m_seconds = x.m_seconds;


    m_milliseconds = x.m_milliseconds;

}

Timestamp& Timestamp::operator =(
        const Timestamp& x)
{
    m_seconds = x.m_seconds;


    m_milliseconds = x.m_milliseconds;

    return *this;
}

Timestamp& Timestamp::operator =(
        Timestamp&& x) noexcept
{
    m_seconds = x.m_seconds;


    m_milliseconds = x.m_milliseconds;

    return *this;
}

bool Timestamp::operator ==(
        const Timestamp& x) const
{
    return (m_seconds == x.m_seconds &&
           m_milliseconds == x.m_milliseconds);
}

bool Timestamp::operator !=(
        const Timestamp& x) const
{
    return !(*this == x);
}

size_t Timestamp::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);
    return Timestamp_max_cdr_typesize;
}

size_t Timestamp::getCdrSerializedSize(
        const Timestamp& data,
        size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    return current_alignment - initial_alignment;
}

void Timestamp::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{
    scdr << m_seconds;

    scdr << m_milliseconds;

}

void Timestamp::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{
    dcdr >> m_seconds;



    dcdr >> m_milliseconds;


}

bool Timestamp::isKeyDefined()
{
    return false;
}

void Timestamp::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;
}

/*!
 * @brief This function sets a value in member seconds
 * @param _seconds New value for member seconds
 */
void Timestamp::seconds(
        int32_t _seconds)
{
    m_seconds = _seconds;
}

/*!
 * @brief This function returns the value of member seconds
 * @return Value of member seconds
 */
int32_t Timestamp::seconds() const
{
    return m_seconds;
}

/*!
 * @brief This function returns a reference to member seconds
 * @return Reference to member seconds
 */
int32_t& Timestamp::seconds()
{
    return m_seconds;
}

/*!
 * @brief This function sets a value in member milliseconds
 * @param _milliseconds New value for member milliseconds
 */
void Timestamp::milliseconds(
        int32_t _milliseconds)
{
    m_milliseconds = _milliseconds;
}

/*!
 * @brief This function returns the value of member milliseconds
 * @return Value of member milliseconds
 */
int32_t Timestamp::milliseconds() const
{
    return m_milliseconds;
}

/*!
 * @brief This function returns a reference to member milliseconds
 * @return Reference to member milliseconds
 */
int32_t& Timestamp::milliseconds()
{
    return m_milliseconds;
}

Point::Point()
{
    // long m_x
    m_x = 0;
    // long m_y
    m_y = 0;
    // long m_z
    m_z = 0;

    // Just to register all known types
    registerCompleteTypes();
}

Point::~Point()
{
}

Point::Point(
        const Point& x)
{
    m_x = x.m_x;


    m_y = x.m_y;


    m_z = x.m_z;

}

Point::Point(
        Point&& x) noexcept
{
    m_x = x.m_x;


    m_y = x.m_y;


    m_z = x.m_z;

}

Point& Point::operator =(
        const Point& x)
{
    m_x = x.m_x;


    m_y = x.m_y;


    m_z = x.m_z;

    return *this;
}

Point& Point::operator =(
        Point&& x) noexcept
{
    m_x = x.m_x;


    m_y = x.m_y;


    m_z = x.m_z;

    return *this;
}

bool Point::operator ==(
        const Point& x) const
{
    return (m_x == x.m_x &&
           m_y == x.m_y &&
           m_z == x.m_z);
}

bool Point::operator !=(
        const Point& x) const
{
    return !(*this == x);
}

size_t Point::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);
    return Point_max_cdr_typesize;
}

size_t Point::getCdrSerializedSize(
        const Point& data,
        size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    return current_alignment - initial_alignment;
}

void Point::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{
    scdr << m_x;

    scdr << m_y;

    scdr << m_z;

}

void Point::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{
    dcdr >> m_x;



    dcdr >> m_y;



    dcdr >> m_z;


}

bool Point::isKeyDefined()
{
    return false;
}

void Point::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;
}

/*!
 * @brief This function sets a value in member x
 * @param _x New value for member x
 */
void Point::x(
        int32_t _x)
{
    m_x = _x;
}

/*!
 * @brief This function returns the value of member x
 * @return Value of member x
 */
int32_t Point::x() const
{
    return m_x;
}

/*!
 * @brief This function returns a reference to member x
 * @return Reference to member x
 */
int32_t& Point::x()
{
    return m_x;
}

/*!
 * @brief This function sets a value in member y
 * @param _y New value for member y
 */
void Point::y(
        int32_t _y)
{
    m_y = _y;
}

/*!
 * @brief This function returns the value of member y
 * @return Value of member y
 */
int32_t Point::y() const
{
    return m_y;
}

/*!
 * @brief This function returns a reference to member y
 * @return Reference to member y
 */
int32_t& Point::y()
{
    return m_y;
}

/*!
 * @brief This function sets a value in member z
 * @param _z New value for member z
 */
void Point::z(
        int32_t _z)
{
    m_z = _z;
}

/*!
 * @brief This function returns the value of member z
 * @return Value of member z
 */
int32_t Point::z() const
{
    return m_z;
}

/*!
 * @brief This function returns a reference to member z
 * @return Reference to member z
 */
int32_t& Point::z()
{
    return m_z;
}

MessageDescriptor::MessageDescriptor()
{
    // unsigned long m_id
    m_id = 0;
    // /type_d() m_topic

    // Timestamp m_time


    // Just to register all known types
    registerCompleteTypes();
}

MessageDescriptor::~MessageDescriptor()
{
}

MessageDescriptor::MessageDescriptor(
        const MessageDescriptor& x)
{
    m_id = x.m_id;


    m_topic = x.m_topic;


    m_time = x.m_time;

}

MessageDescriptor::MessageDescriptor(
        MessageDescriptor&& x) noexcept
{
    m_id = x.m_id;


    m_topic = std::move(x.m_topic);


    m_time = std::move(x.m_time);

}

MessageDescriptor& MessageDescriptor::operator =(
        const MessageDescriptor& x)
{
    m_id = x.m_id;


    m_topic = x.m_topic;


    m_time = x.m_time;

    return *this;
}

MessageDescriptor& MessageDescriptor::operator =(
        MessageDescriptor&& x) noexcept
{
    m_id = x.m_id;


    m_topic = std::move(x.m_topic);


    m_time = std::move(x.m_time);

    return *this;
}

bool MessageDescriptor::operator ==(
        const MessageDescriptor& x) const
{
    return (m_id == x.m_id &&
           m_topic == x.m_topic &&
           m_time == x.m_time);
}

bool MessageDescriptor::operator !=(
        const MessageDescriptor& x) const
{
    return !(*this == x);
}

size_t MessageDescriptor::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);
    return MessageDescriptor_max_cdr_typesize;
}

size_t MessageDescriptor::getCdrSerializedSize(
        const MessageDescriptor& data,
        size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.topic().size() + 1;


    current_alignment += Timestamp::getCdrSerializedSize(data.time(), current_alignment);


    return current_alignment - initial_alignment;
}

void MessageDescriptor::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{
    scdr << m_id;

    scdr << m_topic.c_str();

    scdr << m_time;

}

void MessageDescriptor::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{
    dcdr >> m_id;



    dcdr >> m_topic;



    dcdr >> m_time;


}

bool MessageDescriptor::isKeyDefined()
{
    return false;
}

void MessageDescriptor::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;
}

/*!
 * @brief This function sets a value in member id
 * @param _id New value for member id
 */
void MessageDescriptor::id(
        uint32_t _id)
{
    m_id = _id;
}

/*!
 * @brief This function returns the value of member id
 * @return Value of member id
 */
uint32_t MessageDescriptor::id() const
{
    return m_id;
}

/*!
 * @brief This function returns a reference to member id
 * @return Reference to member id
 */
uint32_t& MessageDescriptor::id()
{
    return m_id;
}

/*!
 * @brief This function copies the value in member topic
 * @param _topic New value to be copied in member topic
 */
void MessageDescriptor::topic(
        const std::string& _topic)
{
    m_topic = _topic;
}

/*!
 * @brief This function moves the value in member topic
 * @param _topic New value to be moved in member topic
 */
void MessageDescriptor::topic(
        std::string&& _topic)
{
    m_topic = std::move(_topic);
}

/*!
 * @brief This function returns a constant reference to member topic
 * @return Constant reference to member topic
 */
const std::string& MessageDescriptor::topic() const
{
    return m_topic;
}

/*!
 * @brief This function returns a reference to member topic
 * @return Reference to member topic
 */
std::string& MessageDescriptor::topic()
{
    return m_topic;
}

/*!
 * @brief This function copies the value in member time
 * @param _time New value to be copied in member time
 */
void MessageDescriptor::time(
        const Timestamp& _time)
{
    m_time = _time;
}

/*!
 * @brief This function moves the value in member time
 * @param _time New value to be moved in member time
 */
void MessageDescriptor::time(
        Timestamp&& _time)
{
    m_time = std::move(_time);
}

/*!
 * @brief This function returns a constant reference to member time
 * @return Constant reference to member time
 */
const Timestamp& MessageDescriptor::time() const
{
    return m_time;
}

/*!
 * @brief This function returns a reference to member time
 * @return Reference to member time
 */
Timestamp& MessageDescriptor::time()
{
    return m_time;
}

Message::Message()
{
    // MessageDescriptor m_descriptor

    // /type_d() m_message


    // Just to register all known types
    registerCompleteTypes();
}

Message::~Message()
{
}

Message::Message(
        const Message& x)
{
    m_descriptor = x.m_descriptor;


    m_message = x.m_message;

}

Message::Message(
        Message&& x) noexcept
{
    m_descriptor = std::move(x.m_descriptor);


    m_message = std::move(x.m_message);

}

Message& Message::operator =(
        const Message& x)
{
    m_descriptor = x.m_descriptor;


    m_message = x.m_message;

    return *this;
}

Message& Message::operator =(
        Message&& x) noexcept
{
    m_descriptor = std::move(x.m_descriptor);


    m_message = std::move(x.m_message);

    return *this;
}

bool Message::operator ==(
        const Message& x) const
{
    return (m_descriptor == x.m_descriptor &&
           m_message == x.m_message);
}

bool Message::operator !=(
        const Message& x) const
{
    return !(*this == x);
}

size_t Message::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);
    return Message_max_cdr_typesize;
}

size_t Message::getCdrSerializedSize(
        const Message& data,
        size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;

    current_alignment += MessageDescriptor::getCdrSerializedSize(data.descriptor(), current_alignment);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.message().size() + 1;


    return current_alignment - initial_alignment;
}

void Message::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{
    scdr << m_descriptor;

    scdr << m_message.c_str();

}

void Message::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{
    dcdr >> m_descriptor;



    dcdr >> m_message;


}

bool Message::isKeyDefined()
{
    return false;
}

void Message::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;
}

/*!
 * @brief This function copies the value in member descriptor
 * @param _descriptor New value to be copied in member descriptor
 */
void Message::descriptor(
        const MessageDescriptor& _descriptor)
{
    m_descriptor = _descriptor;
}

/*!
 * @brief This function moves the value in member descriptor
 * @param _descriptor New value to be moved in member descriptor
 */
void Message::descriptor(
        MessageDescriptor&& _descriptor)
{
    m_descriptor = std::move(_descriptor);
}

/*!
 * @brief This function returns a constant reference to member descriptor
 * @return Constant reference to member descriptor
 */
const MessageDescriptor& Message::descriptor() const
{
    return m_descriptor;
}

/*!
 * @brief This function returns a reference to member descriptor
 * @return Reference to member descriptor
 */
MessageDescriptor& Message::descriptor()
{
    return m_descriptor;
}

/*!
 * @brief This function copies the value in member message
 * @param _message New value to be copied in member message
 */
void Message::message(
        const std::string& _message)
{
    m_message = _message;
}

/*!
 * @brief This function moves the value in member message
 * @param _message New value to be moved in member message
 */
void Message::message(
        std::string&& _message)
{
    m_message = std::move(_message);
}

/*!
 * @brief This function returns a constant reference to member message
 * @return Constant reference to member message
 */
const std::string& Message::message() const
{
    return m_message;
}

/*!
 * @brief This function returns a reference to member message
 * @return Reference to member message
 */
std::string& Message::message()
{
    return m_message;
}

CompleteData::CompleteData()
{
    // unsigned long m_index
    m_index = 0;
    // Point m_main_point

    // sequence<Point> m_internal_data

    // Message m_messages


    // Just to register all known types
    registerCompleteTypes();
}

CompleteData::~CompleteData()
{
}

CompleteData::CompleteData(
        const CompleteData& x)
{
    m_index = x.m_index;


    m_main_point = x.m_main_point;


    m_internal_data = x.m_internal_data;


    m_messages = x.m_messages;

}

CompleteData::CompleteData(
        CompleteData&& x) noexcept
{
    m_index = x.m_index;


    m_main_point = std::move(x.m_main_point);


    m_internal_data = std::move(x.m_internal_data);


    m_messages = std::move(x.m_messages);

}

CompleteData& CompleteData::operator =(
        const CompleteData& x)
{
    m_index = x.m_index;


    m_main_point = x.m_main_point;


    m_internal_data = x.m_internal_data;


    m_messages = x.m_messages;

    return *this;
}

CompleteData& CompleteData::operator =(
        CompleteData&& x) noexcept
{
    m_index = x.m_index;


    m_main_point = std::move(x.m_main_point);


    m_internal_data = std::move(x.m_internal_data);


    m_messages = std::move(x.m_messages);

    return *this;
}

bool CompleteData::operator ==(
        const CompleteData& x) const
{
    return (m_index == x.m_index &&
           m_main_point == x.m_main_point &&
           m_internal_data == x.m_internal_data &&
           m_messages == x.m_messages);
}

bool CompleteData::operator !=(
        const CompleteData& x) const
{
    return !(*this == x);
}

size_t CompleteData::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);
    return CompleteData_max_cdr_typesize;
}

size_t CompleteData::getCdrSerializedSize(
        const CompleteData& data,
        size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += Point::getCdrSerializedSize(data.main_point(), current_alignment);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    for (size_t a = 0; a < data.internal_data().size(); ++a)
    {
        current_alignment += Point::getCdrSerializedSize(data.internal_data().at(a), current_alignment);
    }




    for (size_t a = 0; a < data.messages().size(); ++a)
    {
        current_alignment += Message::getCdrSerializedSize(data.messages().at(a), current_alignment);

    }



    return current_alignment - initial_alignment;
}

void CompleteData::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{
    scdr << m_index;

    scdr << m_main_point;

    scdr << m_internal_data;


    scdr << m_messages;


}

void CompleteData::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{
    dcdr >> m_index;



    dcdr >> m_main_point;



    dcdr >> m_internal_data;



    dcdr >> m_messages;


}

bool CompleteData::isKeyDefined()
{
    return false;
}

void CompleteData::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;
}

/*!
 * @brief This function sets a value in member index
 * @param _index New value for member index
 */
void CompleteData::index(
        uint32_t _index)
{
    m_index = _index;
}

/*!
 * @brief This function returns the value of member index
 * @return Value of member index
 */
uint32_t CompleteData::index() const
{
    return m_index;
}

/*!
 * @brief This function returns a reference to member index
 * @return Reference to member index
 */
uint32_t& CompleteData::index()
{
    return m_index;
}

/*!
 * @brief This function copies the value in member main_point
 * @param _main_point New value to be copied in member main_point
 */
void CompleteData::main_point(
        const Point& _main_point)
{
    m_main_point = _main_point;
}

/*!
 * @brief This function moves the value in member main_point
 * @param _main_point New value to be moved in member main_point
 */
void CompleteData::main_point(
        Point&& _main_point)
{
    m_main_point = std::move(_main_point);
}

/*!
 * @brief This function returns a constant reference to member main_point
 * @return Constant reference to member main_point
 */
const Point& CompleteData::main_point() const
{
    return m_main_point;
}

/*!
 * @brief This function returns a reference to member main_point
 * @return Reference to member main_point
 */
Point& CompleteData::main_point()
{
    return m_main_point;
}

/*!
 * @brief This function copies the value in member internal_data
 * @param _internal_data New value to be copied in member internal_data
 */
void CompleteData::internal_data(
        const std::vector<Point>& _internal_data)
{
    m_internal_data = _internal_data;
}

/*!
 * @brief This function moves the value in member internal_data
 * @param _internal_data New value to be moved in member internal_data
 */
void CompleteData::internal_data(
        std::vector<Point>&& _internal_data)
{
    m_internal_data = std::move(_internal_data);
}

/*!
 * @brief This function returns a constant reference to member internal_data
 * @return Constant reference to member internal_data
 */
const std::vector<Point>& CompleteData::internal_data() const
{
    return m_internal_data;
}

/*!
 * @brief This function returns a reference to member internal_data
 * @return Reference to member internal_data
 */
std::vector<Point>& CompleteData::internal_data()
{
    return m_internal_data;
}

/*!
 * @brief This function copies the value in member messages
 * @param _messages New value to be copied in member messages
 */
void CompleteData::messages(
        const std::array<Message, 2>& _messages)
{
    m_messages = _messages;
}

/*!
 * @brief This function moves the value in member messages
 * @param _messages New value to be moved in member messages
 */
void CompleteData::messages(
        std::array<Message, 2>&& _messages)
{
    m_messages = std::move(_messages);
}

/*!
 * @brief This function returns a constant reference to member messages
 * @return Constant reference to member messages
 */
const std::array<Message, 2>& CompleteData::messages() const
{
    return m_messages;
}

/*!
 * @brief This function returns a reference to member messages
 * @return Reference to member messages
 */
std::array<Message, 2>& CompleteData::messages()
{
    return m_messages;
}

#endif // FASTCDR_VERSION_MAJOR == 1
