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
# Set settings for project ddsrecorder_tool
###############################################################################

set(MODULE_NAME
    ddsrecorder_tool)

set(MODULE_SUMMARY
    "C++ application to run a DDS Recorder from a YAML configuration file.")

set(MODULE_FIND_PACKAGES
    yaml-cpp
    fastcdr
    fastdds
    cpp_utils
    ddspipe_core
    ddspipe_participants
    ddspipe_yaml
    ddsrecorder_participants
    ddsrecorder_yaml)

if(WIN32)
    set(MODULE_FIND_PACKAGES
        ${MODULE_FIND_PACKAGES}
        lz4
        zstd)
endif()

set(MODULE_DEPENDENCIES
    yaml-cpp
    fastcdr
    fastdds
    cpp_utils
    ddspipe_core
    ddspipe_participants
    ddspipe_yaml
    ddsrecorder_participants
    ddsrecorder_yaml
    $<IF:$<BOOL:${WIN32}>,lz4::lz4,lz4>
    $<IF:$<BOOL:${WIN32}>,$<IF:$<TARGET_EXISTS:zstd::libzstd_shared>,zstd::libzstd_shared,zstd::libzstd_static>,zstd>)

set(MODULE_THIRDPARTY_HEADERONLY
    filewatch
    mcap
    nlohmann-json
    optionparser
    sqlite # Not header-only though, need to include the sources
    )

set(MODULE_THIRDPARTY_PATH
    "../thirdparty")

set(MODULE_LICENSE_FILE_PATH
    "../LICENSE")

set(MODULE_VERSION_FILE_PATH
    "../VERSION")

set(MODULE_TARGET_NAME
    "ddsrecorder")

set(MODULE_CPP_VERSION
    C++17)
