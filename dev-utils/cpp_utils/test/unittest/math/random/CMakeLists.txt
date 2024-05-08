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

##################
# RANDOM MANAGER TESTS

set(TEST_NAME randomTest)

set(TEST_SOURCES
        randomTest.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/math/random/RandomManager.cpp
    )

set(TEST_LIST
        trivial_pure
        trivial_sequence
        trivial_sequence_with_seed
        trivial_seeded
        get_random_sequence_number
        get_pure_random_number
        get_seed_random_number
        set_initial_seed
    )

set(TEST_EXTRA_LIBRARIES
    )

set(TEST_NEEDED_SOURCES
    )

add_unittest_executable(
        "${TEST_NAME}"
        "${TEST_SOURCES}"
        "${TEST_LIST}"
        "${TEST_EXTRA_LIBRARIES}"
        "${TEST_NEEDED_SOURCES}"
    )

##################
# THREAD SAFE RANDOM MANAGER TESTS

set(TEST_NAME tsaferandomTest)

set(TEST_SOURCES
        tsaferandomTest.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/math/random/TSafeRandomManager.cpp
    )

set(TEST_LIST
        trivial_pure
        trivial_sequence
        trivial_sequence_with_seed
        trivial_seeded
        get_random_sequence_number
        get_pure_random_number
        get_seed_random_number
        set_initial_seed
        sequential_access
    )

set(TEST_EXTRA_LIBRARIES
        fastrtps
    )

set(TEST_NEEDED_SOURCES
    )

add_unittest_executable(
        "${TEST_NAME}"
        "${TEST_SOURCES}"
        "${TEST_LIST}"
        "${TEST_EXTRA_LIBRARIES}"
        "${TEST_NEEDED_SOURCES}"
    )
