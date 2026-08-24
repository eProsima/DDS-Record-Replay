.. include:: ../../exports/alias.include

.. _recorder_usage_configuration:

######################
Recorder configuration
######################

Recorder accepts a YAML document with four optional top-level groups:
``dds``, ``recorder``, ``remote-controller``, and ``specs``. Unknown keys are
errors. An empty document uses built-in defaults and records every discovered
topic on domain ``0`` to a timestamped MCAP file.

See :ref:`common_dds_configuration` for ``dds`` fields, filters, transports,
manual topics, and QoS. This page covers Recorder-specific fields.

Recording and buffering
=======================

.. _recorder_usage_configuration_cleanup_period:
.. _recorder_usage_configuration_event_window:
.. _recorder_usage_configuration_max_number_pending_samples:
.. _recorder_usage_configuration_onlywithtype:
.. _recorder_usage_configuration_recordtypes:
.. _recorder_usage_configuration_topictypeformat:

.. list-table::
   :header-rows: 1
   :widths: 24 10 13 37 16

   * - YAML path
     - Type
     - Required / default
     - Meaning and constraints
     - Precedence / reloadability
   * - ``recorder.buffer-size``
     - Integer > 0
     - ``100``
     - Samples accumulated before a batch is written while RUNNING.
     - YAML > default. Recreate Recorder from STOPPED.
   * - ``recorder.event-window``
     - Seconds > 0
     - ``20``
     - Rolling amount of recent data retained while PAUSED.
     - YAML > default. Recreate Recorder from STOPPED.
   * - ``recorder.cleanup-period``
     - Seconds > 0
     - Twice ``event-window``
     - Accepted for compatibility. In v1.5.3 the constructor recomputes this
       value as twice ``event-window`` after parsing, so an explicit value does
       not change the effective cleanup period.
     - Constructor-derived. Recreate Recorder from STOPPED.
   * - ``recorder.max-pending-samples``
     - Integer ≥ -1
     - ``5000``
     - Per-type limit while waiting for type information. ``-1`` is unlimited;
       ``0`` disables the pending queue.
     - YAML > default. Recreate Recorder from STOPPED.
   * - ``recorder.only-with-type``
     - Boolean
     - ``false``
     - When true, write a sample only after its dynamic type is available.
     - YAML > default. Recreate Recorder from STOPPED.
   * - ``recorder.record-types``
     - Boolean
     - ``true``
     - Store received dynamic type information for replay and inspection.
     - YAML > default. Recreate Recorder from STOPPED.
   * - ``recorder.ros2-types``
     - Boolean
     - ``false``
     - Encode MCAP schemas and topic names for ROS 2 tooling instead of raw DDS
       OMG IDL conventions.
     - YAML > default. Recreate Recorder from STOPPED.

When a type arrives, queued samples of that type are processed. If the pending
limit is reached, the oldest pending sample is discarded. On shutdown, pending
samples without a type are written only when ``only-with-type`` is false.
Recorder also applies the built-in ROS 2 service safety filters ``rq/*`` and
``rr/*`` in addition to the configured blocklist.

.. _recorder_usage_configuration_outputfile:

Output naming and lifecycle
===========================

.. list-table::
   :header-rows: 1
   :widths: 24 10 15 34 17

   * - YAML path
     - Type
     - Required / default
     - Meaning
     - Precedence / reloadability
   * - ``recorder.output.path``
     - String
     - ``.``
     - Directory for all enabled output formats.
     - YAML > default. Recreate Recorder from STOPPED.
   * - ``recorder.output.filename``
     - String
     - ``output``
     - Base name, without an extension.
     - YAML > default. Recreate Recorder from STOPPED.
   * - ``recorder.output.timestamp-format``
     - String
     - ``%Y-%m-%d_%H-%M-%S_%Z``
     - ``strftime`` format prepended to the base name.
     - YAML > default. Recreate Recorder from STOPPED.
   * - ``recorder.output.local-timestamp``
     - Boolean
     - ``true``
     - Use local time when true and UTC when false.
     - YAML > default. Recreate Recorder from STOPPED.
   * - ``recorder.output.safety-margin``
     - Byte quantity
     - ``10MiB``
     - Free-space reserve used by resource-limit checks. Values below 10 MiB
       are raised to 10 MiB.
     - YAML > default. Recreate Recorder from STOPPED.

The active file is ``<final-name>.<extension>.tmp~``. Closing Recorder,
SUSPEND, STOP, or file rotation finalizes the writer and renames the temporary
file. If multiple files are possible, Recorder appends a numeric file ID. An
existing final or temporary path is reported as an error; use unique names or
the remote ``stop`` command's overwrite-protection argument.

Output selection
================

MCAP is enabled by default and SQL is disabled. If SQL is explicitly enabled
without an ``mcap`` block, Recorder disables the implicit MCAP output. To write
both formats, explicitly enable both:

.. literalinclude:: ../../examples/recorder_dual_output.yaml
   :language: yaml
   :linenos:

At least one format must be enabled.

.. _recorder_usage_configuration_mcap:
.. _recorder_usage_configuration_logpublishtime:
.. _recorder_usage_configuration_compression:

MCAP fields
-----------

.. list-table::
   :header-rows: 1
   :widths: 27 10 14 32 17

   * - YAML path
     - Type
     - Required / default
     - Meaning
     - Precedence / reloadability
   * - ``recorder.mcap.enable``
     - Boolean
     - Required in block; ``true`` implicitly
     - Enable MCAP output.
     - YAML > implicit default. Recreate Recorder from STOPPED.
   * - ``recorder.mcap.log-publish-time``
     - Boolean
     - ``false``
     - Use the DDS source publication timestamp as MCAP ``logTime``. Otherwise
       use the Recorder receive timestamp.
     - YAML > default. Recreate Recorder from STOPPED.
   * - ``recorder.mcap.compression.algorithm``
     - Enum
     - ``zstd``
     - ``none``, ``zstd``, or ``lz4``.
     - YAML > default. Recreate Recorder from STOPPED.
   * - ``recorder.mcap.compression.level``
     - Enum
     - ``default``
     - ``fastest``, ``fast``, ``default``, ``slow``, or ``slowest``.
     - YAML > default. Recreate Recorder from STOPPED.
   * - ``recorder.mcap.compression.force``
     - Boolean
     - ``false``
     - Keep a compressed chunk even when compression does not reduce its size.
     - YAML > default. Recreate Recorder from STOPPED.

.. _recorder_usage_configuration_sql:
.. _recorder_usage_configuration_sql_data_format:

SQLite fields
-------------

.. list-table::
   :header-rows: 1
   :widths: 25 10 14 34 17

   * - YAML path
     - Type
     - Required / default
     - Meaning
     - Precedence / reloadability
   * - ``recorder.sql.enable``
     - Boolean
     - Required in block; ``false`` implicitly
     - Enable SQLite ``.db`` output.
     - YAML > implicit default. Recreate Recorder from STOPPED.
   * - ``recorder.sql.data-format``
     - Enum
     - ``both``
     - ``cdr`` stores replayable serialized payloads, ``json`` stores
       query-friendly decoded data, and ``both`` stores both representations.
     - YAML > default. Recreate Recorder from STOPPED.

JSON conversion requires a dynamic type. A sample whose type is unavailable
can still be stored as CDR when the selected format includes CDR. See
:ref:`file_formats` for the SQL schema and replay limitations.

.. _recorder_usage_configuration_resource_limits:

Resource limits
---------------

Both ``recorder.mcap.resource-limits`` and
``recorder.sql.resource-limits`` accept the fields below. Quantities consist of
a non-negative number and a case-insensitive decimal or binary unit: ``B``,
``KB``, ``MB``, ``GB``, ``TB``, ``PB``, ``KiB``, ``MiB``, ``GiB``, ``TiB``, or
``PiB``.

.. list-table::
   :header-rows: 1
   :widths: 25 14 14 30 17

   * - YAML path suffix under each ``resource-limits`` block
     - Required / default
     - Applies to
     - Behavior
     - Precedence / reloadability
   * - ``max-file-size``
     - Unlimited
     - MCAP; SQL aliases it to total size
     - Finalize the current file when the writer approaches this size.
     - YAML > unlimited. Recreate Recorder from STOPPED.
   * - ``max-size``
     - Unlimited
     - All files of one format
     - Total managed output size. It cannot be smaller than
       ``max-file-size``.
     - YAML > unlimited. Recreate Recorder from STOPPED.
   * - ``size-tolerance``
     - ``1MiB``
     - Size estimation
     - Allowed overshoot and size-check granularity; values below 1 MiB are
       ignored in favor of 1 MiB.
     - YAML > default. Recreate Recorder from STOPPED.
   * - ``log-rotation``
     - ``false``
     - Closed files
     - Delete the oldest managed file when space is needed. A finite file or
       total limit is required.
     - YAML > default. Recreate Recorder from STOPPED.

SQLite produces one database at a time; when one of its size fields is set,
the implementation makes ``max-file-size`` and ``max-size`` equal. MCAP can
split into numbered files. Rotation manages files created during the current
Recorder execution; it is not a general disk cleanup service.

.. _recorder_usage_configuration_remote_controller:

Remote Controller fields
========================

.. list-table::
   :header-rows: 1
   :widths: 27 10 15 31 17

   * - YAML path
     - Type
     - Required / default
     - Meaning
     - Precedence / reloadability
   * - ``remote-controller.enable``
     - Boolean
     - ``true``
     - Create command and status DDS endpoints.
     - YAML > default. Not live; recreate Recorder.
   * - ``remote-controller.domain``
     - Integer 0–232
     - Recorder domain
     - Domain used by controller topics.
     - YAML > Recorder domain. Not live; recreate Recorder.
   * - ``remote-controller.initial-state``
     - Enum, case-insensitive
     - ``RUNNING``
     - ``RUNNING``, ``PAUSED``, ``SUSPENDED``, or ``STOPPED``.
     - YAML > default. Not live; recreate Recorder.
   * - ``remote-controller.command-topic-name``
     - String
     - ``/ddsrecorder/command``
     - Topic receiving ``DdsRecorderCommand``.
     - YAML > default. Not live; recreate Recorder.
   * - ``remote-controller.status-topic-name``
     - String
     - ``/ddsrecorder/status``
     - Topic publishing ``DdsRecorderStatus``.
     - YAML > default. Not live; recreate Recorder.

Controller settings are established when the command receiver starts and are
not hot-reloaded. See :ref:`recorder_remote_control` for states, commands, and
wire types.

.. _recorder_specs_topic_qos:
.. _recorder_specs_logging:
.. _recorder_specs_monitor:

Advanced specifications
=======================

.. list-table::
   :header-rows: 1
   :widths: 27 11 13 32 17

   * - YAML path
     - Type
     - Required / default
     - Meaning
     - Precedence / reloadability
   * - ``specs.threads``
     - Integer > 0
     - ``12``
     - DDS Pipe worker threads.
     - YAML > default. Recreate Recorder from STOPPED.
   * - ``specs.qos``
     - Object
     - Built-in QoS
     - Default topic QoS; see :ref:`recorder_topic_qos`.
     - Per-topic > YAML global > default. Recreate Recorder.
   * - ``specs.rtps``
     - Boolean
     - ``false``
     - Use the lower-level RTPS participant implementation instead of DDS.
     - YAML > default. Recreate Recorder.
   * - ``specs.logging.stdout``
     - Boolean
     - ``true``
     - Enable the standard-output log consumer.
     - CLI where applicable > YAML > default. Restart process.
   * - ``specs.logging.verbosity``
     - Enum
     - ``warning``
     - Minimum severity: ``info``, ``warning``, or ``error``.
     - CLI > YAML > default. Restart process.
   * - ``specs.logging.filter.<severity>``
     - Regex string
     - Tool defaults
     - Category regex for ``info``, ``warning``, or ``error`` messages.
     - CLI > YAML > default. Restart process.
   * - ``specs.logging.publish.enable``
     - Boolean; required in block
     - ``false``
     - Publish log records over DDS.
     - YAML > default. Restart process.
   * - ``specs.logging.publish.domain``
     - Integer 0–232
     - ``0``
     - DDS log domain.
     - YAML > default. Restart process.
   * - ``specs.logging.publish.topic-name``
     - String
     - None
     - DDS log topic; required when publishing is enabled.
     - YAML > default. Restart process.
   * - ``specs.monitor.domain``
     - Integer 0–232
     - ``0``
     - Default domain for monitoring topics.
     - YAML > default. Recreate Recorder.
   * - ``specs.monitor.status`` / ``topics``
     - Object
     - Disabled
     - Each accepts required ``enable`` plus optional ``domain``, ``period`` in
       milliseconds (> 0), and ``topic-name``.
     - YAML > inherited monitor defaults. Recreate Recorder.

CLI logging options override YAML. Treat ``--debug`` as mutually exclusive
with explicit ``--log-filter`` or ``--log-verbosity``.

.. _recorder_usage_configuration_general_example:

Validated examples
==================

The :ref:`record_replay_quickstart` includes validated MCAP and SQLite files.
The shipped comprehensive configuration is available at
``resources/configurations/recorder/complete_config.yaml``. It is a reference,
not a recommended baseline: start with the smallest configuration that states
your DDS domain, filters, and output policy.

.. _recorder_builtin_topics:
.. _recorder_usage_fastdds_configuration:

Recorder-only built-in topics and Fast DDS XML profiles are documented in
:ref:`common_dds_configuration`.
