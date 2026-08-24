.. include:: ../../exports/alias.include

.. _replayer_usage_mcap_convert:

MCAP-to-SQL convert
###################

``mcap-convert`` reads an MCAP recording and creates a DDS Record & Replay
SQLite database. It does not publish DDS traffic.

Basic conversion
================

.. code-block:: console

   $ mcap-convert --input-file ./recording.mcap

The default output is ``./recording.db``. Set another path with
``--sql-output``; ``.db`` is appended when the value has no extension:

.. code-block:: console

   $ mcap-convert -i ./recording.mcap --sql-output ./converted/session.db

The converter preserves message timestamps and MCAP file order. It always
stores CDR and attempts to add JSON when recorded dynamic type information can
decode the payload. Missing type information therefore affects JSON and key
extraction, not the CDR copy.

Configuration subset
====================

``--config-path`` accepts the Replayer YAML shape, but conversion uses only:

* ``dds.partitions`` to select recorded writer partitions;
* ``replayer.begin-time`` and ``replayer.end-time`` to select log times; and
* ``specs.logging`` for log output.

Replayer input, domain, allow/block lists, rate, start time, replay-types, QoS,
transports, acknowledgement timeout, and worker count do not affect conversion.
The MCAP input must always be passed with ``--input-file``.

.. literalinclude:: ../../examples/converter_selection.yaml
   :language: yaml
   :linenos:

Batch size
==========

``--sql-batch-size`` controls how many messages are hydrated and committed per
batch. The default is ``4096``; the accepted range is ``1`` through
``160000001``. Larger batches may improve throughput but use more memory.

Command-line reference
======================

.. list-table::
   :header-rows: 1
   :widths: 29 27 44

   * - Option
     - Value/default
     - Effect
   * - ``-h``, ``--help``
     - None
     - Print usage and exit.
   * - ``-v``, ``--version``
     - None
     - Print version and commit hash, then exit.
   * - ``-i``, ``--input-file``
     - Readable MCAP; required
     - Source recording.
   * - ``-c``, ``--config-path``
     - Readable YAML path
     - Apply the supported selection/logging subset above.
   * - ``--sql-output``
     - Path; input stem plus ``.db``
     - Destination database. Prefer a ``.db`` extension for Replayer detection.
   * - ``--sql-batch-size``
     - Integer; ``4096``
     - Conversion batch size.
   * - ``-d``, ``--debug``
     - None
     - Enable informational logging. Do not combine with explicit logging
       options.
   * - ``--log-filter``
     - Regex; ``DDSREPLAYER``
     - Filter informational and warning categories.
   * - ``--log-verbosity``
     - ``info``, ``warning``, ``error``; ``warning``
     - Minimum displayed severity.

The destination uses ``.db.tmp~`` until the conversion completes. Inspect or
replay only the final database.
