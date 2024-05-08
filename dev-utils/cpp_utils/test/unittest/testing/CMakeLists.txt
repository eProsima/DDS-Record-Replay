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

set(TEST_NAME LogCheckerTest)

set(TEST_SOURCES
        LogCheckerTest.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/event/logging/LogConsumerConnection.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/event/logging/LogEventHandler.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/event/logging/LogSevereEventHandler.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/testing/LogChecker.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/exception/Exception.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/utils.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/Formatter.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/math/math_extension.cpp
    )

set(TEST_LIST
        trivial_create
        non_logs_default
        non_logs
        minimum_logs
        maximum_logs
        exact_logs
        severe_logs_info
        severe_logs_warning
        severe_logs_error
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
    "${TEST_EXTRA_LIBRARIES}")
