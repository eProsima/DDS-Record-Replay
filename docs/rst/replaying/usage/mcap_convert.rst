.. include:: ../../exports/alias.include
.. include:: ../../exports/roles.include

.. _replayer_usage_mcap_convert:

############
MCAP Convert
############

``mcap-convert`` is a standalone command-line tool that converts an MCAP recording into the SQLite
``.db`` format used by DDS Record & Replay.

Unlike |ddsreplayer|, this tool does not publish data back into a DDS domain.
Instead, it reads an existing MCAP file and generates a SQLite output file for workflows that
require SQL output.

Using MCAP Convert
==================

After installing the standalone conversion tool, source the installation environment and run:

.. code-block:: bash

    source install/setup.bash
    mcap-convert -i /path/to/recording.mcap

If ``--sql-output`` is not provided, the tool writes the output next to the input file using the
same base name and the ``.db`` extension.

To write the converted output to a specific location, pass ``--sql-output``:

.. code-block:: bash

    source install/setup.bash
    mcap-convert -i /path/to/recording.mcap --sql-output /path/to/recording.db

If the value given to ``--sql-output`` has no extension, ``.db`` is appended automatically.

Optional Configuration File
===========================

The converter accepts an optional YAML configuration file through ``--config-path``.

This file uses the same structure as the |ddsreplayer| configuration file.
See :ref:`Replay configuration <replayer_usage_configuration>` for the available settings.

If type information is not available for a topic in the input MCAP file, the converter still stores
the CDR payload in the SQLite output, but deserialized type data cannot be generated for that topic.

MCAP Convert Command-Line Parameters
====================================

The ``mcap-convert`` application supports the following input arguments:

.. list-table::
    :header-rows: 1

    *   - Command
        - Description
        - Option
        - Possible Values
        - Default Value

    *   - Help
        - It shows the usage information |br|
          of the application.
        - ``-h`` |br|
          ``--help``
        -
        -

    *   - Version
        - It shows the current version |br|
          of DDS Record & Replay and the |br|
          hash of the last commit of |br|
          the compiled code.
        - ``-v`` |br|
          ``--version``
        -
        -

    *   - Input File
        - Input MCAP file path.
        - ``-i`` |br|
          ``--input-file``
        - Readable file path
        - Required

    *   - Configuration File
        - Optional YAML configuration |br|
          file path.
        - ``-c`` |br|
          ``--config-path``
        - Readable file path
        - -

    *   - SQL Output
        - Output SQLite file path. If the |br|
          path has no extension, ``.db`` |br|
          is appended automatically.
        - ``--sql-output``
        - File path
        - Input file path with ``.db`` |br|
          extension

    *   - Debug
        - Enables the converter logs so the |br|
          execution can be followed by |br|
          internal debugging information. |br|
          Sets ``Log Verbosity`` to ``info`` |br|
          and ``Log Filter`` to ``DDSREPLAYER``.
        - ``-d`` |br|
          ``--debug``
        -
        -

    *   - Log Verbosity
        - Set the verbosity level so |br|
          only log messages with equal |br|
          or higher importance level |br|
          are shown.
        - ``--log-verbosity``
        - ``info`` |br|
          ``warning`` |br|
          ``error``
        - ``warning``

    *   - Log Filter
        - Set a regex string as filter.
        - ``--log-filter``
        - String
        - ``"DDSREPLAYER"``
