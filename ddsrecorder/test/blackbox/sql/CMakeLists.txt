# Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

########################
# SqlFileCreationTest #
########################

set(TEST_NAME SqlFileCreationTest)

file(
    GLOB_RECURSE TEST_SOURCES
    SqlFileCreationTest.cpp
    "${PROJECT_SOURCE_DIR}/test/resources/types/hello_world/*.c*"
    )

file(
    GLOB_RECURSE TEST_LIBRARY_SOURCES
    "${PROJECT_SOURCE_DIR}/src/cpp/*.c*"
    )

list(FILTER TEST_LIBRARY_SOURCES EXCLUDE REGEX "/main.cpp")

# Exclude types' files depending on the Fast DDS version
if ("${fastrtps_VERSION}" VERSION_LESS 2.13)
    list(FILTER TEST_SOURCES EXCLUDE REGEX "v2/")
    list(FILTER TEST_LIBRARY_SOURCES EXCLUDE REGEX "v2/")
else()
    list(FILTER TEST_SOURCES EXCLUDE REGEX "v1/")
    list(FILTER TEST_LIBRARY_SOURCES EXCLUDE REGEX "v1/")
endif()

set(TEST_LIST
        sql_data_msgs
        sql_dds_topic
        sql_ros2_topic
        sql_data_num_msgs
        sql_data_num_msgs_downsampling

        # State
        transition_running
        transition_paused
        transition_suspended
        transition_stopped

        # Transitions
        transition_running_paused
        transition_running_suspended
        transition_running_stopped
        transition_paused_running
        transition_paused_suspended
        transition_paused_stopped
        transition_suspended_running
        transition_suspended_paused
        transition_suspended_stopped
        transition_stopped_running
        transition_stopped_suspended
        transition_stopped_paused

        # Event window
        transition_paused_event_less_window
        transition_paused_event_max_window

        # Events
        transition_paused_event_start
        transition_paused_event_suspend
        transition_paused_event_stop
    )

set(TEST_NEEDED_SOURCES
    )

set(TEST_EXTRA_HEADERS
    ${PROJECT_SOURCE_DIR}/test/resources/types/hello_world/
    )

add_blackbox_executable(
    "${TEST_NAME}"
    "${TEST_SOURCES}"
    "${TEST_LIST}"
    "${TEST_NEEDED_SOURCES}"
    "${TEST_EXTRA_HEADERS}"
    "${TEST_LIBRARY_SOURCES}"
)
