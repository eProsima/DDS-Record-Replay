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
    "${PROJECT_SOURCE_DIR}/../thirdparty/sqlite/*.c"
    )

set(TEST_LIBRARY_SOURCES
    )

set(TEST_LIST
        sql_dds_topic
        sql_ros2_topic
        sql_data_num_msgs
        sql_data_num_msgs_partition
        sql_data_num_msgs_downsampling

        sql_data_format_cdr
        sql_data_format_json
        sql_data_format_both

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


#################################
# SqlFileCreationPartitionTest #
#################################

set(TEST_NAME SqlFileCreationPartitionTest)

file(
    GLOB_RECURSE TEST_SOURCES
    SqlFileCreationPartitionTest.cpp
    "${PROJECT_SOURCE_DIR}/test/resources/types/hello_world/*.c*"
    "${PROJECT_SOURCE_DIR}/../thirdparty/sqlite/*.c"
    )

set(TEST_LIBRARY_SOURCES
    )

set(TEST_LIST
        sql_dds_topic_partition
        sql_ros2_topic_partition
        sql_data_num_msgs_partition
        sql_data_num_msgs_partition_partition
        sql_data_num_msgs_downsampling_partition

        sql_data_format_cdr_partition
        sql_data_format_json_partition
        sql_data_format_both_partition

        # State
        transition_running_partition
        transition_paused_partition
        transition_suspended_partition
        transition_stopped_partition

        # Transitions
        transition_running_paused_partition
        transition_running_suspended_partition
        transition_running_stopped_partition
        transition_paused_running_partition
        transition_paused_suspended_partition
        transition_paused_stopped_partition
        transition_suspended_running_partition
        transition_suspended_paused_partition
        transition_suspended_stopped_partition
        transition_stopped_running_partition
        transition_stopped_suspended_partition
        transition_stopped_paused_partition

        # Event window
        transition_paused_event_less_window_partition
        transition_paused_event_max_window_partition

        # Events
        transition_paused_event_start_partition
        transition_paused_event_suspend_partition
        transition_paused_event_stop_partition

        sql_dds_topic_partition_no_record
        sql_ros2_topic_partition_no_record
        sql_data_num_msgs_partition_no_record
        sql_data_num_msgs_partition_no_record_partition_no_record
        sql_data_num_msgs_downsampling_partition_no_record

        sql_data_format_cdr_partition_no_record
        sql_data_format_json_partition_no_record
        sql_data_format_both_partition_no_record

        # State
        transition_running_partition_no_record
        transition_paused_partition_no_record
        transition_suspended_partition_no_record
        transition_stopped_partition_no_record

        # Transitions
        transition_running_paused_partition_no_record
        transition_running_suspended_partition_no_record
        transition_running_stopped_partition_no_record
        transition_paused_running_partition_no_record
        transition_paused_suspended_partition_no_record
        transition_paused_stopped_partition_no_record
        transition_suspended_running_partition_no_record
        transition_suspended_paused_partition_no_record
        transition_suspended_stopped_partition_no_record
        transition_stopped_running_partition_no_record
        transition_stopped_suspended_partition_no_record
        transition_stopped_paused_partition_no_record
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



###################
# SqlLogErrorTest #
###################

set(TEST_NAME SqlLogErrorTest)

set(TEST_SOURCES
        SqlLogErrorTest.cpp
    )

set(TEST_LIST
        fail_to_open_non_existent_file
        fail_to_open_empty_folder
    )

set(EXTRA_LIBRARIES
        ${MODULE_FIND_PACKAGES}
    )


add_blackbox_executable(
    "${TEST_NAME}"
    "${TEST_SOURCES}"
    "${TEST_LIST}"
    "${TEST_NEEDED_SOURCES}"
    "${EXTRA_LIBRARIES}"
    "${TEST_EXTRA_HEADERS}"
)
