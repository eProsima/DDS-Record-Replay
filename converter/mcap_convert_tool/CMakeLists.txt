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

###############################################################################
# CMake build rules for MCAP Convert Tool
###############################################################################
cmake_minimum_required(VERSION 3.5)

enable_language(CXX)

find_package(cmake_utils REQUIRED)

configure_project()

project(
    ${MODULE_NAME}
    VERSION
        ${MODULE_VERSION}
    DESCRIPTION
        ${MODULE_DESCRIPTION}
    LANGUAGES
        CXX
)

configure_project_cpp()

compile_tool(
    "${PROJECT_SOURCE_DIR}/src/cpp"
)

compile_test_tool(
    "${PROJECT_SOURCE_DIR}/test"
)

eprosima_packaging()
