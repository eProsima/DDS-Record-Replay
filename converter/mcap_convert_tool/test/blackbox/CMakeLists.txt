# Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

set(TEST_NAME McapConvertTest)

set(TEST_SOURCES
    McapConvertTest.cpp
)

set(TEST_LIST
    help_argument
    version_argument
    missing_input
    default_output
    explicit_output
)

set(TEST_NEEDED_SOURCES
)

set(TEST_EXTRA_HEADERS
)

set(TEST_LIBRARY_SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/tool/McapToSqlConverter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/user_interface/arguments_configuration.cpp
)

add_blackbox_executable(
    "${TEST_NAME}"
    "${TEST_SOURCES}"
    "${TEST_LIST}"
    "${TEST_NEEDED_SOURCES}"
    "${TEST_EXTRA_HEADERS}"
    "${TEST_LIBRARY_SOURCES}"
)

target_compile_definitions(
    blackbox_${TEST_NAME}
    PRIVATE
        DDSREPLAYER_RECORDINGS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../../../ddsreplayer/test/resources/recordings"
)
