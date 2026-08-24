.. include:: ../exports/alias.include

.. _record_replay_quickstart:

Record and replay in five minutes
#################################

This walkthrough records ShapesDemo traffic into MCAP and replays the same
samples to a new subscriber. It uses an explicit configuration so an unrelated
``DDS_RECORDER_CONFIGURATION.yaml`` in the current directory cannot change the
result.

Prerequisites
=============

Install DDS Record & Replay using the :ref:`installation_manual_linux` or
:ref:`installation_manual_windows` guide and install
`eProsima ShapesDemo <https://eprosima-shapes-demo.readthedocs.io/en/stable/>`__.
Verify the commands are available:

.. parsed-literal::

   $ ddsrecorder --version
   DDS Record & Replay v|release|
   $ ddsreplayer --version
   DDS Record & Replay v|release|

The exact version output also contains the build commit hash.

1. Configure the recording
===========================

Save the following file as ``recorder.yaml`` in an empty working directory:

.. literalinclude:: ../examples/recorder_quickstart.yaml
   :language: yaml
   :linenos:

This selects DDS domain ``0`` and names the output ``shapesdemo``. Recorder
prepends the start timestamp, so the completed file is named similarly to
``2026-08-24_14-30-00_CEST_shapesdemo.mcap``.

2. Publish ShapesDemo data
==========================

Start ShapesDemo, create a publisher for the ``Square`` topic on domain ``0``,
and leave it publishing with its default settings.

3. Record and finalize the file
===============================

Run Recorder from the working directory:

.. code-block:: console

   $ ddsrecorder --config-path recorder.yaml

Move the square for a few seconds, then press :kbd:`Ctrl+C`. A file being
written has the suffix ``.mcap.tmp~``. Graceful shutdown closes the MCAP index
and renames it to the final ``.mcap`` name. Always use the final file for replay
or inspection.

4. Replay the recording
========================

Stop the ShapesDemo publisher and create a ``Square`` subscriber on domain
``0``. Pass the actual timestamped filename produced in the previous step:

.. code-block:: console

   $ ddsreplayer --input ./2026-08-24_14-30-00_CEST_shapesdemo.mcap

The subscriber displays the recorded movement. Replayer exits successfully
after publishing the last selected sample. Press :kbd:`Ctrl+C` to end it early.

Use SQLite instead
==================

To record SQLite, save this configuration as ``recorder-sql.yaml``:

.. literalinclude:: ../examples/recorder_sql.yaml
   :language: yaml
   :linenos:

Run ``ddsrecorder -c recorder-sql.yaml`` and replay the resulting ``.db`` file
with ``ddsreplayer -i <file>.db``. ``data-format: both`` stores CDR for replay
and JSON for SQL inspection. A JSON-only database is useful for queries but
cannot be replayed because it does not contain the serialized DDS payload.

Open the recording in Foxglove
================================

For a schema-backed MCAP intended for Foxglove, use this stricter Recorder
variant:

.. literalinclude:: ../examples/recorder_foxglove.yaml
   :language: yaml
   :linenos:

It waits for dynamic type information before storing samples. Finalize the
file with :kbd:`Ctrl+C`, then open the final ``.mcap`` rather than ``.tmp~``.
See :ref:`tutorials_foxglove` for the current local-file workflow and ROS 2
schema guidance.

Configuration precedence
========================

Values passed on the command line override YAML; YAML overrides built-in
defaults. Without ``--config-path``, Recorder loads
``./DDS_RECORDER_CONFIGURATION.yaml`` only when that file exists. Otherwise it
uses built-in defaults. Replayer behaves the same way with
``./DDS_REPLAYER_CONFIGURATION.yaml``, but an input file is still required by
CLI or YAML.

Continue with :ref:`recorder_usage_usage`, :ref:`replayer_usage_usage`, or the
:ref:`common_dds_configuration` reference.
