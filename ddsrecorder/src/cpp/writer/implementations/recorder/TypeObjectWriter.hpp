// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TypeObjectWriter.hpp
 */

#pragma once

#include <condition_variable>
#include <mutex>

#include <cpp_utils/time/time_utils.hpp>

#include <writer/implementations/auxiliar/BaseWriter.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {

/**
 * Writer implementation that allows to Read custom data produced internally.
 */
class TypeObjectWriter : public BaseWriter
{
public:

    //! Use parent constructors
    using BaseWriter::BaseWriter;

protected:

    /**
     * @brief Write specific method
     *
     * @param data : data to simulate publication
     * @return RETCODE_OK always
     */
    utils::ReturnCode write_(
            std::unique_ptr<types::DataReceived>& data) noexcept override;
};

} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
