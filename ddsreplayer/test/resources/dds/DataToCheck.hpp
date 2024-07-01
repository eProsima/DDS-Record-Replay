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

/**
 * @file DataToCheck.hpp
 */

#pragma once

#include <string>


struct DataToCheck
{
    unsigned int n_received_msgs;
    std::string type_msg;
    std::string message_msg;
    int min_index_msg;
    int max_index_msg;
    double mean_ms_between_msgs;
    double cummulated_ms_between_msgs;
};
