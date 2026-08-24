.. include:: ../../exports/alias.include

.. _recorder_usage_usage:

##################
Using DDS Recorder
##################

Recorder begins in ``RUNNING`` state by default, discovers DDS writers, and
writes eligible samples to a timestamped MCAP file in the current directory.
Use an explicit configuration for repeatable deployments:

.. code-block:: console

   $ ddsrecorder --config-path ./recorder.yaml

Without ``--config-path``, ``./DDS_RECORDER_CONFIGURATION.yaml`` is loaded only
when it exists. Otherwise the built-in configuration is used. ``--domain`` and
CLI logging values override their YAML equivalents.

Useful invocations
==================

.. code-block:: console

   # Record every discovered topic on domain 7.
   $ ddsrecorder --domain 7

   # Stop gracefully after 60 seconds.
   $ ddsrecorder -c recorder.yaml --timeout 60

   # Poll a symlinked configuration every five seconds.
   $ ddsrecorder -c current.yaml --reload-time 5

See :ref:`common_dds_configuration` for the live-reload matrix and
:ref:`recorder_usage_configuration` for every YAML field.

.. _recorder_usage_close_recorder:

Closing Recorder
================

Press :kbd:`Ctrl+C`, send ``SIGTERM``, or issue the remote ``close`` command.
A graceful close flushes buffered data, closes each active writer, and renames
``.mcap.tmp~`` or ``.db.tmp~`` to its final filename. Do not inspect or replay a
temporary file as though it were complete.

``SUSPEND`` and ``STOP`` also finalize the active file without terminating the
process. See :ref:`recorder_remote_control` for the difference between states.

.. _recorder_usage_usage_application_arguments:

Command-line reference
======================

.. list-table::
   :header-rows: 1
   :widths: 25 28 47

   * - Option
     - Value/default
     - Effect
   * - ``-h``, ``--help``
     - None
     - Print usage and exit.
   * - ``-v``, ``--version``
     - None
     - Print version and commit hash, then exit.
   * - ``-c``, ``--config-path``
     - Readable YAML path
     - Load this configuration instead of the optional conventional filename.
   * - ``-r``, ``--reload-time``
     - Seconds ≥ 0; ``0``
     - Poll the loaded configuration at this interval. Intended for files
       that cannot be watched directly.
   * - ``-t``, ``--timeout``
     - Seconds ≥ 0; ``0``
     - Maximum run time. ``0`` is unlimited.
   * - ``--domain``
     - ``0``–``232``; ``0``
     - Override the recording DDS domain.
   * - ``-d``, ``--debug``
     - None
     - Set informational logging with the Recorder category filter. Do not
       combine with the two explicit logging options.
   * - ``--log-filter``
     - Regex; ``DDSRECORDER``
     - Filter informational and warning log categories.
   * - ``--log-verbosity``
     - ``info``, ``warning``, ``error``; ``warning``
     - Minimum displayed severity, case-insensitive.
