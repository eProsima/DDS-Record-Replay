.. include:: ../exports/alias.include

.. _glossary:

########
Glossary
########

.. glossary::

   CDR
      Common Data Representation, the binary serialization used for DDS
      payloads. Replayer requires CDR data; decoded JSON alone is not enough.

   DataReader
      DDS endpoint that receives samples for a topic. A DataReader belongs to
      one DomainParticipant.

   DataWriter
      DDS endpoint that publishes samples for a topic. A DataWriter belongs to
      one DomainParticipant.

   DDS domain
      Logical communication space identified by a domain ID. Participants in
      different domains do not discover one another by default.

   DDS Pipe
      Shared eProsima communication core that discovers endpoints and routes
      serialized payloads between participants. Recorder, Replayer, DDS Router,
      and Fast DDS Spy build product-specific workflows on this core.

   DomainParticipant
      Entry point for an application in one DDS domain and factory for DDS
      publishers, subscribers, topics, and endpoints.

   DynamicTypes
      DDS type representation that can be discovered and used at runtime
      without generated application classes. Recorder uses it to store schemas
      and decode SQL JSON.

   GUID
      Globally unique identifier for a DDS entity. A GUID contains a GUID prefix
      and an entity ID.

   logTime
      MCAP timestamp used by Replayer for ordering and delays. Recorder normally
      uses receive time, or publication time when ``log-publish-time`` is true.

   MCAP
      Open container format for timestamped heterogeneous data. DDS Record &
      Replay stores CDR messages, channels, schemas, and product metadata in it.

   Partition
      Named DDS routing scope within a domain. Publishers and subscribers
      communicate only through matching partition expressions.

   Publication timestamp
      Source timestamp attached by a DDS writer. It can differ from the time
      Recorder receives the sample.

   QoS
      DDS Quality of Service policies controlling compatibility and behavior,
      including reliability, durability, ownership, history, and partitions.

   SQLite
      Embedded relational database format used for queryable recordings. The
      project uses the ``.db`` extension and can store CDR, decoded JSON, or both.

   Topic
      DDS communication subject identified by a topic name and type name.

For the full DDS entity model, see |FastDDSDocs|.
