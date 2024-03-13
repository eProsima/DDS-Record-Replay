# Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

set(TEST_NAME StdoutMonitorDdsRecorderStatusTest)

set(TEST_SOURCES
        StdoutMonitorDdsRecorderStatusTest.cpp
    )

file(
    GLOB_RECURSE LIBRARY_SOURCES
    # DdsRecorder Monitoring types
    "${PROJECT_SOURCE_DIR}/src/cpp/common/types/monitoring/*.c*"
    "${PROJECT_SOURCE_DIR}/include/common/types/monitoring/*.h*"
    "${PROJECT_SOURCE_DIR}/include/common/types/monitoring/*.ipp"
    # DdsRecorder Monitor
    "${PROJECT_SOURCE_DIR}/src/cpp/recorder/monitoring/*.c*"
    "${PROJECT_SOURCE_DIR}/include/recorder/monitoring/*.h*"
    )

# Exclude types' files depending on the Fast DDS version
if ("${fastrtps_VERSION}" VERSION_LESS 2.13)
    list(FILTER LIBRARY_SOURCES EXCLUDE REGEX "v2/")
else()
    list(FILTER LIBRARY_SOURCES EXCLUDE REGEX "v1/")
endif()

all_library_sources(
        "${TEST_SOURCES}"
        "${LIBRARY_SOURCES}"
    )

set(TEST_LIST
        type_mismatch
        qos_mismatch
        mcap_file_creation_failure
        disk_full
    )

set(TEST_EXTRA_LIBRARIES
        fastcdr
        fastrtps
        cpp_utils
        ddspipe_core
        ddspipe_participants
    )

add_unittest_executable(
        "${TEST_NAME}"
        "${TEST_SOURCES}"
        "${TEST_LIST}"
        "${TEST_EXTRA_LIBRARIES}"
    )
