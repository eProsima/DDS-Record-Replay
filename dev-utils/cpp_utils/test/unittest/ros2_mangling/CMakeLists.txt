# Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#################
# Demangle Test #
#################

set(TEST_NAME ROS2ManglingTest)

set(TEST_SOURCES
        ROS2ManglingTest.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/exception/Exception.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/Formatter.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/math/math_extension.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/utils.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/ros2_mangling.cpp
    )

set(TEST_LIST
        remove_prefix
        add_prefix
        add_suffix
        get_ros_prefix_if_exists
        remove_ros_prefix_if_exists
        add_ros_topic_prefix
        add_ros_service_requester_prefix
        add_ros_service_response_prefix
        get_all_ros_prefixes
        demangle_if_ros_topic
        mangle_if_ros_topic
        demangle_if_ros_type
        mangle_if_ros_type
        demangle_ros_topic_prefix_from_topic
        demangle_ros_service_prefix_from_topic
        demangle_ros_service_request_prefix_from_topic
        mangle_ros_service_request_prefix_in_topic
        demangle_ros_service_reply_prefix_from_topic
        mangle_ros_service_reply_prefix_in_topic
        demangle_service_type_only
        mangle_service_type_only
    )

set(TEST_EXTRA_LIBRARIES
        fastcdr
        fastrtps
        $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )

add_unittest_executable(
        "${TEST_NAME}"
        "${TEST_SOURCES}"
        "${TEST_LIST}"
        "${TEST_EXTRA_LIBRARIES}"
    )
