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

####################
# RecordReplayTest #
####################

# The recorder is built from its sources, as it is a tool and not a library
set(DDSRECORDER_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../ddsrecorder/src/cpp")

set(TEST_NAME RecordReplayTest)

file(
    GLOB_RECURSE TEST_SOURCES
    RecordReplayTest.cpp
    "${PROJECT_SOURCE_DIR}/test/resources/dds/*.c*"
    "${PROJECT_SOURCE_DIR}/test/resources/types/*.c*"
    "${DDSRECORDER_SOURCE_DIR}/tool/DdsRecorder.cpp"
    )

set(TEST_LIST
        record_without_idl_types
        record_with_idl_types
        replay_no_idl_types
        replay_idl_types
        replay_types_recorded_from_idl
        replay_missing_idl_file
        replay_missing_and_valid_idl_files
    )

set(TEST_NEEDED_SOURCES
        ../../resources/config/type/config_file.yaml
        ../../resources/types/configuration/Configuration.idl
    )

set(TEST_EXTRA_HEADERS
        ${DDSRECORDER_SOURCE_DIR}
    )

add_blackbox_executable(
    "${TEST_NAME}"
    "${TEST_SOURCES}"
    "${TEST_LIST}"
    "${TEST_NEEDED_SOURCES}"
    "${TEST_EXTRA_HEADERS}"
)

# Each test runs in its own process, so a replay never shares the type registry of the recording it replays
set_tests_properties(${TEST_NAME}.record_without_idl_types PROPERTIES FIXTURES_SETUP recording_without_idl_types)
set_tests_properties(${TEST_NAME}.record_with_idl_types PROPERTIES FIXTURES_SETUP recording_with_idl_types)

set_tests_properties(
    ${TEST_NAME}.replay_no_idl_types
    ${TEST_NAME}.replay_idl_types
    ${TEST_NAME}.replay_missing_idl_file
    ${TEST_NAME}.replay_missing_and_valid_idl_files
    PROPERTIES FIXTURES_REQUIRED recording_without_idl_types)

set_tests_properties(${TEST_NAME}.replay_types_recorded_from_idl PROPERTIES FIXTURES_REQUIRED recording_with_idl_types)
