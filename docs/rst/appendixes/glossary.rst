.. include:: ../exports/alias.include

.. _glossary:

########
Glossary
########

Networking nomenclature
=======================

.. glossary::

    LAN
        **Local Area Network**

DDS Record & Replay nomenclature
================================

.. glossary::

    MCAP
        Modular container file format for heterogeneous timestamped data.

DDS nomenclature
================

.. glossary::

    DataReader
        DDS element that subscribes to a specific Topic.
        It belong to one and only one Participant, and it is uniquely identified by a Guid.

        See `Fast DDS documentation <https://fast-dds.docs.eprosima.com/en/v2.4.1/fastdds/dds_layer/subscriber/dataReader/dataReader.html>`__
        for further information.

    DataWriter
        DDS entity that publish data in a specific Topic.
        It belong to one and only one Participant, and it is uniquely identified by a Guid.

        See `Fast DDS documentation <https://fast-dds.docs.eprosima.com/en/v2.4.1/fastdds/dds_layer/publisher/dataWriter/dataWriter.html>`__
        for further information.

    Domain Id
        The Domain Id is a virtual partition for DDS networks.
        Only DomainParticipants with the same Domain Id would be able to communicate to each other.
        DomainParticipants  in different Domains will not even discover each other.

        See `Fast DDS documentation <https://fast-dds.docs.eprosima.com/en/v2.4.1//fastdds/dds_layer/domain/domain.html>`__
        for further information.

    DomainParticipant
        A DomainParticipant is the entry point of the application to a DDS Domain.
        Every DomainParticipant is linked to a single domain from its creation, and cannot change such domain.
        It also acts as a factory for Publisher, Subscriber and Topic.

        See `Fast DDS documentation <https://fast-dds.docs.eprosima.com/en/v2.4.1/fastdds/dds_layer/domain/domainParticipant/domainParticipant.html>`__
        for further information.

    Endpoint
        DDS element that publish or subscribes in a specific Topic. Endpoint kinds are *DataWriter* or *DataReader*.

    Guid
        Global Unique Identifier.
        It contains a GuidPrefix and an EntityId.
        The EntityId uniquely identifies sub-entities inside a Participant.
        Identifies uniquely a DDS entity (DomainParticipant, DataWriter or DataReader).

    GuidPrefix
        Global Unique Identifier shared by a Participant and all its sub-entities.
        Identifies uniquely a DDS Participant.

    Topic
        DDS isolation abstraction to encapsulate subscriptions and publications.
        Each Topic is uniquely identified by a topic name and a topic type name (name of the data type it transmits).

        See `Fast DDS documentation <https://fast-dds.docs.eprosima.com/en/v2.4.1/fastdds/dds_layer/topic/topic.html>`__
        for further information.

    DynamicTypes
        The dynamic topic types offer the possibility to work over DDS without the restrictions related to the IDLs.
        Using them, the users can declare the different types that they need and manage the information directly, avoiding the additional step of updating the IDL file and the generation of C++ classes.

        See `Fast DDS documentation <https://fast-dds.docs.eprosima.com/en/latest/fastdds/dynamic_types/dynamic_types.html>`__
        for further information.
