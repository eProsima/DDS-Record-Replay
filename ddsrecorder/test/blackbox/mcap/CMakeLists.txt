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

########################
# McapFileCreationTest #
########################

set(TEST_NAME McapFileCreationTest)

# Determine Fast DDS version
if ("${fastrtps_VERSION}" VERSION_LESS 2.13)
    set(DDS_TYPES_VERSION "v1")
else()
    set(DDS_TYPES_VERSION "v2")
endif()

file(
    GLOB_RECURSE TEST_LIBRARY_SOURCES
        "${PROJECT_SOURCE_DIR}/src/cpp/tool/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/cpp/user_interface/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/cpp/command_receiver/Command*.cpp"
        "${PROJECT_SOURCE_DIR}/src/cpp/*/*/${DDS_TYPES_VERSION}/*/*.cxx"
    )

set(TEST_SOURCES
        McapFileCreationTest.cpp
        ${PROJECT_SOURCE_DIR}/test/blackbox/mcap/types/${DDS_TYPES_VERSION}/hello_world/HelloWorld.cxx
        ${PROJECT_SOURCE_DIR}/test/blackbox/mcap/types/${DDS_TYPES_VERSION}/hello_world/HelloWorldPubSubTypes.cxx
        ${PROJECT_SOURCE_DIR}/test/blackbox/mcap/types/${DDS_TYPES_VERSION}/hello_world/HelloWorldTypeObject.cxx
        $<$<STREQUAL:${DDS_TYPES_VERSION},v2>:${PROJECT_SOURCE_DIR}/test/blackbox/mcap/types/v2/hello_world/HelloWorldv1.cxx>
    )

set(TEST_LIST
        mcap_data_msgs
        mcap_dds_topic
        mcap_ros2_topic
        mcap_data_num_msgs
        mcap_data_num_msgs_downsampling
        transition_running
        transition_paused
        transition_stopped
        transition_suspended
        transition_running_paused
        transition_running_stopped
        transition_running_suspended
        transition_paused_running
        transition_paused_stopped
        transition_paused_suspended
        transition_stopped_running
        transition_stopped_paused
        transition_stopped_suspended
        transition_suspended_running
        transition_suspended_paused
        transition_suspended_stopped
        transition_paused_event_less_window
        transition_paused_event_max_window
        transition_paused_event_start
        transition_paused_event_stop
        transition_paused_event_suspend
    )

set(TEST_NEEDED_SOURCES
    )

set(TEST_EXTRA_HEADERS
    ${PROJECT_SOURCE_DIR}/test/blackbox/mcap/types/${DDS_TYPES_VERSION}/hello_world
    ${PROJECT_SOURCE_DIR}/test/blackbox/mcap/types/${DDS_TYPES_VERSION})

add_blackbox_executable(
    "${TEST_NAME}"
    "${TEST_SOURCES}"
    "${TEST_LIST}"
    "${TEST_NEEDED_SOURCES}"
    "${TEST_EXTRA_HEADERS}"
    "${TEST_LIBRARY_SOURCES}"
)


############################
# Disk Full Log Error Test #
############################

set(TEST_NAME DiskFullLogErrorMockTest)

set(TEST_SOURCES
        DiskFullLogErrorMockTest.cpp
    )

set(TEST_LIST
        log_error_when_disk_is_full
    )

set(EXTRA_LIBRARIES
        ${MODULE_FIND_PACKAGES}
        $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )


add_blackbox_executable(
    "${TEST_NAME}"
    "${TEST_SOURCES}"
    "${TEST_LIST}"
    "${TEST_NEEDED_SOURCES}"
    "${EXTRA_LIBRARIES}"
    "${TEST_EXTRA_HEADERS}"
)
