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

###############################################################################
# Find Externals
###############################################################################

# Find projects as REQUIRED from a list of arguments
#
# ARGUMENTS:
# - EXTERNAL_PROJECT_NAMES: List of names of external projects to find
#
# If ${${EXTERNAL_PROJECT}_MINIMUM_VERSION} is set for any of the project names passed as argument, it will
# be used in order to look for a minimum version for this package.
#

macro(find_external_projects EXTERNAL_PROJECT_NAMES)

    foreach(EXTERNAL_PROJECT ${EXTERNAL_PROJECT_NAMES})
        # Check if this project requires a minimum version, set as ${EXTERNAL_PROJECT}_MINIMUM_VERSION
        if (${${EXTERNAL_PROJECT}_MINIMUM_VERSION})

            find_package("${EXTERNAL_PROJECT}" "${${EXTERNAL_PROJECT}_MINIMUM_VERSION}" REQUIRED)
            message(
                STATUS
                "Package ${EXTERNAL_PROJECT} found higher than minimum version ${EXTERNAL_PROJECT}_MINIMUM_VERSION")

        else()

            find_package("${EXTERNAL_PROJECT}" REQUIRED)
            message(STATUS "Package ${EXTERNAL_PROJECT} found")

        endif()
    endforeach()

    # Finish macro
    message(STATUS "Finished finding dependency packages.")

endmacro()

# Find projects as REQUIRED from a list of arguments
#
# ARGUMENTS:
# - THIRDPARTIES_NAMES  : List of names of thirdparty projects to include headeronly
# - THIRDPARTY_PATH     : Path to thirdparty projects
macro(find_thirdparties_headeronly THIRDPARTIES_NAMES THIRDPARTY_PATH)

    foreach(THIRDPARTY_HEADERONLY_DEPENDENCY ${THIRDPARTIES_NAMES})
        include_directories(${THIRDPARTY_PATH}/${THIRDPARTY_HEADERONLY_DEPENDENCY})
        message(STATUS "Thirdparty HeaderOnly ${THIRDPARTY_HEADERONLY_DEPENDENCY} found")
    endforeach()

    # Finish macro
    message(STATUS "Finished finding thirdparties headeronly.")

endmacro()
