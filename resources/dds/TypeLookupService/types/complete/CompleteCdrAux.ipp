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
 * @file CompleteCdrAux.ipp
 * This source file contains some declarations of CDR related functions.
 *
 * This file was generated by the tool fastddsgen.
 */

#ifndef _FAST_DDS_GENERATED_COMPLETECDRAUX_IPP_
#define _FAST_DDS_GENERATED_COMPLETECDRAUX_IPP_

#include "CompleteCdrAux.hpp"

#include <fastcdr/Cdr.h>
#if FASTCDR_VERSION_MAJOR > 1
#include <fastcdr/CdrSizeCalculator.hpp>
#endif // FASTCDR_VERSION_MAJOR > 1



#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

namespace eprosima {
namespace fastcdr {

template<>
eProsima_user_DllExport size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const Timestamp& data,
        size_t& current_alignment)
{
    static_cast<void>(data);

#if FASTCDR_VERSION_MAJOR == 1

    static_cast<void>(calculator);
    static_cast<void>(current_alignment);

    size_t initial_alignment {current_alignment};

            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);



    return current_alignment - initial_alignment;

#else

    eprosima::fastcdr::EncodingAlgorithmFlag previous_encoding = calculator.get_encoding();
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::CdrVersion::XCDRv2 == calculator.get_cdr_version() ?
                                eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2 :
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR,
                                current_alignment)};


        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(0),
                data.seconds(), current_alignment);

        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(1),
                data.milliseconds(), current_alignment);


    calculated_size += calculator.end_calculate_type_serialized_size(previous_encoding, current_alignment);

    return calculated_size;

#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
eProsima_user_DllExport void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const Timestamp& data)
{
#if FASTCDR_VERSION_MAJOR > 1
    eprosima::fastcdr::Cdr::state current_state(scdr);
    scdr.begin_serialize_type(current_state,
            eprosima::fastcdr::CdrVersion::XCDRv2 == scdr.get_cdr_version() ?
            eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2 :
            eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR);
#endif // FASTCDR_VERSION_MAJOR > 1

#if FASTCDR_VERSION_MAJOR == 1
            scdr << data.seconds()
            ;


            scdr << data.milliseconds()
            ;


#else
    scdr
        << eprosima::fastcdr::MemberId(0) << data.seconds()
        << eprosima::fastcdr::MemberId(1) << data.milliseconds()
;
#endif // FASTCDR_VERSION_MAJOR == 1

#if FASTCDR_VERSION_MAJOR > 1
    scdr.end_serialize_type(current_state);
#endif // FASTCDR_VERSION_MAJOR > 1
}

template<>
eProsima_user_DllExport void deserialize(
        eprosima::fastcdr::Cdr& cdr,
        Timestamp& data)
{
#if FASTCDR_VERSION_MAJOR == 1
                cdr >> data.seconds();
                cdr >> data.milliseconds();
;
#else
    cdr.deserialize_type(eprosima::fastcdr::CdrVersion::XCDRv2 == cdr.get_cdr_version() ?
            eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2 :
            eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR,
            [&data](eprosima::fastcdr::Cdr& dcdr, const eprosima::fastcdr::MemberId& mid) -> bool
            {
                bool ret_value = true;
                switch (mid.id)
                {
                                        case 0:
                                                dcdr >> data.seconds();
                                            break;

                                        case 1:
                                                dcdr >> data.milliseconds();
                                            break;

                    default:
                        ret_value = false;
                        break;
                }
                return ret_value;
            });
#endif // FASTCDR_VERSION_MAJOR == 1
}

void serialize_key(
        eprosima::fastcdr::Cdr& scdr,
        const Timestamp& data)
{
    static_cast<void>(scdr);
    static_cast<void>(data);
}


template<>
eProsima_user_DllExport size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const Point& data,
        size_t& current_alignment)
{
    static_cast<void>(data);

#if FASTCDR_VERSION_MAJOR == 1

    static_cast<void>(calculator);
    static_cast<void>(current_alignment);

    size_t initial_alignment {current_alignment};

            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);



    return current_alignment - initial_alignment;

#else

    eprosima::fastcdr::EncodingAlgorithmFlag previous_encoding = calculator.get_encoding();
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::CdrVersion::XCDRv2 == calculator.get_cdr_version() ?
                                eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2 :
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR,
                                current_alignment)};


        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(0),
                data.x(), current_alignment);

        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(1),
                data.y(), current_alignment);

        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(2),
                data.z(), current_alignment);


    calculated_size += calculator.end_calculate_type_serialized_size(previous_encoding, current_alignment);

    return calculated_size;

#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
eProsima_user_DllExport void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const Point& data)
{
#if FASTCDR_VERSION_MAJOR > 1
    eprosima::fastcdr::Cdr::state current_state(scdr);
    scdr.begin_serialize_type(current_state,
            eprosima::fastcdr::CdrVersion::XCDRv2 == scdr.get_cdr_version() ?
            eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2 :
            eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR);
#endif // FASTCDR_VERSION_MAJOR > 1

#if FASTCDR_VERSION_MAJOR == 1
            scdr << data.x()
            ;


            scdr << data.y()
            ;


            scdr << data.z()
            ;


#else
    scdr
        << eprosima::fastcdr::MemberId(0) << data.x()
        << eprosima::fastcdr::MemberId(1) << data.y()
        << eprosima::fastcdr::MemberId(2) << data.z()
;
#endif // FASTCDR_VERSION_MAJOR == 1

#if FASTCDR_VERSION_MAJOR > 1
    scdr.end_serialize_type(current_state);
#endif // FASTCDR_VERSION_MAJOR > 1
}

template<>
eProsima_user_DllExport void deserialize(
        eprosima::fastcdr::Cdr& cdr,
        Point& data)
{
#if FASTCDR_VERSION_MAJOR == 1
                cdr >> data.x();
                cdr >> data.y();
                cdr >> data.z();
;
#else
    cdr.deserialize_type(eprosima::fastcdr::CdrVersion::XCDRv2 == cdr.get_cdr_version() ?
            eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2 :
            eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR,
            [&data](eprosima::fastcdr::Cdr& dcdr, const eprosima::fastcdr::MemberId& mid) -> bool
            {
                bool ret_value = true;
                switch (mid.id)
                {
                                        case 0:
                                                dcdr >> data.x();
                                            break;

                                        case 1:
                                                dcdr >> data.y();
                                            break;

                                        case 2:
                                                dcdr >> data.z();
                                            break;

                    default:
                        ret_value = false;
                        break;
                }
                return ret_value;
            });
#endif // FASTCDR_VERSION_MAJOR == 1
}

void serialize_key(
        eprosima::fastcdr::Cdr& scdr,
        const Point& data)
{
    static_cast<void>(scdr);
    static_cast<void>(data);
}


template<>
eProsima_user_DllExport size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const MessageDescriptor& data,
        size_t& current_alignment)
{
    static_cast<void>(data);

#if FASTCDR_VERSION_MAJOR == 1

    static_cast<void>(calculator);
    static_cast<void>(current_alignment);

    size_t initial_alignment {current_alignment};

            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.topic().size() + 1;


            current_alignment += calculate_serialized_size(calculator, data.time(), current_alignment);



    return current_alignment - initial_alignment;

#else

    eprosima::fastcdr::EncodingAlgorithmFlag previous_encoding = calculator.get_encoding();
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::CdrVersion::XCDRv2 == calculator.get_cdr_version() ?
                                eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2 :
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR,
                                current_alignment)};


        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(0),
                data.id(), current_alignment);

        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(1),
                data.topic(), current_alignment);

        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(2),
                data.time(), current_alignment);


    calculated_size += calculator.end_calculate_type_serialized_size(previous_encoding, current_alignment);

    return calculated_size;

#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
eProsima_user_DllExport void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const MessageDescriptor& data)
{
#if FASTCDR_VERSION_MAJOR > 1
    eprosima::fastcdr::Cdr::state current_state(scdr);
    scdr.begin_serialize_type(current_state,
            eprosima::fastcdr::CdrVersion::XCDRv2 == scdr.get_cdr_version() ?
            eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2 :
            eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR);
#endif // FASTCDR_VERSION_MAJOR > 1

#if FASTCDR_VERSION_MAJOR == 1
            scdr << data.id()
            ;


            scdr << data.topic()
            ;


            scdr << data.time()
            ;


#else
    scdr
        << eprosima::fastcdr::MemberId(0) << data.id()
        << eprosima::fastcdr::MemberId(1) << data.topic()
        << eprosima::fastcdr::MemberId(2) << data.time()
;
#endif // FASTCDR_VERSION_MAJOR == 1

#if FASTCDR_VERSION_MAJOR > 1
    scdr.end_serialize_type(current_state);
#endif // FASTCDR_VERSION_MAJOR > 1
}

template<>
eProsima_user_DllExport void deserialize(
        eprosima::fastcdr::Cdr& cdr,
        MessageDescriptor& data)
{
#if FASTCDR_VERSION_MAJOR == 1
                cdr >> data.id();
                cdr >> data.topic();
                cdr >> data.time();
;
#else
    cdr.deserialize_type(eprosima::fastcdr::CdrVersion::XCDRv2 == cdr.get_cdr_version() ?
            eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2 :
            eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR,
            [&data](eprosima::fastcdr::Cdr& dcdr, const eprosima::fastcdr::MemberId& mid) -> bool
            {
                bool ret_value = true;
                switch (mid.id)
                {
                                        case 0:
                                                dcdr >> data.id();
                                            break;

                                        case 1:
                                                dcdr >> data.topic();
                                            break;

                                        case 2:
                                                dcdr >> data.time();
                                            break;

                    default:
                        ret_value = false;
                        break;
                }
                return ret_value;
            });
#endif // FASTCDR_VERSION_MAJOR == 1
}

void serialize_key(
        eprosima::fastcdr::Cdr& scdr,
        const MessageDescriptor& data)
{
    static_cast<void>(scdr);
    static_cast<void>(data);
}


template<>
eProsima_user_DllExport size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const Message& data,
        size_t& current_alignment)
{
    static_cast<void>(data);

#if FASTCDR_VERSION_MAJOR == 1

    static_cast<void>(calculator);
    static_cast<void>(current_alignment);

    size_t initial_alignment {current_alignment};

            current_alignment += calculate_serialized_size(calculator, data.descriptor(), current_alignment);


            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.message().size() + 1;



    return current_alignment - initial_alignment;

#else

    eprosima::fastcdr::EncodingAlgorithmFlag previous_encoding = calculator.get_encoding();
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::CdrVersion::XCDRv2 == calculator.get_cdr_version() ?
                                eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2 :
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR,
                                current_alignment)};


        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(0),
                data.descriptor(), current_alignment);

        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(1),
                data.message(), current_alignment);


    calculated_size += calculator.end_calculate_type_serialized_size(previous_encoding, current_alignment);

    return calculated_size;

#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
eProsima_user_DllExport void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const Message& data)
{
#if FASTCDR_VERSION_MAJOR > 1
    eprosima::fastcdr::Cdr::state current_state(scdr);
    scdr.begin_serialize_type(current_state,
            eprosima::fastcdr::CdrVersion::XCDRv2 == scdr.get_cdr_version() ?
            eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2 :
            eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR);
#endif // FASTCDR_VERSION_MAJOR > 1

#if FASTCDR_VERSION_MAJOR == 1
            scdr << data.descriptor()
            ;


            scdr << data.message()
            ;


#else
    scdr
        << eprosima::fastcdr::MemberId(0) << data.descriptor()
        << eprosima::fastcdr::MemberId(1) << data.message()
;
#endif // FASTCDR_VERSION_MAJOR == 1

#if FASTCDR_VERSION_MAJOR > 1
    scdr.end_serialize_type(current_state);
#endif // FASTCDR_VERSION_MAJOR > 1
}

template<>
eProsima_user_DllExport void deserialize(
        eprosima::fastcdr::Cdr& cdr,
        Message& data)
{
#if FASTCDR_VERSION_MAJOR == 1
                cdr >> data.descriptor();
                cdr >> data.message();
;
#else
    cdr.deserialize_type(eprosima::fastcdr::CdrVersion::XCDRv2 == cdr.get_cdr_version() ?
            eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2 :
            eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR,
            [&data](eprosima::fastcdr::Cdr& dcdr, const eprosima::fastcdr::MemberId& mid) -> bool
            {
                bool ret_value = true;
                switch (mid.id)
                {
                                        case 0:
                                                dcdr >> data.descriptor();
                                            break;

                                        case 1:
                                                dcdr >> data.message();
                                            break;

                    default:
                        ret_value = false;
                        break;
                }
                return ret_value;
            });
#endif // FASTCDR_VERSION_MAJOR == 1
}

void serialize_key(
        eprosima::fastcdr::Cdr& scdr,
        const Message& data)
{
    static_cast<void>(scdr);
    static_cast<void>(data);
}


template<>
eProsima_user_DllExport size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const CompleteData& data,
        size_t& current_alignment)
{
    static_cast<void>(data);

#if FASTCDR_VERSION_MAJOR == 1

    static_cast<void>(calculator);
    static_cast<void>(current_alignment);

    size_t initial_alignment {current_alignment};

            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


            current_alignment += calculate_serialized_size(calculator, data.main_point(), current_alignment);


            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


            for(size_t a = 0; a < data.internal_data().size(); ++a)
            {
                current_alignment += calculate_serialized_size(calculator, data.internal_data().at(a), current_alignment);
            }




            for(size_t a = 0; a < data.messages().size(); ++a)
            {
                    current_alignment += calculate_serialized_size(calculator, data.messages().at(a), current_alignment);

            }




    return current_alignment - initial_alignment;

#else

    eprosima::fastcdr::EncodingAlgorithmFlag previous_encoding = calculator.get_encoding();
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::CdrVersion::XCDRv2 == calculator.get_cdr_version() ?
                                eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2 :
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR,
                                current_alignment)};


        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(0),
                data.index(), current_alignment);

        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(1),
                data.main_point(), current_alignment);

        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(2),
                data.internal_data(), current_alignment);

        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(3),
                data.messages(), current_alignment);


    calculated_size += calculator.end_calculate_type_serialized_size(previous_encoding, current_alignment);

    return calculated_size;

#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
eProsima_user_DllExport void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const CompleteData& data)
{
#if FASTCDR_VERSION_MAJOR > 1
    eprosima::fastcdr::Cdr::state current_state(scdr);
    scdr.begin_serialize_type(current_state,
            eprosima::fastcdr::CdrVersion::XCDRv2 == scdr.get_cdr_version() ?
            eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2 :
            eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR);
#endif // FASTCDR_VERSION_MAJOR > 1

#if FASTCDR_VERSION_MAJOR == 1
            scdr << data.index()
            ;


            scdr << data.main_point()
            ;


            scdr << data.internal_data()
            ;


            scdr << data.messages()
            ;


#else
    scdr
        << eprosima::fastcdr::MemberId(0) << data.index()
        << eprosima::fastcdr::MemberId(1) << data.main_point()
        << eprosima::fastcdr::MemberId(2) << data.internal_data()
        << eprosima::fastcdr::MemberId(3) << data.messages()
;
#endif // FASTCDR_VERSION_MAJOR == 1

#if FASTCDR_VERSION_MAJOR > 1
    scdr.end_serialize_type(current_state);
#endif // FASTCDR_VERSION_MAJOR > 1
}

template<>
eProsima_user_DllExport void deserialize(
        eprosima::fastcdr::Cdr& cdr,
        CompleteData& data)
{
#if FASTCDR_VERSION_MAJOR == 1
                cdr >> data.index();
                cdr >> data.main_point();
                cdr >> data.internal_data();
                cdr >> data.messages();
;
#else
    cdr.deserialize_type(eprosima::fastcdr::CdrVersion::XCDRv2 == cdr.get_cdr_version() ?
            eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2 :
            eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR,
            [&data](eprosima::fastcdr::Cdr& dcdr, const eprosima::fastcdr::MemberId& mid) -> bool
            {
                bool ret_value = true;
                switch (mid.id)
                {
                                        case 0:
                                                dcdr >> data.index();
                                            break;

                                        case 1:
                                                dcdr >> data.main_point();
                                            break;

                                        case 2:
                                                dcdr >> data.internal_data();
                                            break;

                                        case 3:
                                                dcdr >> data.messages();
                                            break;

                    default:
                        ret_value = false;
                        break;
                }
                return ret_value;
            });
#endif // FASTCDR_VERSION_MAJOR == 1
}

void serialize_key(
        eprosima::fastcdr::Cdr& scdr,
        const CompleteData& data)
{
    static_cast<void>(scdr);
    static_cast<void>(data);
}



} // namespace fastcdr
} // namespace eprosima

#endif // _FAST_DDS_GENERATED_COMPLETECDRAUX_IPP_

