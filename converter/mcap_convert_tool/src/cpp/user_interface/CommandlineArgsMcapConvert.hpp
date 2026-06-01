// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#pragma once

#include <string>

#include <ddsrecorder_yaml/replayer/CommandlineArgsReplayer.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace converter {

struct CommandlineArgsMcapConvert : public yaml::CommandlineArgsReplayer
{
    std::string sql_output{""};

    bool is_valid(
            utils::Formatter& error_msg) const noexcept override
    {
        if (input_file.empty())
        {
            error_msg << "Option '-i' / '--input-file' is required.";
            return false;
        }

        return yaml::CommandlineArgsReplayer::is_valid(error_msg);
    }
};

} /* namespace converter */
} /* namespace ddsrecorder */
} /* namespace eprosima */
