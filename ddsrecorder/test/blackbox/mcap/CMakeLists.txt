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

file(
    GLOB_RECURSE TEST_SOURCES
    McapFileCreationTest.cpp
    "${PROJECT_SOURCE_DIR}/test/resources/types/hello_world/*.c*"
    )

set(TEST_LIBRARY_SOURCES
    )

set(TEST_LIST

        mcap_data_msgs
        mcap_dds_topic
        mcap_ros2_topic
        mcap_data_num_msgs
        mcap_data_num_msgs_downsampling

        # State
        transition_running
        transition_paused
        transition_stopped
        transition_suspended

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
        transition_stopped_paused
        transition_stopped_suspended

        # Event window
        transition_paused_event_less_window
        transition_paused_event_max_window

        # Events
        transition_paused_event_start
        transition_paused_event_stop
        transition_paused_event_suspend

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
# McapFileCreationPartitionTest #
#################################

set(TEST_NAME McapFileCreationPartitionTest)

file(
    GLOB_RECURSE TEST_SOURCES
    McapFileCreationPartitionTest.cpp
    "${PROJECT_SOURCE_DIR}/test/resources/types/hello_world/*.c*"
    )

set(TEST_LIBRARY_SOURCES
    )

set(TEST_LIST
        mcap_data_msgs_partition
        mcap_dds_topic_partition
        mcap_ros2_topic_partition
        mcap_data_num_msgs_partition
        mcap_data_num_msgs_downsampling_partition

        # State
        transition_running_partition
        transition_paused_partition
        transition_stopped_partition
        transition_suspended_partition

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
        transition_stopped_paused_partition
        transition_stopped_suspended_partition

        # Event window
        transition_paused_event_less_wi_partitionndow
        transition_paused_event_max_win_partitiondow

        # Events
        transition_paused_event_start_partition
        transition_paused_event_stop_partition
        transition_paused_event_suspend_partition


        mcap_data_msgs_partition_mp_record
        mcap_dds_topic_partition_mp_record
        mcap_ros2_topic_partition_mp_record
        mcap_data_num_msgs_partition_mp_record
        mcap_data_num_msgs_downsampling_partition_mp_record

        # State
        transition_running_partition_mp_record
        transition_paused_partition_mp_record
        transition_stopped_partition_mp_record
        transition_suspended_partition_mp_record

        # Transitions
        transition_running_paused_partition_mp_record
        transition_running_suspended_partition_mp_record
        transition_running_stopped_partition_mp_record
        transition_paused_running_partition_mp_record
        transition_paused_suspended_partition_mp_record
        transition_paused_stopped_partition_mp_record
        transition_suspended_running_partition_mp_record
        transition_suspended_paused_partition_mp_record
        transition_suspended_stopped_partition_mp_record
        transition_stopped_running_partition_mp_record
        transition_stopped_paused_partition_mp_record
        transition_stopped_suspended_partition_mp_record

        # Event window
        transition_paused_event_less_wi_partitionndow_mp_record
        transition_paused_event_max_win_partitiondow_mp_record

        # Events
        transition_paused_event_start_partition_mp_record
        transition_paused_event_stop_partition_mp_record
        transition_paused_event_suspend_partition_mp_record
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

####################
# McapLogErrorTest #
####################

set(TEST_NAME McapLogErrorTest)

set(TEST_SOURCES
        McapLogErrorTest.cpp
    )

set(TEST_LIST
        fail_to_open_file
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
