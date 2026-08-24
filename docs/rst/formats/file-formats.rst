.. include:: ../exports/alias.include

.. _file_formats:

###########################
Recording files and formats
###########################

Recorder can write MCAP, SQLite, or both in the same run. Select a format based
on the next consumer rather than file extension alone.

.. list-table::
   :header-rows: 1
   :widths: 20 26 27 27

   * - Need
     - MCAP
     - SQLite with CDR
     - SQLite with JSON
   * - Replay with DDS Replayer
     - Yes
     - Yes
     - No, unless CDR is also stored
   * - Generic SQL queries
     - No
     - Metadata queries
     - Metadata and decoded samples
   * - Foxglove and MCAP tools
     - Best choice
     - No
     - No
   * - Record without a known type
     - CDR with a blank schema when allowed
     - Yes
     - No decoded value

File lifecycle
==============

Recorder first opens a temporary path ending in ``.mcap.tmp~`` or ``.db.tmp~``.
The final name becomes visible when the writer closes after graceful process
shutdown, SUSPEND, STOP, or rotation. A temporary file may lack the final MCAP
summary or pending SQLite WAL changes and is not a supported replay input.

The normal filename is:

.. code-block:: text

   <timestamp>_<configured-name>[_<rotation-id>].<extension>

The rotation ID appears when ``max-size`` is greater than ``max-file-size``.
See :ref:`recorder_usage_configuration_resource_limits`.

MCAP
====

Each MCAP message contains a CDR payload, sequence number, publication time,
and log time. Channels contain topic name, type, recorded QoS, partition
metadata, and ROS 2 format information. When discovered, type schemas are
stored as OMG IDL or ROS 2 message definitions; the complete dynamic-type data
is also stored as a DDS Record & Replay attachment. Version and source-writer
metadata allow Replayer to restore additional DDS semantics.

Generic MCAP readers can enumerate channels and consume raw payloads. A viewer
can decode values only when it understands the stored schema encoding and a
usable schema was recorded.

SQLite
======

Recorder uses SQLite WAL mode while writing and creates these tables:

.. list-table::
   :header-rows: 1
   :widths: 28 72

   * - Table
     - Contents
   * - ``Types``
     - Type name, serialized type information/object, and ROS 2 marker.
   * - ``Topics``
     - Topic/type pairs, QoS, and ROS 2 topic marker.
   * - ``Messages``
     - Writer GUID, sequence number, optional JSON, optional CDR, key, topic,
       type, log time, and publication time.
   * - ``Partitions``
     - Distinct recorded partition names.
   * - ``TopicsPartitions``
     - Topic/type-to-partition relationships.
   * - ``MessagesPartitions``
     - Message-to-partition relationships.

Example read-only queries after Recorder has finalized the database:

.. code-block:: sql

   SELECT topic, COUNT(*) AS samples,
          MIN(log_time) AS first_sample,
          MAX(log_time) AS last_sample
   FROM Messages
   GROUP BY topic
   ORDER BY topic;

.. code-block:: sql

   SELECT topic, log_time, data_json
   FROM Messages
   WHERE data_json IS NOT NULL
   ORDER BY log_time
   LIMIT 20;

``data-format: cdr`` minimizes decoding work and remains replayable.
``data-format: json`` requires type information and is not replayable.
``data-format: both`` is the recommended choice when both replay and SQL
inspection are required.

Compatibility guidance
======================

Use the same or a newer compatible DDS Record & Replay version to replay a
recording. Replayer logs the version stored in MCAP and warns when it differs.
QoS overrides, topic filters, and a target domain may be changed during replay;
they do not modify the source file.
