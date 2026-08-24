.. include:: ../../exports/alias.include

.. _replayer_usage_configuration:

######################
Replayer configuration
######################

Replayer accepts a YAML document with the optional top-level groups ``dds``,
``replayer``, and ``specs``. Unknown keys are errors. See
:ref:`common_dds_configuration` for DDS discovery, filters, partitions,
transports, manual topics, QoS, and runtime reload behavior.

An input file is mandatory for execution, but it may come from
``--input`` or ``replayer.input-file``. The CLI value takes precedence.

Input and playback
==================

.. list-table::
   :header-rows: 1
   :widths: 25 11 13 34 17

   * - YAML path
     - Type
     - Required / default
     - Meaning and constraints
     - Precedence / reloadability
   * - ``replayer.input-file``
     - String
     - Required to run; none
     - Readable MCAP or DDS Record & Replay SQLite ``.db`` file.
     - CLI > YAML. Restart Replayer.
   * - ``replayer.rate``
     - Number > 0
     - ``1.0``
     - Playback speed relative to recorded time. ``2`` is twice as fast and
       ``0.5`` is half speed.
     - YAML > default. Restart Replayer.
   * - ``replayer.replay-types``
     - Boolean
     - ``true``
     - Publish available recorded dynamic types before data samples.
     - YAML > default. Restart Replayer.
   * - ``replayer.begin-time``
     - Timestamp
     - Start of file
     - Discard samples before this recorded timestamp.
     - YAML > file boundary. Restart Replayer.
   * - ``replayer.end-time``
     - Timestamp
     - End of file
     - Discard samples after this recorded timestamp. It must be later than
       ``begin-time`` when both are set.
     - YAML > file boundary. Restart Replayer.
   * - ``replayer.start-replay-time``
     - Timestamp
     - Process start
     - Delay the first selected sample until this wall-clock time. A time in
       the past starts immediately.
     - YAML > process start. Restart Replayer.

Replayer chooses the reader from the input extension, case-insensitively.
MCAP playback uses message ``logTime``. SQLite playback uses ``Messages.log_time``.
Those values represent Recorder receive time unless the MCAP recording was
created with ``log-publish-time: true``.

.. _replayer_replay_configuration_begintime:
.. _replayer_replay_configuration_endtime:
.. _replayer_replay_configuration_startreplaytime:

Timestamp objects
-----------------

``begin-time``, ``end-time``, and ``start-replay-time`` share this shape:

.. literalinclude:: ../../examples/replayer_time_range.yaml
   :language: yaml
   :linenos:

.. list-table::
   :header-rows: 1
   :widths: 20 11 16 36 17

   * - Key
     - Type
     - Required / default
     - Meaning
     - Precedence / reloadability
   * - ``datetime``
     - String
     - Required
     - Date/time text parsed using ``format``.
     - YAML only. Restart Replayer.
   * - ``format``
     - String
     - ``%Y-%m-%d_%H-%M-%S``
     - ``strptime``-compatible format.
     - YAML > default. Restart Replayer.
   * - ``local``
     - Boolean
     - ``true``
     - Interpret ``datetime`` in local time; false interprets it as UTC.
     - YAML > default. Restart Replayer.
   * - ``milliseconds``
     - Integer ≥ 0
     - ``0``
     - Additional millisecond precision.
     - YAML > default. Restart Replayer.
   * - ``microseconds``
     - Integer ≥ 0
     - ``0``
     - Additional microsecond precision.
     - YAML > default. Restart Replayer.
   * - ``nanoseconds``
     - Integer ≥ 0
     - ``0``
     - Additional nanosecond precision.
     - YAML > default. Restart Replayer.

.. _replayer_replay_configuration_playbackrate:

Timing behavior
---------------

The first sample selected by ``begin-time`` establishes the playback origin.
Subsequent delays preserve differences between recorded timestamps divided by
``rate``. ``start-replay-time`` shifts that origin; it does not change which
samples the begin/end range selects.

.. _replayer_replay_configuration_replaytypes:

Types and ROS 2 services
------------------------

With ``replay-types: true``, Replayer registers and publishes type information
available in the recording. A receiver that already knows a type can consume
CDR even if the recording has no stored dynamic type. Type-dependent tools may
not be able to decode that sample.

Replayer automatically blocks ROS 2 service request and response topics
(``rq/*`` and ``rr/*``). Replaying them could make a recording appear to be a
live service server. This safety filter is applied in addition to the user
allowlist and blocklist.

.. _replayer_specs_topic_qos:
.. _replayer_specs_logging:

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
     - YAML > default. Restart Replayer.
   * - ``specs.qos``
     - Object
     - Built-in QoS
     - Default published topic QoS; see :ref:`replayer_topic_qos`.
     - Per-topic > YAML global > recording/default. Restart Replayer.
   * - ``specs.rtps``
     - Boolean
     - ``false``
     - Use the lower-level RTPS participant implementation instead of DDS.
     - YAML > default. Restart Replayer.
   * - ``specs.wait-all-acked-timeout``
     - Milliseconds ≥ 0
     - ``0``
     - How long a reliable writer waits for acknowledgements when it closes;
       ``0`` does not wait.
     - YAML > default. Restart Replayer.
   * - ``specs.logging.stdout``
     - Boolean
     - ``true``
     - Enable the standard-output log consumer.
     - CLI where applicable > YAML > default. Restart Replayer.
   * - ``specs.logging.verbosity``
     - Enum
     - ``warning``
     - Minimum severity: ``info``, ``warning``, or ``error``.
     - CLI > YAML > default. Restart Replayer.
   * - ``specs.logging.filter.<severity>``
     - Regex string
     - Tool defaults
     - Category regex for ``info``, ``warning``, or ``error``.
     - CLI > YAML > default. Restart Replayer.
   * - ``specs.logging.publish.enable``
     - Boolean; required in block
     - ``false``
     - Publish logs over DDS.
     - YAML > default. Restart Replayer.
   * - ``specs.logging.publish.domain``
     - Integer 0–232
     - ``0``
     - DDS log domain.
     - YAML > default. Restart Replayer.
   * - ``specs.logging.publish.topic-name``
     - String
     - None
     - DDS log topic; required when publishing is enabled.
     - YAML > default. Restart Replayer.

CLI logging options override YAML. Treat ``--debug`` as mutually exclusive
with explicit ``--log-filter`` or ``--log-verbosity``.

Runtime changes
===============

While Replayer is running, configuration reload applies ``dds.allowlist``,
``dds.blocklist``, and ``dds.partitions``. Input path, time range, playback
rate, start time, replay-types, topic QoS, transports, logging, and thread count
are fixed for the current run. Restart Replayer to change them.

.. _replayer_usage_configuration_general_example:

Validated example
=================

Save this file and replace ``recording.mcap`` with the real path:

.. literalinclude:: ../../examples/replayer_basic.yaml
   :language: yaml
   :linenos:

The shipped comprehensive reference is
``resources/configurations/replayer/complete_config.yaml``.
