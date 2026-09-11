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

set(TEST_NAME FileTrackerTest)

set(TEST_SOURCES
        FileTrackerTest.cpp
    )

set(TEST_LIST
        existing_files_not_tracked_by_default
        existing_files_tracked
        unrelated_files_not_tracked
        timestamped_existing_files_tracked
        oldest_existing_file_removed
        existing_files_exceeding_max_size_removed
    )

set(TEST_EXTRA_LIBRARIES
        cpp_utils
        ddspipe_core
        ddspipe_participants
        ddsrecorder_participants
    )

add_unittest_executable(
        "${TEST_NAME}"
        "${TEST_SOURCES}"
        "${TEST_LIST}"
        "${TEST_EXTRA_LIBRARIES}"
    )
