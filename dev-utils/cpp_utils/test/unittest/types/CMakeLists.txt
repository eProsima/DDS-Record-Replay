# Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
# FUZZY TEST
############################

set(TEST_NAME fuzzyTest)

set(TEST_SOURCES
        fuzzyTest.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/Formatter.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/exception/Exception.cpp
    )

set(TEST_LIST
        default_construct
        construct
        assign_operator
        arrow_operator
    )

set(TEST_EXTRA_LIBRARIES
    )

set(TEST_NEEDED_SOURCES
    )

add_unittest_executable(
        "${TEST_NAME}"
        "${TEST_SOURCES}"
        "${TEST_LIST}"
        "${TEST_EXTRA_LIBRARIES}"
        "${TEST_NEEDED_SOURCES}"
    )

############################
# SINGLETON TEST
############################

set(TEST_NAME singletonTest)

set(TEST_SOURCES
        singletonTest.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/wait/BooleanWaitHandler.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/utils.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/time/time_utils.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/Formatter.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/exception/Exception.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/math/math_extension.cpp
    )

set(TEST_LIST
        trivial_get_instance
        different_index_class
    )

set(TEST_EXTRA_LIBRARIES
        fastcdr
        fastrtps
        $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )

set(TEST_NEEDED_SOURCES
    )

add_unittest_executable(
        "${TEST_NAME}"
        "${TEST_SOURCES}"
        "${TEST_LIST}"
        "${TEST_EXTRA_LIBRARIES}"
        "${TEST_NEEDED_SOURCES}"
    )


############################
# SINGLETON ORDER TEST
############################

set(TEST_NAME singletonOrderTest)

set(TEST_SOURCES
        singletonOrderTest.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/wait/BooleanWaitHandler.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/utils.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/time/time_utils.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/Formatter.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/exception/Exception.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/math/math_extension.cpp
    )

set(TEST_LIST
        correct_construction_order
        correct_destruction_order
    )

set(TEST_EXTRA_LIBRARIES
        fastcdr
        fastrtps
        $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    )

set(TEST_NEEDED_SOURCES
    )

add_unittest_executable(
        "${TEST_NAME}"
        "${TEST_SOURCES}"
        "${TEST_LIST}"
        "${TEST_EXTRA_LIBRARIES}"
        "${TEST_NEEDED_SOURCES}"
    )
