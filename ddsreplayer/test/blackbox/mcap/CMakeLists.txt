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

####################
# McapFileReadTest #
####################

set(TEST_NAME McapFileReadTest)

set(TEST_SOURCES
    McapFileReadTest.cpp
    ${PROJECT_SOURCE_DIR}/test/blackbox/mcap/dds/types/configuration/Configuration.hpp
    ${PROJECT_SOURCE_DIR}/test/blackbox/mcap/dds/types/configuration/ConfigurationPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/blackbox/mcap/dds/types/configuration/ConfigurationTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/blackbox/mcap/dds/ConfigurationSubscriber.cpp
)

set(TEST_LIST
        trivial
        dds_data_to_check
        less_playback_rate
        more_playback_rate
        begin_time
        end_time
        start_replay_time_earlier
    )

set(TEST_NEEDED_SOURCES
        resources/configuration.mcap
        resources/config_file_notype.yaml
        resources/config_file_less_hz_notype.yaml
        resources/config_file_more_hz_notype.yaml
        resources/config_file_begin_time_notype.yaml
        resources/config_file_end_time_notype.yaml
        resources/config_file_start_replay_time_earlier_notype.yaml
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

############################
# McapFileReadWithTypeTest #
############################

set(TEST_NAME McapFileReadWithTypeTest)

set(TEST_SOURCES
        McapFileReadWithTypeTest.cpp
        ${PROJECT_SOURCE_DIR}/test/blackbox/mcap/dds/types/configuration/Configuration.hpp
        ${PROJECT_SOURCE_DIR}/test/blackbox/mcap/dds/types/configuration/ConfigurationPubSubTypes.cxx
        ${PROJECT_SOURCE_DIR}/test/blackbox/mcap/dds/types/configuration/ConfigurationTypeObjectSupport.cxx
        ${PROJECT_SOURCE_DIR}/test/blackbox/mcap/dds/ConfigurationSubscriber.cpp
        ${PROJECT_SOURCE_DIR}/test/blackbox/mcap/dds/ConfigurationDynTypesSubscriber.cpp
    )

set(TEST_LIST
        trivial
        dds_data_to_check
        dds_less_playback_rate
        dds_more_playback_rate
        begin_time
        end_time
        start_replay_time_earlier
    )

set(TEST_NEEDED_SOURCES
        resources/configuration.mcap
        resources/config_file_withtypes.yaml
        resources/config_file_less_hz_withtypes.yaml
        resources/config_file_more_hz_withtypes.yaml
        resources/config_file_begin_time_with_types.yaml
        resources/config_file_end_time_with_types.yaml
        resources/config_file_start_replay_time_earlier_with_types.yaml
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
