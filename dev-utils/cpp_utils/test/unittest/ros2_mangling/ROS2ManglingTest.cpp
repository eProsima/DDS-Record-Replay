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

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>


#include <cpp_utils/ros2_mangling.hpp>

using namespace eprosima::utils;

/**
 * Test remove_prefix() method
 *
 * CASES:
 * - Name: "hello"      Prefix: "world"  Return: ""
 * - Name: "hello/rt"   Prefix: "rt"     Return: ""
 * - Name: "rt/hello"   Prefix: "rt/"    Return: ""
 * - Name: "rt/hello"   Prefix: "rt"     Return: "/hello"
 */
TEST(ROS2ManglingTest, remove_prefix)
{
    EXPECT_EQ("", remove_prefix("hello", "world"));
    EXPECT_EQ("", remove_prefix("hello/rt", "rt"));
    EXPECT_EQ("", remove_prefix("rt/hello", "rt/"));
    EXPECT_EQ("/hello", remove_prefix("rt/hello", "rt"));
}

/**
 * Test add_prefix() method
 *
 * CASES:
 * - Name: "hello"  Prefix: "world"  Return: "worldhello"
 * - Name: "hello"  Prefix: "rt"     Return: "rthello"
 * - Name: "/hello" Prefix: "rt/"    Return: "rt//hello"
 * - Name: "/hello" Prefix: "rt"     Return: "rt/hello"
 */
TEST(ROS2ManglingTest, add_prefix)
{
    EXPECT_EQ("worldhello", add_prefix("hello", "world"));
    EXPECT_EQ("rthello", add_prefix("hello", "rt"));
    EXPECT_EQ("rt//hello", add_prefix("/hello", "rt/"));
    EXPECT_EQ("rt/hello", add_prefix("/hello", "rt"));
}

/**
 * Test add_suffix() method
 *
 * CASES:
 * - Name: "hello"  Suffix: "world"  Return: "helloworld"
 * - Name: "hello"  Suffix: "rt"     Return: "hellort"
 * - Name: "/hello" Suffix: "rt/"    Return: "/hellort/"
 * - Name: "/hello" Suffix: "rt"     Return: "/hellort"
 * - Name: "rt/"    Suffix: "hello"  Return: "rt/hello"
 */
TEST(ROS2ManglingTest, add_suffix)
{
    EXPECT_EQ("helloworld", add_suffix("hello", "world"));
    EXPECT_EQ("hellort", add_suffix("hello", "rt"));
    EXPECT_EQ("/hellort/", add_suffix("/hello", "rt/"));
    EXPECT_EQ("/hellort", add_suffix("/hello", "rt"));
    EXPECT_EQ("rt/hello", add_suffix("rt/", "hello"));
}

/**
 * Test get_ros_prefix_if_exists() method
 *
 * CASES:
 * - ROS 2 Topic Name: "hello"      Return: ""
 * - ROS 2 Topic Name: "hello/rt"   Return: ""
 * - ROS 2 Topic Name: "rt/hello"   Return: "rt"
 * - ROS 2 Topic Name: "/rt/hello"  Return: ""
 * - ROS 2 Topic Name: "hello/rq"   Return: ""
 * - ROS 2 Topic Name: "rq/hello"   Return: "rq"
 * - ROS 2 Topic Name: "/rq/hello"  Return: ""
 * - ROS 2 Topic Name: "hello/rr"   Return: ""
 * - ROS 2 Topic Name: "rr/hello"   Return: "rr"
 * - ROS 2 Topic Name: "/rr/hello"  Return: ""
 */
TEST(ROS2ManglingTest, get_ros_prefix_if_exists)
{
    EXPECT_EQ("", get_ros_prefix_if_exists("hello"));

    // ros_topic_prefix
    EXPECT_EQ("", get_ros_prefix_if_exists("hello/rt"));
    EXPECT_EQ("rt", get_ros_prefix_if_exists("rt/hello"));
    EXPECT_EQ("", get_ros_prefix_if_exists("/rt/hello"));

    // ros_service_requester_prefix
    EXPECT_EQ("", get_ros_prefix_if_exists("hello/rq"));
    EXPECT_EQ("rq", get_ros_prefix_if_exists("rq/hello"));
    EXPECT_EQ("", get_ros_prefix_if_exists("/rq/hello"));

    // ros_service_requester_prefix
    EXPECT_EQ("", get_ros_prefix_if_exists("hello/rr"));
    EXPECT_EQ("rr", get_ros_prefix_if_exists("rr/hello"));
    EXPECT_EQ("", get_ros_prefix_if_exists("/rr/hello"));
}

/**
 * Test remove_ros_prefix_if_exists() method
 *
 * CASES:
 * - ROS 2 Topic Name: "hello"      Return: ""
 * - ROS 2 Topic Name: "hello/rt"   Return: "hello/rt"
 * - ROS 2 Topic Name: "rt/hello"   Return: "/hello"
 * - ROS 2 Topic Name: "/rt/hello"  Return: "/rt/hello"
 * - ROS 2 Topic Name: "hello/rq"   Return: "hello/rq"
 * - ROS 2 Topic Name: "rq/hello"   Return: "/hello"
 * - ROS 2 Topic Name: "/rq/hello"  Return: "/rq/hello"
 * - ROS 2 Topic Name: "hello/rr"   Return: "hello/rr"
 * - ROS 2 Topic Name: "rr/hello"   Return: "/hello"
 * - ROS 2 Topic Name: "/rr/hello"  Return: "/rr/hello"
 */
TEST(ROS2ManglingTest, remove_ros_prefix_if_exists)
{
    EXPECT_EQ("hello", remove_ros_prefix_if_exists("hello"));

    // ros_topic_prefix
    EXPECT_EQ("hello/rt", remove_ros_prefix_if_exists("hello/rt"));
    EXPECT_EQ("/hello", remove_ros_prefix_if_exists("rt/hello"));
    EXPECT_EQ("/rt/hello", remove_ros_prefix_if_exists("/rt/hello"));

    // ros_service_requester_prefix
    EXPECT_EQ("hello/rq", remove_ros_prefix_if_exists("hello/rq"));
    EXPECT_EQ("/hello", remove_ros_prefix_if_exists("rq/hello"));
    EXPECT_EQ("/rq/hello", remove_ros_prefix_if_exists("/rq/hello"));

    // ros_service_requester_prefix
    EXPECT_EQ("hello/rr", remove_ros_prefix_if_exists("hello/rr"));
    EXPECT_EQ("/hello", remove_ros_prefix_if_exists("rr/hello"));
    EXPECT_EQ("/rr/hello", remove_ros_prefix_if_exists("/rr/hello"));
}

/**
 * Test add_ros_topic_prefix() method
 *
 * CASES:
 * - ROS 2 Topic Name: "hello"     Return: "rthello"
 * - ROS 2 Topic Name: "hello/rt"  Return: "rthello/rt"
 * - ROS 2 Topic Name: "/hello"    Return: "rt/hello"
 * - ROS 2 Topic Name: "rt/hello"  Return: "rtrt/hello"
 */
TEST(ROS2ManglingTest, add_ros_topic_prefix)
{
    EXPECT_EQ("rthello", add_ros_topic_prefix("hello"));
    EXPECT_EQ("rthello/rt", add_ros_topic_prefix("hello/rt"));
    EXPECT_EQ("rt/hello", add_ros_topic_prefix("/hello"));
    EXPECT_EQ("rtrt/hello", add_ros_topic_prefix("rt/hello"));
}

/**
 * Test add_ros_service_requester_prefix() method
 *
 * CASES:
 * - ROS 2 Topic Name: "hello"     Return: "rqhello"
 * - ROS 2 Topic Name: "hello/rq"  Return: "rqhello/rq"
 * - ROS 2 Topic Name: "/hello"    Return: "rq/hello"
 * - ROS 2 Topic Name: "rq/hello"  Return: "rqrq/hello"
 */
TEST(ROS2ManglingTest, add_ros_service_requester_prefix)
{
    EXPECT_EQ("rqhello", add_ros_service_requester_prefix("hello"));
    EXPECT_EQ("rqhello/rq", add_ros_service_requester_prefix("hello/rq"));
    EXPECT_EQ("rq/hello", add_ros_service_requester_prefix("/hello"));
    EXPECT_EQ("rqrq/hello", add_ros_service_requester_prefix("rq/hello"));
}

/**
 * Test add_ros_service_response_prefix() method
 *
 * CASES:
 * - ROS 2 Topic Name: "hello"     Return: "rrhello"
 * - ROS 2 Topic Name: "hello/rr"  Return: "rrhello/rr"
 * - ROS 2 Topic Name: "/hello"    Return: "rr/hello"
 * - ROS 2 Topic Name: "rr/hello"  Return: "rrrr/hello"
 */
TEST(ROS2ManglingTest, add_ros_service_response_prefix)
{
    EXPECT_EQ("rrhello", add_ros_service_response_prefix("hello"));
    EXPECT_EQ("rrhello/rr", add_ros_service_response_prefix("hello/rr"));
    EXPECT_EQ("rr/hello", add_ros_service_response_prefix("/hello"));
    EXPECT_EQ("rrrr/hello", add_ros_service_response_prefix("rr/hello"));
}

/**
 * Test get_all_ros_prefixes() method
 */
TEST(ROS2ManglingTest, get_all_ros_prefixes)
{
    const std::vector<std::string> ros_prefixes = {"rt", "rq", "rr"};
    EXPECT_EQ(ros_prefixes, get_all_ros_prefixes());
}

/**
 * Test demangle_if_ros_topic() method
 *
 * CASES:
 * - Topic Name: "hello"      Return: "hello"
 * - Topic Name: "hello/rt"   Return: "hello/rt"
 * - Topic Name: "rt/hello"   Return: "/hello"
 * - Topic Name: "/rt/hello"  Return: "/rt/hello"
 * - Topic Name: "hello/rq"   Return: "hello/rq"
 * - Topic Name: "rq/hello"   Return: "/hello"
 * - Topic Name: "/rq/hello"  Return: "/rq/hello"
 * - Topic Name: "hello/rr"   Return: "hello/rr"
 * - Topic Name: "rr/hello"   Return: "/hello"
 * - Topic Name: "/rr/hello"  Return: "/rr/hello"
 */
TEST(ROS2ManglingTest, demangle_if_ros_topic)
{
    EXPECT_EQ("hello", demangle_if_ros_topic("hello"));

    // ros_topic_prefix
    EXPECT_EQ("hello/rt", demangle_if_ros_topic("hello/rt"));
    EXPECT_EQ("/hello", demangle_if_ros_topic("rt/hello"));
    EXPECT_EQ("/rt/hello", demangle_if_ros_topic("/rt/hello"));

    // ros_service_requester_prefix
    EXPECT_EQ("hello/rq", demangle_if_ros_topic("hello/rq"));
    EXPECT_EQ("/hello", demangle_if_ros_topic("rq/hello"));
    EXPECT_EQ("/rq/hello", demangle_if_ros_topic("/rq/hello"));

    // ros_service_requester_prefix
    EXPECT_EQ("hello/rr", demangle_if_ros_topic("hello/rr"));
    EXPECT_EQ("/hello", demangle_if_ros_topic("rr/hello"));
    EXPECT_EQ("/rr/hello", demangle_if_ros_topic("/rr/hello"));
}

/**
 * Test mangle_if_ros_topic() method
 *
 * CASES:
 * - Topic Name: "hello"      Return: "hello"
 * - Topic Name: "hello/rt"   Return: "hello/rt"
 * - Topic Name: "/hello"     Return: "rt/hello"
 * - Topic Name: "/rt/hello"  Return: "rt/rt/hello"
 */
TEST(ROS2ManglingTest, mangle_if_ros_topic)
{
    EXPECT_EQ("hello", mangle_if_ros_topic("hello"));
    EXPECT_EQ("hello/rt", mangle_if_ros_topic("hello/rt"));
    EXPECT_EQ("rt/hello", mangle_if_ros_topic("/hello"));
    EXPECT_EQ("rt/rt/hello", mangle_if_ros_topic("/rt/hello"));
}

/**
 * Test demangle_if_ros_type() method
 *
 * CASES:
 * - Type Name: "hello"                         Return: "hello"
 * - Type Name: "msg::dds_"                     Return: "msg::dds_"
 * - Type Name: "msgs::msg::"                   Return: "msgs::msg::"
 * - Type Name: "msgs::msg::dds_"               Return: "msgs::msg::dds_"
 * - Type Name: "msg::dds_::hello"              Return: "msg::dds_::hello"
 * - Type Name: "std_msgs::msg::dds_::String_"  Return: "std_msgs/msg/String"
 */
TEST(ROS2ManglingTest, demangle_if_ros_type)
{
    // not a ROS type
    EXPECT_EQ("hello", demangle_if_ros_type("hello"));
    EXPECT_EQ("msg::dds_", demangle_if_ros_type("msg::dds_"));
    EXPECT_EQ("msgs::msg::", demangle_if_ros_type("msgs::msg::"));
    EXPECT_EQ("msgs::msg::dds_", demangle_if_ros_type("msgs::msg::dds_"));
    EXPECT_EQ("msgs::msg::hello", demangle_if_ros_type("msgs::msg::hello"));

    // ROS type
    EXPECT_EQ("std_msgs/msg/String", demangle_if_ros_type("std_msgs::msg::dds_::String_"));
}

/**
 * Test mangle_if_ros_type() method
 *
 * CASES:
 * - Type Name: "hello"                Return: "hello"
 * - Type Name: "msg"                  Return: "msg"
 * - Type Name: "std_msgs/msg/"        Return: "std_msgs/msg/"
 * - Type Name: "std_msgs/msg/String"  Return: "std_msgs::msg::dds_::String_"
 */
TEST(ROS2ManglingTest, mangle_if_ros_type)
{
    // not a ROS type
    EXPECT_EQ("hello", mangle_if_ros_type("hello"));
    EXPECT_EQ("msg", mangle_if_ros_type("msg"));
    EXPECT_EQ("std_msgs/msg/", mangle_if_ros_type("std_msgs/msg/"));

    // ROS type
    EXPECT_EQ("std_msgs::msg::dds_::String_", mangle_if_ros_type("std_msgs/msg/String"));
}

/**
 * Test demangle_ros_topic_prefix_from_topic() method
 *
 * CASES:
 * - Topic Name: "hello"     Return: ""
 * - Topic Name: "hello/rt"  Return: ""
 * - Topic Name: "rt/hello"  Return: "/hello"
 * - Topic Name: "hello/rq"  Return: ""
 * - Topic Name: "rq/hello"  Return: ""
 * - Topic Name: "hello/rr"  Return: ""
 * - Topic Name: "rr/hello"  Return: ""
 */
TEST(ROS2ManglingTest, demangle_ros_topic_prefix_from_topic)
{
    EXPECT_EQ("", demangle_ros_topic_prefix_from_topic("hello"));
    EXPECT_EQ("", demangle_ros_topic_prefix_from_topic("hello/rt"));
    EXPECT_EQ("/hello", demangle_ros_topic_prefix_from_topic("rt/hello"));

    EXPECT_EQ("", demangle_ros_topic_prefix_from_topic("hello/rq"));
    EXPECT_EQ("", demangle_ros_topic_prefix_from_topic("rq/hello"));

    EXPECT_EQ("", demangle_ros_topic_prefix_from_topic("hello/rr"));
    EXPECT_EQ("", demangle_ros_topic_prefix_from_topic("rr/hello"));
}

/**
 * Test demangle_ros_service_prefix_from_topic() method
 *
 * CASES:
 * - Service Name: "hello"                  Return: ""
 * - Service Name: "rq/hello"               Return: ""
 * - Service Name: "rr/hello"               Return: ""
 * - Service Name: "rq/hello/worldRequest"  Return: "/hello/world"
 * - Service Name: "rr/hello/worldReply"    Return: "/hello/world"
 * - Service Name: "Request/hello/worldrq"  Return: ""
 * - Service Name: "Reply/hello/worldrr"    Return: ""
 */
TEST(ROS2ManglingTest, demangle_ros_service_prefix_from_topic)
{
    EXPECT_EQ("", demangle_ros_service_prefix_from_topic("hello"));

    EXPECT_EQ("", demangle_ros_service_prefix_from_topic("rq/hello"));
    EXPECT_EQ("", demangle_ros_service_prefix_from_topic("rr/hello"));

    EXPECT_EQ("/hello/world", demangle_ros_service_prefix_from_topic("rq/hello/worldRequest"));
    EXPECT_EQ("/hello/world", demangle_ros_service_prefix_from_topic("rr/hello/worldReply"));

    EXPECT_EQ("", demangle_ros_service_prefix_from_topic("Request/hello/worldrq"));
    EXPECT_EQ("", demangle_ros_service_prefix_from_topic("Reply/hello/worldrr"));
}

/**
 * Test demangle_ros_service_request_prefix_from_topic() method
 *
 * CASES:
 * - Service Request Name: "hello"                  Return: ""
 * - Service Request Name: "rq/hello"               Return: ""
 * - Service Request Name: "rr/hello"               Return: ""
 * - Service Request Name: "rq/hello/worldRequest"  Return: "/hello/world"
 * - Service Request Name: "rr/hello/worldReply"    Return: ""
 * - Service Request Name: "Request/hello/worldrq"  Return: ""
 * - Service Request Name: "Reply/hello/worldrr"    Return: ""
 */
TEST(ROS2ManglingTest, demangle_ros_service_request_prefix_from_topic)
{
    EXPECT_EQ("", demangle_ros_service_request_prefix_from_topic("hello"));

    EXPECT_EQ("", demangle_ros_service_request_prefix_from_topic("rq/hello"));
    EXPECT_EQ("", demangle_ros_service_request_prefix_from_topic("rr/hello"));

    EXPECT_EQ("/hello/world", demangle_ros_service_request_prefix_from_topic("rq/hello/worldRequest"));
    EXPECT_EQ("", demangle_ros_service_request_prefix_from_topic("rr/hello/worldReply"));

    EXPECT_EQ("", demangle_ros_service_request_prefix_from_topic("Request/hello/worldrq"));
    EXPECT_EQ("", demangle_ros_service_request_prefix_from_topic("Reply/hello/worldrr"));
}

/**
 * Test mangle_ros_service_request_prefix_in_topic() method
 *
 * CASES:
 * - Service Request Name: "hello"     Return: ""
 * - Service Request Name: "rq/hello"  Return: ""
 * - Service Request Name: "rr/hello"  Return: ""
 * - Service Request Name: "/hello"    Return: "rq/helloRequest"
 */
TEST(ROS2ManglingTest, mangle_ros_service_request_prefix_in_topic)
{
    EXPECT_EQ("", mangle_ros_service_request_prefix_in_topic("hello"));

    EXPECT_EQ("", mangle_ros_service_request_prefix_in_topic("rq/hello"));
    EXPECT_EQ("", mangle_ros_service_request_prefix_in_topic("rr/hello"));

    EXPECT_EQ("rq/helloRequest", mangle_ros_service_request_prefix_in_topic("/hello"));
}

/**
 * Test demangle_ros_service_reply_prefix_from_topic() method
 *
 * CASES:
 * - Service Reply Name: "hello"                  Return: ""
 * - Service Reply Name: "rq/hello"               Return: ""
 * - Service Reply Name: "rr/hello"               Return: ""
 * - Service Reply Name: "rq/hello/worldRequest"  Return: ""
 * - Service Reply Name: "rr/hello/worldReply"    Return: "/hello/world"
 * - Service Reply Name: "Request/hello/worldrq"  Return: ""
 * - Service Reply Name: "Reply/hello/worldrr"    Return: ""
 */
TEST(ROS2ManglingTest, demangle_ros_service_reply_prefix_from_topic)
{
    EXPECT_EQ("", demangle_ros_service_reply_prefix_from_topic("hello"));

    EXPECT_EQ("", demangle_ros_service_reply_prefix_from_topic("rq/hello"));
    EXPECT_EQ("", demangle_ros_service_reply_prefix_from_topic("rr/hello"));

    EXPECT_EQ("", demangle_ros_service_reply_prefix_from_topic("rq/hello/worldRequest"));
    EXPECT_EQ("/hello/world", demangle_ros_service_reply_prefix_from_topic("rr/hello/worldReply"));

    EXPECT_EQ("", demangle_ros_service_reply_prefix_from_topic("Request/hello/worldrq"));
    EXPECT_EQ("", demangle_ros_service_reply_prefix_from_topic("Reply/hello/worldrr"));
}

/**
 * Test mangle_ros_service_reply_prefix_in_topic() method
 *
 * CASES:
 * - Service Reply Name: "hello"     Return: ""
 * - Service Reply Name: "rq/hello"  Return: ""
 * - Service Reply Name: "rr/hello"  Return: ""
 * - Service Reply Name: "/hello"    Return: "rr/helloReply"
 */
TEST(ROS2ManglingTest, mangle_ros_service_reply_prefix_in_topic)
{
    EXPECT_EQ("", mangle_ros_service_reply_prefix_in_topic("hello"));

    EXPECT_EQ("", mangle_ros_service_reply_prefix_in_topic("rq/hello"));
    EXPECT_EQ("", mangle_ros_service_reply_prefix_in_topic("rr/hello"));

    EXPECT_EQ("rr/helloReply", mangle_ros_service_reply_prefix_in_topic("/hello"));
}

/**
 * Test demangle_service_type_only() method
 *
 * CASES:
 * - DDS Service Name: "hello"                          Return: ""
 * - DDS Service Name: "rq/hello"                       Return: ""
 * - DDS Service Name: "rr/hello"                       Return: ""
 * - DDS Service Name: "rt/hello"                       Return: ""
 * - DDS Service Name: "rq::dds_::hello"                Return: ""
 * - DDS Service Name: "rr::dds_::hello"                Return: ""
 * - DDS Service Name: "rt::dds_::hello"                Return: ""
 * - DDS Service Name: "rq::srv::dds_::hello_Request_"  Return: "rq/srv/hello"
 * - DDS Service Name: "rr::srv::dds_::hello_Request_"  Return: "rr/srv/hello"
 */
TEST(ROS2ManglingTest, demangle_service_type_only)
{
    // not a ROS service type
    EXPECT_EQ("", demangle_service_type_only("hello"));
    EXPECT_EQ("", demangle_service_type_only("rq/hello"));
    EXPECT_EQ("", demangle_service_type_only("rr/hello"));
    EXPECT_EQ("", demangle_service_type_only("rt/hello"));

    EXPECT_EQ("", demangle_service_type_only("rq::dds_::hello"));
    EXPECT_EQ("", demangle_service_type_only("rr::dds_::hello"));
    EXPECT_EQ("", demangle_service_type_only("rt::dds_::hello"));

    // ROS service type
    EXPECT_EQ("rq/srv/hello", demangle_service_type_only("rq::srv::dds_::hello_Request_"));
    EXPECT_EQ("rr/srv/hello", demangle_service_type_only("rr::srv::dds_::hello_Response_"));
}

/**
 * Test mangle_service_type_only() method
 *
 * CASES:
 * - DDS Service Name: "hello"            Return: ""
 * - DDS Service Name: "rt/hello"         Return: ""
 * - DDS Service Name: "rq/srv/hello"     Return: "rq::srv::dds_::hello_Request_"
 * - DDS Service Name: "rr/srv/hello"     Return: "rr::srv::dds_::hello_Response_"
 * - DDS Service Name: "/srv/hello"       Return: ""
 * - DDS Service Name: "/srv/hello"       Return: ""
 * - DDS Service Name: "rq::dds_::hello"  Return: ""
 * - DDS Service Name: "rr::dds_::hello"  Return: ""
 * - DDS Service Name: "rt::dds_::hello"  Return: ""
 */
TEST(ROS2ManglingTest, mangle_service_type_only)
{
    EXPECT_EQ("", mangle_service_type_only("hello"));
    EXPECT_EQ("", mangle_service_type_only("rt/hello"));

    EXPECT_EQ("rq::srv::dds_::hello_Request_", mangle_service_type_only("rq/srv/hello"));
    EXPECT_EQ("rr::srv::dds_::hello_Response_", mangle_service_type_only("rr/srv/hello"));

    EXPECT_EQ("", mangle_service_type_only("/srv/hello"));
    EXPECT_EQ("", mangle_service_type_only("/srv/hello"));

    EXPECT_EQ("", mangle_service_type_only("rq::dds_::hello"));
    EXPECT_EQ("", mangle_service_type_only("rr::dds_::hello"));
    EXPECT_EQ("", mangle_service_type_only("rt::dds_::hello"));
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
