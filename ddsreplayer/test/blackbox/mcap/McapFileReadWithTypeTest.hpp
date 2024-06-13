// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>

#include "McapFileReadTest.hpp"

/**
 * @brief Test class for Mcap file read tests that publish the type.
 *
 * Include the tests that are exclusive to publishing the type in MCAP files.
 */
class McapFileReadWithTypeTest : public McapFileReadTest
{
protected:

    const bool publish_type_ = true;
    const std::string input_file_type_{"../../resources/recordings/type/helloworld_with_type.mcap"};
    const std::string input_file_ros2_{"../../resources/recordings/ros2/ros2.mcap"};
};
