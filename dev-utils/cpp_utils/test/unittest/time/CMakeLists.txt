# Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

############################
# TIME UTILS TEST
############################

set(TEST_NAME time_utils_test)

set(TEST_SOURCES
        time_utils_test.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/utils.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/math/math_extension.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/time/time_utils.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/time/Timer.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/exception/Exception.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/Formatter.cpp
    )

set(TEST_LIST
        timestamp_to_string_to_timestamp
        timestamp_to_string_to_timestamp_local
        timestamp_to_string_format
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

############################
# TIMER TEST
############################

set(TEST_NAME TimerTest)

set(TEST_SOURCES
        TimerTest.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/time/time_utils.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/time/Timer.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/exception/Exception.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/Formatter.cpp
    )

set(TEST_LIST
        elapsed
        elapsed_ms
        reset
    )

set(TEST_EXTRA_LIBRARIES
        fastcdr
        fastrtps
    )

add_unittest_executable(
        "${TEST_NAME}"
        "${TEST_SOURCES}"
        "${TEST_LIST}"
        "${TEST_EXTRA_LIBRARIES}"
    )

############################
# PAUSABLE TIMER TEST
############################

set(TEST_NAME PausableTimerTest)

set(TEST_SOURCES
        PausableTimerTest.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/time/time_utils.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/time/Timer.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/time/PausableTimer.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/exception/Exception.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/Formatter.cpp
    )

set(TEST_LIST
        create
        repeat_play_pause
        play_and_pause
    )

set(TEST_EXTRA_LIBRARIES
        fastcdr
        fastrtps
    )

add_unittest_executable(
        "${TEST_NAME}"
        "${TEST_SOURCES}"
        "${TEST_LIST}"
        "${TEST_EXTRA_LIBRARIES}"
    )
