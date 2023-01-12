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
 * @file CommonParticipant.hpp
 */

#ifndef __SRC_DDSRECORDER_PARTICIPANT_IMPLEMENTATIONS_RTPS_COMMONPARTICIPANT_HPP_
#define __SRC_DDSRECORDER_PARTICIPANT_IMPLEMENTATIONS_RTPS_COMMONPARTICIPANT_HPP_

#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastdds/rtps/reader/ReaderDiscoveryInfo.h>
#include <fastdds/rtps/rtps_fwd.h>
#include <fastdds/rtps/writer/WriterDiscoveryInfo.h>
#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/rtps/participant/RTPSParticipantListener.h>

#include <ddsrecorder/configuration/participant/ParticipantConfiguration.hpp>
#include <ddsrecorder/types/dds/DomainId.hpp>

#include <participant/implementations/auxiliar/BaseParticipant.hpp>
#include <reader/implementations/auxiliar/InternalReader.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace core {
namespace rtps {

/**
 * Abstract generic class for a RTPS Participant wrapper.
 *
 * Concrete classes that inherit from this would only need to specialize specific methods related with the
 * qos and attributes.
 *
 * @warning This object is not RAII and must be initialized before used.
 */
class CommonParticipant
    : public BaseParticipant
    , public fastrtps::rtps::RTPSParticipantListener
{
public:

    /**
     * @brief Construct a CommonParticipant
     */
    CommonParticipant(
            std::shared_ptr<configuration::ParticipantConfiguration> participant_configuration,
            std::shared_ptr<PayloadPool> payload_pool,
            std::shared_ptr<DiscoveryDatabase> discovery_database,
            const types::DomainId& domain_id,
            const fastrtps::rtps::RTPSParticipantAttributes& participant_attributes);

    //! Remove internal RTPS Participant
    virtual ~CommonParticipant();

    /**
     * @brief Create the internal RTPS Participant using the attributes given.
     *
     * @attention this method should be called right after constructor to create enable internal entities.
     * This is required as this object is a Listener that could be called before finishing construction.
     * Other alternatives have been studied but none have really fit for this case.
     *
     * @throw InitializationException if RTPS Participant creation fails
     *
     * @warning this method is not thread safe.
     * @pre this method can only be called once.
     */
    void init();

    /**
     * @brief Override method from \c RTPSParticipantListener .
     *
     * This method only is for debugging purposes.
     */
    virtual void onParticipantDiscovery(
            fastrtps::rtps::RTPSParticipant* participant,
            fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;

    /**
     * @brief Override method from \c RTPSParticipantListener .
     *
     * This method adds to database the endpoint discovered or modified.
     */
    virtual void onReaderDiscovery(
            fastrtps::rtps::RTPSParticipant* participant,
            fastrtps::rtps::ReaderDiscoveryInfo&& info) override;

    /**
     * @brief Override method from \c RTPSParticipantListener .
     *
     * This method adds to database the endpoint discovered or modified.
     */
    virtual void onWriterDiscovery(
            fastrtps::rtps::RTPSParticipant* participant,
            fastrtps::rtps::WriterDiscoveryInfo&& info) override;

    virtual void on_type_discovery(
            fastrtps::rtps::RTPSParticipant* participant,
            const fastrtps::rtps::SampleIdentity& request_sample_id,
            const fastrtps::string_255& topic,
            const fastrtps::types::TypeIdentifier* identifier,
            const fastrtps::types::TypeObject* object,
            fastrtps::types::DynamicType_ptr dyn_type) override;

    virtual void on_type_information_received(
        fastrtps::rtps::RTPSParticipant* participant,
        const fastrtps::string_255& topic_name,
        const fastrtps::string_255& type_name,
        const fastrtps::types::TypeInformation& type_information) override;

protected:

    /**
     * @brief Auxiliary method to create the internal RTPS participant.
     */
    void create_participant_(
            const types::DomainId& domain,
            const fastrtps::rtps::RTPSParticipantAttributes& participant_attributes);

    /**
     * @brief Create a writer object
     *
     * Depending on the Topic QoS creates a Basic or Specific Writer.
     */
    std::shared_ptr<IWriter> create_writer_(
            types::DdsTopic topic) override;

    /**
     * @brief Create a reader object
     *
     * Depending on the Topic QoS creates a Basic or Specific Reader.
     */
    std::shared_ptr<IReader> create_reader_(
            types::DdsTopic topic) override;

    /**
     * @brief Create a endpoint from info object
     *
     * Specialized for \c WriterDiscoveryInfo and \c ReaderDiscoveryInfo .
     */
    template<class DiscoveryInfoKind>
    types::Endpoint create_endpoint_from_info_(
            DiscoveryInfoKind& info);

    //! Create a endpoint from common info from method \c create_endpoint_from_info_ .
    template<class DiscoveryInfoKind>
    types::Endpoint create_common_endpoint_from_info_(
            DiscoveryInfoKind& info);

    void internal_notify_type_object_(const std::string& type_name);

    /////
    // RTPS specific methods

    /**
     * @brief Static method that gives the std attributes for a Participant.
     *
     * @note This method must be specialized from inherit classes.
     */
    static fastrtps::rtps::RTPSParticipantAttributes get_participant_attributes_(
            const configuration::ParticipantConfiguration* participant_configuration);

    /////
    // VARIABLES
    //! Internal RTPS Participant
    eprosima::fastrtps::rtps::RTPSParticipant* rtps_participant_;

    //! Domain Id to create the internal RTPS Participant.
    types::DomainId domain_id_;

    //! Participant attributes to create the internal RTPS Participant.
    fastrtps::rtps::RTPSParticipantAttributes participant_attributes_;

    //! Type Object Internal Reader
    std::shared_ptr<InternalReader> type_object_reader_;

    // This vector is required now so no dyn type is destroyed
    // In the future each type object must be sent (completely or better in a shared ptr)
    // in the payload
    std::vector<fastrtps::types::DynamicType_ptr> dyn_types_;
};

} /* namespace rtps */
} /* namespace core */
} /* namespace ddsrecorder */
} /* namespace eprosima */

#endif /* __SRC_DDSRECORDER_PARTICIPANT_IMPLEMENTATIONS_RTPS_COMMONPARTICIPANT_HPP_ */
