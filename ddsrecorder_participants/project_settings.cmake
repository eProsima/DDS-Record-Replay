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

###############################################################################
# Set settings for project ddsrecorder_participants
###############################################################################

set(MODULE_NAME
    ddsrecorder_participants)

set(MODULE_SUMMARY
    "DDS Recorder Participants implementation.")

set(MODULE_FIND_PACKAGES
    yaml-cpp
    fastcdr
    fastrtps
    cpp_utils
    ddspipe_core
    ddspipe_participants)

if(WIN32)
    set(MODULE_FIND_PACKAGES
        ${MODULE_FIND_PACKAGES}
        lz4
        zstd)
endif()

set(MODULE_THIRDPARTY_HEADERONLY
    mcap)

set(fastrtps_MINIMUM_VERSION "2.8")

set(MODULE_DEPENDENCIES
    yaml-cpp
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    fastcdr
    fastrtps
    cpp_utils
    ddspipe_core
    ddspipe_participants
    $<IF:$<BOOL:${WIN32}>,lz4::lz4,lz4>
    $<IF:$<BOOL:${WIN32}>,$<IF:$<TARGET_EXISTS:zstd::libzstd_shared>,zstd::libzstd_shared,zstd::libzstd_static>,zstd>
    sqlite3)

set(MODULE_CPP_VERSION
    C++17)
