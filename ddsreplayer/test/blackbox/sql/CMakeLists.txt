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

###################
# SqlFileReadTest #
###################

set(TEST_NAME SqlFileReadTest)

# Determine Fast DDS version
if ("${fastrtps_VERSION}" VERSION_LESS 2.13)
    set(DDS_TYPES_VERSION "v1")
else()
    set(DDS_TYPES_VERSION "v2")
endif()

file(
    GLOB_RECURSE TEST_SOURCES
    SqlFileReadTest.cpp
    "${PROJECT_SOURCE_DIR}/test/resources/dds/*.c*"
    "${PROJECT_SOURCE_DIR}/test/resources/types/*.c*"
    )

list(FILTER TEST_LIBRARY_SOURCES EXCLUDE REGEX "/main.cpp")

# Exclude types' files depending on the Fast DDS version
if ("${fastrtps_VERSION}" VERSION_LESS 2.13)
    list(FILTER TEST_SOURCES EXCLUDE REGEX "v2/")
else()
    list(FILTER TEST_SOURCES EXCLUDE REGEX "v1/")
endif()

set(TEST_LIST
        trivial
        data_to_check
        less_playback_rate
        more_playback_rate
        begin_time
        end_time
        start_replay_time_earlier
    )

set(TEST_NEEDED_SOURCES
        ../../resources/config/config_file.yaml
        ../../resources/config/config_file_less_hz.yaml
        ../../resources/config/config_file_more_hz.yaml
        ../../resources/config/config_file_begin_time.yaml
        ../../resources/config/config_file_end_time.yaml
        ../../resources/config/config_file_start_replay_time_earlier.yaml
        ../../resources/recordings/basic/helloworld.db
    )

set(TEST_EXTRA_HEADERS
    )

add_blackbox_executable(
    "${TEST_NAME}"
    "${TEST_SOURCES}"
    "${TEST_LIST}"
    "${TEST_NEEDED_SOURCES}"
    "${TEST_EXTRA_HEADERS}"
)

###########################
# SqlFileReadWithTypeTest #
###########################

set(TEST_NAME SqlFileReadWithTypeTest)

# Determine Fast DDS version
if ("${fastrtps_VERSION}" VERSION_LESS 2.13)
    set(DDS_TYPES_VERSION "v1")
else()
    set(DDS_TYPES_VERSION "v2")
endif()

file(
    GLOB_RECURSE TEST_SOURCES
    SqlFileReadWithTypeTest.cpp
    "${PROJECT_SOURCE_DIR}/test/resources/dds/*.c*"
    "${PROJECT_SOURCE_DIR}/test/resources/types/*.c*"
    )

list(FILTER TEST_LIBRARY_SOURCES EXCLUDE REGEX "/main.cpp")

# Exclude types' files depending on the Fast DDS version
if ("${fastrtps_VERSION}" VERSION_LESS 2.13)
    list(FILTER TEST_SOURCES EXCLUDE REGEX "v2/")
else()
    list(FILTER TEST_SOURCES EXCLUDE REGEX "v1/")
endif()

set(TEST_LIST
        trivial
        dds_data_to_check
        ros2_data_to_check
        dds_less_playback_rate
        ros2_less_playback_rate
        dds_more_playback_rate
        ros2_more_playback_rate
        begin_time
        end_time
        start_replay_time_earlier
    )

set(TEST_NEEDED_SOURCES
        ../../resources/config/config_file.yaml
        ../../resources/config/config_file_less_hz.yaml
        ../../resources/config/config_file_more_hz.yaml
        ../../resources/config/config_file_begin_time_with_types.yaml
        ../../resources/config/config_file_end_time_with_types.yaml
        ../../resources/config/config_file_start_replay_time_earlier_with_types.yaml
        ../../resources/recordings/ros2/ros2.db
        ../../resources/recordings/type/helloworld_with_type.db
    )

set(TEST_EXTRA_HEADERS
    )

add_blackbox_executable(
    "${TEST_NAME}"
    "${TEST_SOURCES}"
    "${TEST_LIST}"
    "${TEST_NEEDED_SOURCES}"
    "${TEST_EXTRA_HEADERS}"
)
