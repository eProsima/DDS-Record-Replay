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
 * @file Endpoint.hpp
 */

#ifndef _DDSRECORDER_TYPES_ENDPOINT_ENDPOINT_HPP_
#define _DDSRECORDER_TYPES_ENDPOINT_ENDPOINT_HPP_

#include <cpp_utils/macros/custom_enumeration.hpp>

#include <ddsrecorder/library/library_dll.h>
#include <ddsrecorder/types/dds/Guid.hpp>
#include <ddsrecorder/types/participant/ParticipantId.hpp>
#include <ddsrecorder/types/topic/rpc/RPCTopic.hpp>
#include <ddsrecorder/types/topic/dds/DdsTopic.hpp>
#include <ddsrecorder/types/dds/SpecificEndpointQoS.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {
namespace types {

using EndpointKindType = unsigned int;

//! Possible kinds of the endpoint
ENUMERATION_BUILDER(
    EndpointKind,
    invalid,
    writer,
    reader
    );

/**
 * Data collection to describe an Endpoint
 *
 * This class works as a data storage struct with the information of a discovered Endpoint
 */
class Endpoint
{
public:

    //! Default Endpoint that returns an invalid one
    DDSRECORDER_DllAPI Endpoint() noexcept;

    /**
     * Constructor with Endpoint information
     */
    DDSRECORDER_DllAPI Endpoint(
            const EndpointKind& kind,
            const Guid& guid,
            const DdsTopic& topic,
            const ParticipantId& discoverer_participant_id = ParticipantId(),
            const SpecificEndpointQoS& specific_qos = SpecificEndpointQoS()) noexcept;

    //! Endpoint kind getter
    DDSRECORDER_DllAPI EndpointKind kind() const noexcept;

    //! Endpoint kind setter
    DDSRECORDER_DllAPI void kind(
            const EndpointKind& kind) noexcept;

    //! Guid getter
    DDSRECORDER_DllAPI Guid guid() const noexcept;

    //! TopicQoS getter
    DDSRECORDER_DllAPI TopicQoS topic_qos() const noexcept;

    //! SpecificQoS getter
    DDSRECORDER_DllAPI SpecificEndpointQoS specific_qos() const noexcept;

    //! SpecificQoS setter
    DDSRECORDER_DllAPI void specific_qos(
            const SpecificEndpointQoS& specific_qos) noexcept;

    //! Topic getter
    DDSRECORDER_DllAPI DdsTopic topic() const noexcept;

    //! Id of participant who discovered this endpoint
    DDSRECORDER_DllAPI ParticipantId discoverer_participant_id() const noexcept;

    //! Whether the endpoint referenced is currently active
    DDSRECORDER_DllAPI bool active() const noexcept;

    //! Set active status of the Endpoint
    DDSRECORDER_DllAPI void active(
            bool status) noexcept;

    //! Whether the endpoint referenced is valid
    DDSRECORDER_DllAPI bool is_valid() const noexcept;

    /********************
    * SPECIFIC GETTERS *
    ********************/

    //! Whether the endpoint is a writer
    DDSRECORDER_DllAPI bool is_writer() const noexcept;

    //! Whether the endpoint is a reader
    DDSRECORDER_DllAPI bool is_reader() const noexcept;

    //! Whether the endpoint belongs to a RPC server (i.e. is request reader or reply writer)
    DDSRECORDER_DllAPI bool is_server_endpoint() const noexcept;

    //! Copy operator
    DDSRECORDER_DllAPI Endpoint& operator =(
            const Endpoint& other) noexcept;

    //! Equality operator (does not take \c active_ into consideration)
    DDSRECORDER_DllAPI bool operator ==(
            const Endpoint& other) const noexcept;

protected:

    //! Kind of the endpoint
    EndpointKind kind_;

    //! Unique id of the endpoint
    Guid guid_;

    //! Topic that this endpoint belongs to
    DdsTopic topic_;

    //! Whether the endpoint is currently active
    bool active_;

    //! Id of participant who discovered this endpoint
    ParticipantId discoverer_participant_id_;

    //! Specific QoS of the entity
    SpecificEndpointQoS specific_qos_;

    // Allow operator << to use private variables
    DDSRECORDER_DllAPI friend std::ostream& operator <<(
            std::ostream&,
            const Endpoint&);
};

/**
 * @brief \c Endpoint to stream serialization
 */
DDSRECORDER_DllAPI std::ostream& operator <<(
        std::ostream& os,
        const Endpoint& endpoint);

} /* namespace types */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */

#endif /* _DDSRECORDER_TYPES_ENDPOINT_ENDPOINT_HPP_ */
