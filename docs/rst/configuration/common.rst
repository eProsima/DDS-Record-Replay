.. include:: ../exports/alias.include

.. _common_dds_configuration:
.. _recorder_dds_recorder_configuration_dds_configuration:
.. _replayer_dds_recorder_configuration_dds_configuration:

########################
Common DDS configuration
########################

Recorder and Replayer use DDS Pipe for discovery, filtering, transports, and
QoS. Their shared YAML settings live under ``dds``; tool-wide endpoint defaults
live under ``specs.qos``. An empty or null YAML document is valid, but unknown
keys are rejected by the configuration schema.

Command-line values take precedence over YAML. In particular, ``--domain``
overrides ``dds.domain`` and Replayer ``--input`` overrides
``replayer.input-file``.

Participant and transport settings
==================================

.. _recorder_usage_configuration_domain_id:
.. _replayer_usage_configuration_domain_id:

.. list-table::
   :header-rows: 1
   :widths: 23 11 14 35 17

   * - YAML path
     - Type
     - Required / default
     - Meaning and constraints
     - Precedence / reloadability
   * - ``dds.domain``
     - Integer
     - ``0``
     - DDS domain ID, from ``0`` through ``232``. ``--domain`` wins.
     - CLI > YAML > default. Recreate Recorder from STOPPED or restart Replayer.
   * - ``dds.recorder-profile``
     - String
     - None
     - Recorder only. Fast DDS XML participant profile used by its DDS endpoint.
     - YAML > default. Recreate Recorder.
   * - ``dds.replayer-profile``
     - String
     - None
     - Replayer only. Fast DDS XML participant profile used by its DDS endpoint.
     - YAML > default. Restart Replayer.
   * - ``dds.xml.files``
     - String list
     - Empty
     - XML profile files to load. At least one item is required when present.
     - YAML > default. Recreate or restart.
   * - ``dds.xml.raw``
     - String
     - None
     - Inline Fast DDS XML. ``raw`` and ``files`` may both be supplied.
     - YAML > default. Recreate or restart.
   * - ``dds.transport``
     - Enum
     - ``builtin``
     - ``builtin``, ``udp``, or ``shm``. ``builtin`` lets Fast DDS use its
       built-in transports.
     - YAML > default. Recreate or restart.
   * - ``dds.whitelist-interfaces``
     - String list
     - Empty
     - Restricts network interfaces. Values are passed to the selected Fast DDS
       transport; an empty list does not restrict interfaces.
     - YAML > default. Recreate or restart.
   * - ``dds.ros2-easy-mode``
     - IPv4 string
     - None
     - Enables ROS 2 Easy Mode with the given discovery-server address. Do not
       combine it with an explicit ``dds.transport`` selection.
     - YAML > default. Recreate or restart.
   * - ``dds.ignore-participant-flags``
     - Enum
     - ``no_filter``
     - Controls which discovered participants are ignored: ``no_filter``,
       ``filter_different_host``, ``filter_different_process``,
       ``filter_same_process``, or ``filter_different_and_same_process``.
     - YAML > default. Recreate or restart.

Topic and partition filtering
=============================

.. _recorder_topic_filtering:
.. _replayer_topic_filtering:
.. _recorder_partition_filtering:
.. _replayer_partition_filtering:

``allowlist`` and ``blocklist`` entries match topic names and, optionally, type
names. Patterns use wildcard matching. With no allowlist, all discovered topics
are candidates; a blocklist match always excludes a topic.

.. literalinclude:: ../examples/recorder_filters.yaml
   :language: yaml
   :linenos:

.. list-table::
   :header-rows: 1
   :widths: 23 11 13 36 17

   * - YAML path
     - Type
     - Required / default
     - Meaning and constraints
     - Precedence / reloadability
   * - ``dds.allowlist[]``
     - Topic filter
     - Empty
     - Topics eligible to record or replay. Each entry requires ``name`` and
       may contain ``type``, ``qos``, or a payload ``filter``.
     - YAML > default. Reloaded live.
   * - ``dds.blocklist[]``
     - Topic filter
     - Empty
     - Topics excluded from processing. Same fields as an allowlist entry.
     - YAML > built-in safety filters. Reloaded live without removing them.
   * - ``dds.partitions[]``
     - String list
     - All partitions
     - DDS partitions accepted by Recorder or selected from a recording by
       Replayer. Wildcards are supported.
     - YAML > default. Reloaded live.
   * - ``dds.topics[]``
     - Manual topic
     - Empty
     - Predeclares a topic or overrides its QoS/content filter. ``name`` is
       required; ``type``, ``qos``, ``filter``, and ``participants`` are
       optional.
     - YAML > discovery/default. Only Recorder content-filter edits reload live.
   * - ``dds.topic``
     - Manual topic
     - None
     - Recorder-only legacy singular form. It is mutually exclusive with
       ``dds.topics``; new configurations should use ``topics``.
     - YAML > default. Same reload behavior as ``topics``.
   * - ``dds.builtin-topics[]``
     - Name/type pair
     - Empty
     - Recorder only. Creates a topic without waiting for discovery. Both
       ``name`` and ``type`` are required.
     - YAML > default. Recreate Recorder.

``participants`` accepts internal DDS Pipe participant IDs. Those IDs are an
advanced integration detail and are not a stable end-user interface; omit the
field unless an embedding application defines the IDs explicitly.

Content filters
---------------

The optional ``filter`` in a topic entry is a Fast DDS SQL-like content-filter
expression, for example ``"x > 0 AND color = 'RED'"``. The type must be known
before the filter can be applied. Content filters reduce samples at the DDS
reader; allowlists and blocklists decide which topic bridges exist.

.. _recorder_topic_qos:
.. _replayer_topic_qos:
.. _recorder_manual_topics:
.. _replayer_manual_topics:

Topic QoS
=========

QoS may be set globally under ``specs.qos`` or per topic under
``dds.topics[].qos``. Per-topic values take precedence. Values learned during
discovery or stored in the recording are used when they are not overridden.

.. list-table::
   :header-rows: 1
   :widths: 20 12 14 35 19

   * - QoS key
     - Type
     - Required / default
     - Meaning
     - Precedence / reloadability
   * - ``durability``
     - Boolean
     - ``false``
     - ``true`` selects transient-local; ``false`` selects volatile.
     - Per-topic > global > learned/default. Recreate or restart.
   * - ``reliability``
     - Boolean
     - ``false``
     - ``true`` selects reliable; ``false`` selects best-effort.
     - Per-topic > global > learned/default. Recreate or restart.
   * - ``ownership``
     - Boolean
     - ``false``
     - ``true`` selects exclusive; ``false`` selects shared ownership.
     - Per-topic > global > learned/default. Recreate or restart.
   * - ``partitions``
     - Boolean
     - ``false``
     - Whether the topic uses DDS partitions.
     - Per-topic > global > learned/default. Recreate or restart.
   * - ``history-depth``
     - Integer ≥ 0
     - ``5000``
     - Endpoint history depth.
     - Per-topic > global > learned/default. Recreate or restart.
   * - ``keyed``
     - Boolean
     - ``false``
     - Whether the topic type has a DDS key.
     - Per-topic > global > learned/default. Recreate or restart.
   * - ``max-rx-rate``
     - Number ≥ 0
     - ``0``
     - Recorder only. Maximum accepted sample rate in hertz; ``0`` is unlimited.
     - Per-topic > global > default. Recreate Recorder.
   * - ``downsampling``
     - Integer ≥ 1
     - ``1``
     - Recorder only. Keep one of every N received samples.
     - Per-topic > global > default. Recreate Recorder.
   * - ``max-tx-rate``
     - Number ≥ 0
     - ``0``
     - Replayer only. Maximum publication rate in hertz; ``0`` is unlimited.
     - Per-topic > global > recording/default. Restart Replayer.

.. _recorder_history_depth:
.. _replayer_history_depth:
.. _recorder_max_rx_rate:
.. _replayer_max_tx_rate:
.. _recorder_downsampling:

Runtime reload
==============

Recorder and Replayer watch whichever YAML file they loaded, whether selected
with ``--config-path`` or found under the conventional filename. ``--reload-time
N`` adds polling every N seconds for environments where file notifications do
not work, such as some symbolic-link deployments. A value of ``0`` disables
polling; direct file watching still applies to a regular configuration file.
No reload mechanism is created when execution uses only built-in defaults.

.. list-table::
   :header-rows: 1
   :widths: 30 22 48

   * - Setting
     - Live behavior
     - Notes
   * - ``dds.allowlist`` / ``dds.blocklist``
     - Reloaded
     - Existing and future topic bridges are reevaluated.
   * - ``dds.partitions``
     - Reloaded
     - Recorder readers and Replayer recording selection are updated.
   * - ``dds.topics[].filter``
     - Recorder: reloaded
     - Added, changed, and removed Recorder content filters are applied to
       active and future readers. Replayer content filters require restart.
   * - Other DDS, QoS, transport, logging, or tool settings
     - Restart required
     - Recorder reloads these when a STOPPED instance is started and recreated;
       Replayer must be restarted.

If a changed file is syntactically invalid, contains an unknown key, or fails a
cross-field check, the process logs a configuration error and continues with
the previous valid live configuration.

.. _recorder_ignore_participant_flags:
.. _replayer_ignore_participant_flags:
.. _recorder_custom_transport_descriptors:
.. _replayer_custom_transport_descriptors:
.. _recorder_easy_mode:
.. _replayer_easy_mode:
.. _recorder_interface_whitelist:
.. _replayer_interface_whitelist:

The complete tool-specific fields are listed in :ref:`recorder_usage_configuration`
and :ref:`replayer_usage_configuration`.
