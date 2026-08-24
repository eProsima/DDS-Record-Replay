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

####################################################################################################
# Add tests
####################################################################################################

message(STATUS "Building the eProsima DDS Record & Replay documentation tests.")

find_package(Python 3.7 REQUIRED COMPONENTS Interpreter)

# Check docs style
find_program(DOC8 doc8 DOC "path to doc8 executable")
if(DOC8)
    message(STATUS "Found doc8: ${DOC8}")
    add_test(
        NAME documentation.doc8
        COMMAND ${DOC8} --ignore D001 ${PROJECT_SOURCE_DOCS_DIR})
else()
    message(FATAL_ERROR "Could not find doc8")
endif()

# Check docs spelling
add_test(NAME documentation.spell_check
    COMMAND
    ${SPHINX_EXECUTABLE} -W --keep-going
    -b spelling
    -d "${PROJECT_BINARY_DOCS_DIR}/doctrees"
    ${PROJECT_SOURCE_DOCS_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/spelling)

# Build HTML with missing references and warnings treated as errors.
add_test(NAME documentation.html
    COMMAND
    ${SPHINX_EXECUTABLE} -nW --keep-going
    -b html
    -d "${PROJECT_BINARY_DOCS_DIR}/doctrees-html"
    ${PROJECT_SOURCE_DOCS_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/html)

# Validate the exact YAML files included by the user documentation.
add_test(NAME documentation.yaml_examples
    COMMAND
    ${Python_EXECUTABLE} ${PROJECT_SOURCE_DOCS_DIR}/test/validate_examples.py)
