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

set(TEST_NAME YamlReaderDdsRecorderReplayerTest)

set(TEST_SOURCES
        YamlReaderDdsRecorderReplayerTest.cpp
    )

set(TEST_LIST
        recorder_domain_cli_overrides_yaml
        recorder_max_pending_samples_below_minus_one_throws
        recorder_sql_resource_limits_max_size_copies_to_max_file_size
        recorder_duplicate_manual_topic_overwrites_filter
        recorder_malformed_file_throws
        recorder_is_valid_defensive_checks
        replayer_domain_cli_overrides_yaml
        replayer_begin_time_after_end_time_throws
        replayer_malformed_file_throws
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

add_unittest_executable(
    "${TEST_NAME}"
    "${TEST_SOURCES}"
    "${TEST_LIST}"
    "${TEST_EXTRA_LIBRARIES}")
