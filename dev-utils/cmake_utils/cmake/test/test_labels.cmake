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

# In order to centralize all the labels for all the tests, these macros has been implemented
# A variable ${PROJECT_NAME}_TEST_LABELS is set with all labels accepted (labels could be repeated)
# A variable ${PROJECT_NAME}_LABEL_TEST_${N} is set for each N and contains the name of the file
#  where to read the tests that must have such label set in previous variable.
# The value N in ${PROJECT_NAME}_TEST_LABELS is the label to set in tests listed in file
#  ${PROJECT_NAME}_LABEL_TEST_${N}

macro(set_test_label_file FILE_NAME LABEL)

    message(STATUS "Set ${LABEL} label to tests in ${FILE_NAME} <${${PROJECT_NAME}_TEST_LABELS}>")

    # First check if file exists
    if(NOT EXISTS ${FILE_NAME})
        message(WARNING "File ${FILE_NAME} does not exist, impossible to set label ${LABEL}")
        return()
    else()
        # Read file
        file(STRINGS ${FILE_NAME} TEST_LIST)
    endif()

    list(LENGTH INTERNAL_TEST_LABELS LABELS_SIZE)

    message(STATUS "set_test_label_file <${INTERNAL_TEST_LABELS}> <${LABELS_SIZE}>")

    # Append the new label to the list
    list(APPEND INTERNAL_TEST_LABELS "${LABEL}")
    set(${PROJECT_NAME}_TEST_LABELS "${INTERNAL_TEST_LABELS}")
    set(${PROJECT_NAME}_TEST_LABELS "${INTERNAL_TEST_LABELS}" PARENT_SCOPE)

    # Store the file content in a variable with the label index in its name
    # NOTE: for some reason this must be set twice, once with parent scope and one without
    set(${PROJECT_NAME}_LABEL_TEST_${LABELS_SIZE} "${TEST_LIST}")
    set(${PROJECT_NAME}_LABEL_TEST_${LABELS_SIZE} "${TEST_LIST}" PARENT_SCOPE)

endmacro()

macro(check_and_add_tests_label TEST_NAME TEST_LABEL_LIST LABEL)

    if(${TEST_NAME} IN_LIST "${TEST_LABEL_LIST}")
        message(STATUS "Setting label ${LABEL} to test ${TEST_NAME}")
        # NOTE: set_property override other labels, it must append new label
        get_property(CURRENT_LABELS TEST ${TEST_NAME} PROPERTY LABELS)
        list(APPEND CURRENT_LABELS ${LABEL})
        set_property(TEST ${TEST_NAME} PROPERTY LABELS ${CURRENT_LABELS})
    endif()

endmacro()

macro(set_test_labels TEST_NAME)

    set (CURRENT_INDEX 0)
    foreach(TEST_LABEL ${${PROJECT_NAME}_TEST_LABELS})

        check_and_add_tests_label(
            "${TEST_NAME}"
            ${PROJECT_NAME}_LABEL_TEST_${CURRENT_INDEX}
            "${TEST_LABEL}"
        )

        MATH(EXPR CURRENT_INDEX "${CURRENT_INDEX} + 1")

    endforeach()

endmacro()
