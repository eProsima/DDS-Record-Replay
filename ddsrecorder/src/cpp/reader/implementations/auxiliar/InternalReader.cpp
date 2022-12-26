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
 * @file InternalReader.cpp
 */

#include <reader/implementations/auxiliar/InternalReader.hpp>
#include <cpp_utils/Log.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {

using namespace eprosima::ddsrecorder::core::types;

void InternalReader::simulate_data_reception(
        std::unique_ptr<types::DataReceived>&& data) noexcept
{
    std::lock_guard<DataReceviedType> lock(data_to_send_);

    // Even if disabled, the data will be stored
    data_to_send_.push(std::move(data));

    // Call on data available callback
    // TODO(recorder) check that track is already available, otherwise check what happens
    on_data_available_();
}

utils::ReturnCode InternalReader::take_(
        std::unique_ptr<DataReceived>& data) noexcept
{
    std::lock_guard<DataReceviedType> lock(data_to_send_);

    // Enable check is done in BaseReader

    // There is no data pending sent
    if (data_to_send_.empty())
    {
        return utils::ReturnCode::RETCODE_NO_DATA;
    }

    // Get next data received
    std::unique_ptr<types::DataReceived> next_data_to_send = std::move(data_to_send_.front());
    data_to_send_.pop();

    // Copy properties
    data->properties = next_data_to_send->properties;
    // "Copy" (no copy because of payload pool) payload
    eprosima::fastrtps::rtps::IPayloadPool* payload_owner = payload_pool_.get();
    payload_pool_->get_payload(
        next_data_to_send->payload,
        payload_owner,
        data->payload
        );

    payload_pool_->release_payload(next_data_to_send->payload);

    return utils::ReturnCode::RETCODE_OK;
}

} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */
