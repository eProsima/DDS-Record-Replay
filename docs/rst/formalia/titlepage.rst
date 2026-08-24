###################
DDS Record & Replay
###################

*eProsima DDS Record & Replay* records live DDS traffic and publishes it again
later while preserving message order and timing. It is designed for debugging,
reproducible testing, demonstrations, and offline inspection of DDS systems.

Choose a workflow
=================

.. list-table::
   :header-rows: 1
   :widths: 22 48 30

   * - Goal
     - Start here
     - Tool
   * - Capture and replay traffic
     - :ref:`record_replay_quickstart`
     - ``ddsrecorder`` and ``ddsreplayer``
   * - Keep a rolling pre-event window
     - :ref:`recorder_remote_control`
     - ``ddsrecorder`` and Remote Controller
   * - Inspect or query a recording
     - :ref:`file_formats`
     - MCAP tools or SQLite clients
   * - Convert MCAP to SQLite
     - :ref:`replayer_usage_mcap_convert`
     - ``mcap-convert``
   * - Configure DDS discovery and filtering
     - :ref:`common_dds_configuration`
     - Recorder or Replayer

What is included
================

``ddsrecorder``
    Discovers DDS topics and writes samples, topic metadata, partitions, QoS,
    and available dynamic type information to MCAP and/or SQLite.

``ddsreplayer``
    Reads an MCAP or DDS Record & Replay SQLite ``.db`` file and publishes its
    samples into a DDS domain. Playback can be filtered or shifted in time and
    can run faster or slower than the original recording.

Remote Controller
    Controls Recorder state through DDS. It can start, pause, capture an event
    window, suspend, stop, or close a Recorder. A graphical controller is
    available when it is enabled at build time.

``mcap-convert``
    Converts an MCAP recording into the SQLite schema used by this project. It
    does not publish DDS traffic.

How the tools use DDS Pipe
==========================

DDS Pipe is the shared communication core. It discovers participants and
topics, applies routing and filtering rules, and moves serialized payloads
between endpoints. Recorder connects a DDS endpoint to storage writers;
Replayer connects a recording reader to a DDS endpoint. DDS Router and Fast
DDS Spy use the same core for their own products, so concepts such as topic
filtering, transports, and participant QoS are intentionally consistent.

.. figure:: /rst/figures/record-replay-architecture.svg
   :align: center
   :alt: DDS publishers and subscribers connect through the DDS Pipe core to Recorder and Replayer storage endpoints.

   Recorder and Replayer data flow through the shared DDS Pipe core.

Supported file workflows
========================

.. list-table::
   :header-rows: 1
   :widths: 18 18 18 18 28

   * - Format
     - Record
     - Replay
     - Convert from MCAP
     - Best suited for
   * - MCAP
     - Yes; default
     - Yes
     - Source format
     - Portable recordings and visualization tools
   * - SQLite ``.db``
     - Yes; optional
     - Yes, when CDR data is stored
     - Yes
     - SQL queries and application-specific analysis

The next step is the :ref:`record_replay_quickstart`. For a production setup,
read :ref:`file_formats` before selecting an output format and resource limit.

Commercial support
==================

For commercial support, contact ``info@eprosima.com`` or visit
`eProsima <https://www.eprosima.com/>`__.
