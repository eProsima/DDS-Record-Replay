# Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

# prevent multiple inclusion
if(DEFINED _cmake_utils_INCLUDED)
  message(FATAL_ERROR "cmake_utils/cmake/core/load_all.cmake included multiple times")
endif()
set(_cmake_utils_INCLUDED TRUE)

if(NOT DEFINED cmake_utils_DIR)
  message(FATAL_ERROR "cmake_utils_DIR is not set")
endif()

######
# Include functions, methods and modules
# Get every cmake that must be included
file(GLOB_RECURSE
        CMAKE_INCLUSIONS
        "${cmake_utils_DIR}/**/*.cmake"
    )

# Include functions / macros
foreach(FILENAME ${CMAKE_INCLUSIONS})
    include(${FILENAME})
endforeach()

# Add modules folder so FindPackages file are in CMAKE_MODULE_PATH and could be accessed
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${cmake_utils_DATA_DIR}/cmake_utils/modules)

######
# Set some useful variables

# Path to the templates for Config.cmake files
set(cmake_utils_CONFIG_TEMPLATES_PATH "${cmake_utils_DATA_DIR}/cmake_utils/templates/cmake/packaging")
set(cmake_utils_LIBRARY_HEADERS_TEMPLATES_PATH "${cmake_utils_DATA_DIR}/cmake_utils/templates/cpp/library")
