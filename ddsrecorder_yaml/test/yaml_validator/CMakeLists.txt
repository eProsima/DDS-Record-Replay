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

###########################################
# Dds Record & Replay Yaml Validator Test #
###########################################

set(TEST_NAME YamlValidatorDdsRecorderReplayerTest)


set(TEST_SOURCES
        YamlValidatorDdsRecorderReplayerTest.cpp
    )

set(TEST_LIST
        recorder_direct_validation_passed
        recorder_direct_validation_failed
        replayer_direct_validation_passed
        replayer_direct_validation_failed
        recorder_reader_validation_passed
        recorder_reader_validation_failed
        replayer_reader_validation_passed
        replayer_reader_validation_failed
    )

set(TEST_EXTRA_LIBRARIES
        yaml-cpp
        fastcdr
        fastdds
        cpp_utils
        ddspipe_core
        ddspipe_participants
        ddspipe_yaml
        ddsrecorder_yaml
    )

configure_file(
    "${PROJECT_SOURCE_DIR}/../resources/configurations/ddsrecorder_config_schema.json"
    "${CMAKE_CURRENT_BINARY_DIR}/ddsrecorder_config_schema.json"
    COPYONLY
)
configure_file(
    "${PROJECT_SOURCE_DIR}/../resources/configurations/ddsreplayer_config_schema.json"
    "${CMAKE_CURRENT_BINARY_DIR}/ddsreplayer_config_schema.json"
    COPYONLY
)

file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/valid_config_files_recorder/")
file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/invalid_config_files_recorder/")
file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/valid_config_files_replayer/")
file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/invalid_config_files_replayer/")


file(GLOB_RECURSE TEST_NEEDED_SOURCES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "*.json"
    "valid_config_files_recorder/*.yaml"
    "invalid_config_files_recorder/*.yaml"
    "valid_config_files_replayer/*.yaml"
    "invalid_config_files_replayer/*.yaml"
)

file(GLOB EXAMPLE_YAMLS_RECORDER
    "${PROJECT_SOURCE_DIR}/../resources/configurations/recorder/*.yaml"
)
file(COPY ${EXAMPLE_YAMLS_RECORDER}
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/valid_config_files_recorder/"
)

file(GLOB EXAMPLE_YAMLS_REPLAYER
    "${PROJECT_SOURCE_DIR}/../resources/configurations/replayer/*.yaml"
)
file(COPY ${EXAMPLE_YAMLS_REPLAYER}
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/valid_config_files_replayer/"
)

add_unittest_executable(
    "${TEST_NAME}"
    "${TEST_SOURCES}"
    "${TEST_LIST}"
    "${TEST_EXTRA_LIBRARIES}"
    "${TEST_NEEDED_SOURCES}"
)

