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

######################
# ResourceLimitsTest #
######################

set(TEST_NAME ResourceLimitsTest)

file(
    GLOB_RECURSE TEST_SOURCES
    ResourceLimitsTest.cpp
    "${PROJECT_SOURCE_DIR}/test/resources/types/hello_world/*.c*"
    )

set(TEST_LIBRARY_SOURCES
    )

set(TEST_LIST # TODO. danip
        mcap_max_file_size
    #sql_max_file_size
        mcap_max_size
    #sql_max_size
        mcap_file_rotation
    #sql_log_rotation
    )

set(TEST_NEEDED_SOURCES
    )

set(TEST_EXTRA_HEADERS
    )

add_blackbox_executable(
    "${TEST_NAME}"
    "${TEST_SOURCES}"
    "${TEST_LIST}"
    "${TEST_NEEDED_SOURCES}"
    "${TEST_EXTRA_HEADERS}"
    "${TEST_LIBRARY_SOURCES}"
)
