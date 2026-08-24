.. include:: ../../exports/alias.include

.. _replayer_usage_usage:

##################
Using DDS Replayer
##################

Replayer requires a readable MCAP or DDS Record & Replay SQLite ``.db`` file.
The shortest invocation is:

.. code-block:: console

   $ ddsreplayer --input ./recording.mcap

The input can instead be set in YAML. If both are supplied, the CLI path wins:

.. code-block:: console

   $ ddsreplayer --config-path ./replayer.yaml --input ./override.db

Without ``--config-path``, ``./DDS_REPLAYER_CONFIGURATION.yaml`` is loaded only
when it exists. An input file remains mandatory. Replayer publishes selected
samples according to their recorded timing and exits after the last sample.

Useful invocations
==================

.. code-block:: console

   # Replay on another domain without editing YAML.
   $ ddsreplayer -i recording.mcap --domain 7

   # Reload topic and partition selection from a symlink every five seconds.
   $ ddsreplayer -i recording.mcap -c current.yaml --reload-time 5

See :ref:`replayer_usage_configuration` for playback timing and
:ref:`common_dds_configuration` for filtering and QoS.

.. _replayer_usage_close_replayer:

Ending playback
===============

Normal playback ends after the last selected message and waits for queued work
to complete. Press :kbd:`Ctrl+C` or send ``SIGTERM`` to stop early. For reliable
writers, ``specs.wait-all-acked-timeout`` controls the acknowledgement wait when
an endpoint closes.

.. _replayer_usage_usage_application_arguments:

Command-line reference
======================

.. list-table::
   :header-rows: 1
   :widths: 27 27 46

   * - Option
     - Value/default
     - Effect
   * - ``-h``, ``--help``
     - None
     - Print usage and exit.
   * - ``-v``, ``--version``
     - None
     - Print version and commit hash, then exit.
   * - ``-i``, ``--input``
     - Readable ``.mcap`` or ``.db`` path
     - Input recording. Required by CLI or YAML.
   * - ``-c``, ``--config-path``
     - Readable YAML path
     - Load this configuration instead of the optional conventional filename.
   * - ``-r``, ``--reload-time``
     - Seconds ≥ 0; ``0``
     - Poll the loaded configuration at this interval.
   * - ``--domain``
     - ``0``–``232``; ``0``
     - Override the replay DDS domain.
   * - ``-d``, ``--debug``
     - None
     - Set informational logging with the Replayer category filter. Do not
       combine with the two explicit logging options.
   * - ``--log-filter``
     - Regex; ``DDSREPLAYER``
     - Filter informational and warning log categories.
   * - ``--log-verbosity``
     - ``info``, ``warning``, ``error``; ``warning``
     - Minimum displayed severity, case-insensitive.
