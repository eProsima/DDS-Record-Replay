// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SpecificEndpointQoS.hpp
 */

#ifndef _DDSRECORDER_TYPES_ENDPOINT_SpecificEndpointQoS_HPP_
#define _DDSRECORDER_TYPES_ENDPOINT_SpecificEndpointQoS_HPP_

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/common/InstanceHandle.h>
#include <fastdds/rtps/common/Types.h>

#include <ddsrecorder/library/library_dll.h>

namespace eprosima {
namespace ddsrecorder {
namespace core {
namespace types {

//! Partitions
using PartitionQosPolicy = eprosima::fastdds::dds::PartitionQosPolicy;

//! Ownership Strength
using OwnershipStrengthQosPolicy = eprosima::fastdds::dds::OwnershipStrengthQosPolicy;

/**
 * Collection of QoS of an Endpoint
 *
 * @todo Divide this in Common, Reader and Writer QoS
 */
struct DDSRECORDER_DllAPI SpecificEndpointQoS
{
    /////////////////////////
    // CONSTRUCTORS
    /////////////////////////

    //! Default SpecificEndpointQoS with reader less restrictive parameters
    SpecificEndpointQoS() = default;

    /////////////////////////
    // OPERATORS
    /////////////////////////

    //! Minor comparison operator
    bool operator < (
            const SpecificEndpointQoS& other) const noexcept;

    //! Equality operator
    bool operator == (
            const SpecificEndpointQoS& other) const noexcept;

    /////////////////////////
    // VARIABLES
    /////////////////////////

    //! Partitions of the data
    PartitionQosPolicy partitions{};

    //! Ownership strength of the data
    OwnershipStrengthQosPolicy ownership_strength{};
};

/**
 * @brief \c PartitionQosPolicy to stream serialization
 */
DDSRECORDER_DllAPI std::ostream& operator <<(
        std::ostream& os,
        const PartitionQosPolicy& qos);

/**
 * @brief \c OwnershipStrengthQosPolicy to stream serialization
 */
DDSRECORDER_DllAPI std::ostream& operator <<(
        std::ostream& os,
        const OwnershipStrengthQosPolicy& qos);

/**
 * @brief \c SpecificEndpointQoS to stream serialization
 */
DDSRECORDER_DllAPI std::ostream& operator <<(
        std::ostream& os,
        const SpecificEndpointQoS& qos);

} /* namespace types */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */

#endif /* _DDSRECORDER_TYPES_ENDPOINT_SpecificEndpointQoS_HPP_ */
