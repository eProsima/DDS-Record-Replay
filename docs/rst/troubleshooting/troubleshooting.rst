.. include:: ../exports/alias.include

.. _troubleshooting:

###############
Troubleshooting
###############

Start with ``--log-verbosity info`` and keep the default tool log filter. Avoid
combining explicit logging options with ``--debug``.

No topics are discovered
========================

Check these items in order:

#. Recorder/Replayer and the DDS application use the same domain. A CLI
   ``--domain`` value overrides YAML.
#. ``dds.allowlist``, ``dds.blocklist``, and ``dds.partitions`` admit the topic.
#. The selected ``dds.transport`` and ``dds.whitelist-interfaces`` can reach the
   peer. Temporarily remove interface restrictions to isolate the cause.
#. ``dds.ignore-participant-flags`` is not excluding the peer's host or process.
#. Fast DDS XML profiles load successfully and the configured profile name
   exists.

Recorder discovers a topic but writes no samples
================================================

* Confirm the state is RUNNING. PAUSED waits for ``event``; SUSPENDED discards
  samples; STOPPED has no Recorder DDS entities.
* Check a per-topic content filter, ``max-rx-rate``, and ``downsampling``.
* With ``only-with-type: true``, verify the publisher provides dynamic type
  information. Otherwise samples wait for the type and may reach
  ``max-pending-samples``.
* Verify at least one output is enabled and the output directory is writable.

The type or decoded data is missing
===================================

CDR can be recorded without a type, but JSON and schema-aware viewers require
one. Keep ``record-types: true`` and ensure the publisher participates in Fast
DDS type discovery. For ROS 2 viewers, configure ``ros2-types`` for the source
system. ``only-with-type: true`` trades completeness of raw capture for a
guarantee that every stored sample has a schema.

Configuration is rejected or a reload has no effect
====================================================

Unknown YAML keys are rejected. Check indentation, spelling, enum case, numeric
ranges, and cross-field rules. In particular:

* at least one Recorder output must be enabled;
* ``max-size`` cannot be smaller than ``max-file-size``;
* rotation requires a finite limit or usable safety margin;
* Replayer ``begin-time`` must be earlier than ``end-time``; and
* a timestamp block requires ``datetime``.

On a bad live edit, the process logs the error and retains the previous valid
configuration. Only filters and partitions are hot-reloadable; see the matrix
in :ref:`common_dds_configuration`. Restart Replayer or use Recorder STOPPED →
START for other changes.

Only a ``.tmp~`` file exists
============================

The writer has not finalized. End Recorder with :kbd:`Ctrl+C`, ``SIGTERM``,
SUSPEND, STOP, or CLOSE. After a crash or forced termination, the temporary
file is not guaranteed to contain a valid MCAP summary or committed SQLite WAL.
Do not rename it and assume it is complete; preserve it for recovery work and
make a copy before attempting third-party repair tools.

The output filename already exists
==================================

Recorder reports collisions rather than silently choosing another base name.
Use timestamped names, a different output directory, or a unique filename. If
restarting from STOPPED with rotation enabled, send
``{"avoid_overwriting_output": true}`` with the stop command when previous
files must be detached from rotation tracking.

Recorder reaches a size or disk limit
=====================================

Check ``max-file-size``, ``max-size``, ``size-tolerance``,
``output.safety-margin``, and free space on the output filesystem. With
``log-rotation: false``, Recorder stops creating files when its managed limit is
exhausted. With rotation enabled, it deletes the oldest file managed by the
current execution. SQL uses one database and makes its file/total limits equal.

Replayer publishes nothing
===========================

* Confirm the input is a finalized ``.mcap`` or ``.db`` file.
* A SQLite input must contain ``data_cdr``; JSON-only databases cannot replay.
* Remove begin/end time, topic, and partition filters to test the complete file.
* Verify the target domain and subscriber QoS. A reliable or durable subscriber
  may need matching Replayer overrides.
* ROS 2 service request/response topics are intentionally blocked.

Playback timing is unexpected
=============================

MCAP playback follows ``logTime``. By default that is Recorder receive time;
with ``log-publish-time: true`` it is the DDS publication timestamp. SQLite
uses ``Messages.log_time``. ``rate`` scales differences between selected
timestamps, while ``start-replay-time`` changes only when the first sample is
sent.

Foxglove cannot decode a channel
================================

Use a finalized MCAP file and verify the channel has a non-empty schema. Record
with ``record-types: true`` and, for ROS 2 data, the correct ``ros2-types``
setting. A raw DDS schema may be valid for DDS Replayer but unsupported by a
particular Foxglove panel. Inspect channel and schema metadata before treating
the payload as corrupt.

Collecting a useful issue report
================================

Include the DDS Record & Replay ``--version`` output, operating system, exact
command, redacted YAML, relevant info-level logs, source and target domains,
file format, and whether the problem reproduces with filters removed. Never
attach a recording that contains sensitive DDS payloads without authorization.
