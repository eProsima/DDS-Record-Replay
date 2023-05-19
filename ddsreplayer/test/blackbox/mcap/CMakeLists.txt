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

####################
# McapFileReadTest #
####################

set(TEST_NAME McapFileReadTest)

set(TEST_SOURCES
        McapFileReadTest.cpp
        ${PROJECT_SOURCE_DIR}/test/blackbox/mcap/dds/types/hello_world/HelloWorld.cxx
        ${PROJECT_SOURCE_DIR}/test/blackbox/mcap/dds/types/hello_world/HelloWorldPubSubTypes.cxx
        ${PROJECT_SOURCE_DIR}/test/blackbox/mcap/dds/HelloWorldSubscriber.cpp
    )

set(TEST_LIST
        trivial
        n_msgs
        msg_type
        msg_message
        msg_min_index
        msg_max_index
        msg_hz
    )

set(TEST_NEEDED_SOURCES
        resources/helloworld_file.mcap
        resources/config_file.yaml
    )

set(TEST_EXTRA_HEADERS
    )

add_blackbox_executable(
    "${TEST_NAME}"
    "${TEST_SOURCES}"
    "${TEST_LIST}"
    "${TEST_NEEDED_SOURCES}"
    "${TEST_EXTRA_HEADERS}"
)
