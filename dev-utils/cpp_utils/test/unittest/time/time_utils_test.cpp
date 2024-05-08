// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <thread>
#include <iomanip>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/macros/macros.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/utils.hpp>

using namespace eprosima::utils;

/**
 * Test function timestamp_to_string with a time stamp, as well the inverse conversion string_to_timestamp.
 *
 * CASES:
 * - now
 * - now with alternative format
 * - old time
 * - future time
 * - some time today
 */
TEST(time_utils_test, timestamp_to_string_to_timestamp)
{
    // now
    {
        Timestamp now_time = now();
        std::string now_time_str = timestamp_to_string(now_time);

        time_t time = std::chrono::system_clock::to_time_t(now_time);
        std::tm local_tm = *gmtime(&time);

        std::ostringstream expected_string_os;
        expected_string_os
            << number_trailing_zeros_format(local_tm.tm_year + 1900, 4)
            << "-" << number_trailing_zeros_format(local_tm.tm_mon + 1, 2)
            << "-" << number_trailing_zeros_format(local_tm.tm_mday, 2)
            << "_" << number_trailing_zeros_format(local_tm.tm_hour, 2)
            << "-" << number_trailing_zeros_format(local_tm.tm_min, 2)
            << "-" << number_trailing_zeros_format(local_tm.tm_sec, 2);

        // Test timestamp_to_string
        ASSERT_EQ(now_time_str, expected_string_os.str());

        // Test string_to_timestamp
        Timestamp now_time_from_str = string_to_timestamp(now_time_str);
        // NOTE: cannot directly compare timestamps because some precision is lost during ts->str conversion
        ASSERT_EQ(timestamp_to_string(now_time_from_str), expected_string_os.str());
    }

    // now with alternative format
    {
        Timestamp now_time = now();
        std::string format = "%S-%M-%H___%d-%m-%Y";
        std::string now_time_str = timestamp_to_string(now_time, format);

        time_t time = std::chrono::system_clock::to_time_t(now_time);
        std::tm local_tm = *gmtime(&time);

        std::ostringstream expected_string_os;
        expected_string_os
            << number_trailing_zeros_format(local_tm.tm_sec, 2)
            << "-" << number_trailing_zeros_format(local_tm.tm_min, 2)
            << "-" << number_trailing_zeros_format(local_tm.tm_hour, 2)
            << "___" << number_trailing_zeros_format(local_tm.tm_mday, 2)
            << "-" << number_trailing_zeros_format(local_tm.tm_mon + 1, 2)
            << "-" << number_trailing_zeros_format(local_tm.tm_year + 1900, 4);

        // Test timestamp_to_string
        ASSERT_EQ(now_time_str, expected_string_os.str());

        // Test string_to_timestamp
        Timestamp now_time_from_str = string_to_timestamp(now_time_str, format);
        // NOTE: cannot directly compare timestamps because some precision is lost during ts->str conversion
        ASSERT_EQ(timestamp_to_string(now_time_from_str, format), expected_string_os.str());
    }

    // old time
    {
        Timestamp old_time = date_to_timestamp(1970u, 7u, 20u, 6u, 39u, 42u);
        std::string old_time_str = timestamp_to_string(old_time);

        std::ostringstream expected_string_os;
        expected_string_os
            << 1970
            << "-" << "07"
            << "-" << 20
            << "_" << "06"
            << "-" << 39
            << "-" << 42;

        // Test timestamp_to_string
        ASSERT_EQ(old_time_str, expected_string_os.str());

        // Test string_to_timestamp
        Timestamp old_time_from_str = string_to_timestamp(old_time_str);
        // NOTE: cannot directly compare timestamps because some precision is lost during ts->str conversion
        ASSERT_EQ(timestamp_to_string(old_time_from_str), expected_string_os.str());
    }

    // future time
    {
        Timestamp future_time = date_to_timestamp(2233u, 5u, 22u);
        std::string future_time_str = timestamp_to_string(future_time);

        std::ostringstream expected_string_os;
        expected_string_os
            << 2233
            << "-" << "05"
            << "-" << 22
            << "_" << "00"
            << "-" << "00"
            << "-" << "00";

        // Test timestamp_to_string
        ASSERT_EQ(future_time_str, expected_string_os.str());

        // Test string_to_timestamp
        Timestamp future_time_from_str = string_to_timestamp(future_time_str);
        // NOTE: cannot directly compare timestamps because some precision is lost during ts->str conversion
        ASSERT_EQ(timestamp_to_string(future_time_from_str), expected_string_os.str());
    }

    // some time today
    {
        Timestamp some_time_today = time_to_timestamp(13u, 13u, 13u);
        std::string some_time_today_str = timestamp_to_string(some_time_today);

        // Get current timestamp and use its date to construct expected string
        Timestamp now_ts = now();
        std::string now_time_str = timestamp_to_string(now_ts);
        time_t now_time = std::chrono::system_clock::to_time_t(now_ts);
        std::tm now_tm = *gmtime(&now_time);

        std::ostringstream expected_string_os;
        expected_string_os
            << number_trailing_zeros_format(now_tm.tm_year + 1900, 4)
            << "-" << number_trailing_zeros_format(now_tm.tm_mon + 1, 2)
            << "-" << number_trailing_zeros_format(now_tm.tm_mday, 2)
            << "_" << "13"
            << "-" << "13"
            << "-" << "13";

        // Test timestamp_to_string
        ASSERT_EQ(some_time_today_str, expected_string_os.str());

        // Test string_to_timestamp
        Timestamp some_time_today_from_str = string_to_timestamp(some_time_today_str);
        // NOTE: cannot directly compare timestamps because some precision is lost during ts->str conversion
        ASSERT_EQ(timestamp_to_string(some_time_today_from_str), expected_string_os.str());
    }
}

/**
 * Test function timestamp_to_string with a time stamp with local time, as well the inverse conversion string_to_timestamp.
 *
 * NOTE: only case is now because the local time zone depends on the part of the year and the test becomes a mess.
 */
TEST(time_utils_test, timestamp_to_string_to_timestamp_local)
{
    Timestamp now_time = now();
    std::string format = "%Y-%m-%d_%H-%M-%S";
    std::string now_time_str = timestamp_to_string(now_time, format, true);

    time_t time = std::chrono::system_clock::to_time_t(now_time);
    std::tm local_tm = *localtime(&time);


    std::ostringstream expected_string_os;
    expected_string_os
        << number_trailing_zeros_format(local_tm.tm_year + 1900, 4)
        << "-" << number_trailing_zeros_format(local_tm.tm_mon + 1, 2)
        << "-" << number_trailing_zeros_format(local_tm.tm_mday, 2)
        << "_" << number_trailing_zeros_format(local_tm.tm_hour, 2)
        << "-" << number_trailing_zeros_format(local_tm.tm_min, 2)
        << "-" << number_trailing_zeros_format(local_tm.tm_sec, 2);

    // Test timestamp_to_string
    ASSERT_EQ(now_time_str, expected_string_os.str());

    // Test string_to_timestamp
    Timestamp now_time_from_str = string_to_timestamp(now_time_str, format, true);
    // NOTE: cannot directly compare timestamps because some precision is lost during ts->str conversion
    ASSERT_EQ(timestamp_to_string(now_time_from_str, format, true), expected_string_os.str());
}

/**
 * Test function timestamp_to_string with a time stamp changing format string.
 *
 * CASES:
 * - time zone
 * - time zone offset from UTC
 * - year
 * - 2 digits year
 * - month
 * - month abbreviation
 * - month name
 * - day of the month
 * - week day
 * - day of the year
 * - hours
 * - minutes
 * - seconds
 * - hours:minutes:seconds
 * - year & seconds (with strange format)
 * - seconds & TZone & week day (with strange format)
 * - TZone & TZone deviation (with local date)
 *
 * NOTE: There are more available values to format. These are the most common.
 */
TEST(time_utils_test, timestamp_to_string_format)
{
    // The 20th of July 1969 at 6h 39min 42s the man put a foot in the moon.
    // However, we use here 1970 because windows time format does not allow years before 1970
    auto date = date_to_timestamp(1970u, 7u, 20u, 6u, 39u, 42u);

    // time zone
    {
        // String for Time Zone is different in Windows or Linux
#if _EPROSIMA_WINDOWS_PLATFORM
        std::string expected_str = "Coordinated Universal Time";
#else
        std::string expected_str = "GMT";
#endif  // _EPROSIMA_WINDOWS_PLATFORM

        std::string date_str = timestamp_to_string(date, "%Z");
        ASSERT_EQ(date_str, expected_str);
    }

    // time zone offset from UTC
    {
        // String for Time Zone offset is different in Windows or Linux
        // Thus the first char (+/-) is removed because it is arbitrary (or looks like it) in Windows.
        std::string expected_str = "0000";
        std::string date_str = timestamp_to_string(date, "%z");
        date_str = date_str.substr(1);
        ASSERT_EQ(date_str, expected_str);
    }

    // year
    {
        std::string expected_str = "1970";
        std::string date_str = timestamp_to_string(date, "%Y");
        ASSERT_EQ(date_str, expected_str);
    }

    // 2 digits year
    {
        std::string expected_str = "70";
        std::string date_str = timestamp_to_string(date, "%y");
        ASSERT_EQ(date_str, expected_str);
    }

    // month
    {
        std::string expected_str = "07";
        std::string date_str = timestamp_to_string(date, "%m");
        ASSERT_EQ(date_str, expected_str);
    }

    // month abbreviation
    {
        std::string expected_str = "Jul";
        std::string date_str = timestamp_to_string(date, "%b");
        ASSERT_EQ(date_str, expected_str);
    }

    // month name
    {
        std::string expected_str = "July";
        std::string date_str = timestamp_to_string(date, "%B");
        ASSERT_EQ(date_str, expected_str);
    }

    // day of the month
    {
        std::string expected_str = "20";
        std::string date_str = timestamp_to_string(date, "%d");
        ASSERT_EQ(date_str, expected_str);
    }

    // week day
    {
        std::string expected_str = "1";
        std::string date_str = timestamp_to_string(date, "%w");
        ASSERT_EQ(date_str, expected_str);
    }

    // day of the year
    {
        std::string expected_str = "201";
        std::string date_str = timestamp_to_string(date, "%j");
        ASSERT_EQ(date_str, expected_str);
    }

    // hours
    {
        std::string expected_str = "06";
        std::string date_str = timestamp_to_string(date, "%H");
        ASSERT_EQ(date_str, expected_str);
    }

    // minutes
    {
        std::string expected_str = "39";
        std::string date_str = timestamp_to_string(date, "%M");
        ASSERT_EQ(date_str, expected_str);
    }

    // seconds
    {
        std::string expected_str = "42";
        std::string date_str = timestamp_to_string(date, "%S");
        ASSERT_EQ(date_str, expected_str);
    }

    // hours:minutes:seconds
    {
        std::string expected_str = "06:39:42";
        std::string date_str = timestamp_to_string(date, "%T");
        ASSERT_EQ(date_str, expected_str);
    }

    // year & seconds (with strange format)
    {
        std::string expected_str = "_1970_-_42_";
        std::string date_str = timestamp_to_string(date, "_%Y_-_%S_");
        ASSERT_EQ(date_str, expected_str);
    }

    // seconds & TZone & week day (with strange format)
    {
        std::string expected_str = "42::1";
        std::string date_str = timestamp_to_string(date, "%S::%w");
        ASSERT_EQ(date_str, expected_str);
    }

    // TZone & TZone deviation (with local date)
    {
        auto date_now = now();
        std::string this_time_zone_str = timestamp_to_string(date_now, "%Z", true);
        std::string this_time_zone_dev_str = timestamp_to_string(date_now, "%z", true);

        std::ostringstream expected_ostr;
        expected_ostr << " " << this_time_zone_str << "( " << this_time_zone_dev_str << " ) ";

        std::string date_str = timestamp_to_string(date_now, " %Z( %z ) ", true);
        ASSERT_EQ(date_str, expected_ostr.str());
    }
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
