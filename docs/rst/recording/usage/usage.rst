.. include:: ../../exports/alias.include
.. include:: ../../exports/roles.include

.. _usage_usage:

#####
Usage
#####

|eddsrecorder| is a user application executed from command line.

.. contents::
    :local:
    :backlinks: none
    :depth: 1

Starting Recording Application
------------------------------

|eddsrecorder| depends on ``fastrtps``, ``fastcdr`` and ``ddsrouter`` libraries.
In order to correctly execute the recorder, make sure that ``fastrtps``, ``fastcdr`` and ``ddsrouter`` are properly sourced.

.. code-block:: bash

    source <path-to-fastdds-installation>/install/setup.bash
    source <path-to-ddsrouter-installation>/install/setup.bash
    source <path-to-ddsrecorder-installation>/install/setup.bash

.. note::

    If Fast DDS, DDS Router and DDS Recorder have been installed in the system, these libraries would be sourced by default.

To start |eddsrecorder| with a default configuration, enter:

.. code-block:: bash

    ddsrecorder

Stopping Recording Application
------------------------------

SIGINT
^^^^^^

To stop |eddsrecorder|, press ``Ctrl+C``. |eddsrecorder| will perform a clean shutdown.

SIGTERM
^^^^^^^

Write command ``kill <pid>`` in a different terminal, where ``<pid>`` is the id of the process running the DDS Router. Use ``ps`` or ``top`` programs to check the process ids.

TIMEOUT
^^^^^^^

Setting a maximum amount of seconds that the application will work using argument ``--timeout`` will close the application once the time has expired.

.. _usage_usage_application_arguments:

Recording Service Command-Line Parameters
-----------------------------------------

The |ddsrecorder| application supports several input arguments:

.. list-table::
    :header-rows: 1

    *   - Command
        - Description
        - Option
        - Posible Values
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
          of the DDS Recorder and the |br|
          hash of the last commit of |br|
          the compiled code.
        - ``-v`` |br|
          ``--version``
        -
        -


    *   - Configuration File
        - Configuration file path.
        - ``-c`` |br|
          ``--config-path``
        -
        - ``./DDS_RECORDER_CONFIGURATION.yaml``

    *   - Reload Timer
        - The configuration file will be |br|
          automatically reloaded |br|
          according to the specified |br|
          time period.
        - ``-r`` |br|
          ``--reload-time``
        - Unsigned Integer
        - ``0``

    *   - Timeout
        - Set a maximum time while the |br|
          application will be running. |br|
          `0`` means that the application |br|
          will run forever (until kill |br|
          via signal).
        - ``-r`` |br|
          ``--reload-time``
        - Unsigned Integer
        - ``0``

    *   - Debug
        - Enables the |ddsrecorder| |br|
          logs so the execution can be |br|
          followed by internal |br|
          debugging information. |br|
          Sets ``Log Verbosity`` |br|
          to ``info`` and |br|
          ``Log Filter`` |br|
          to ``DDSRECORDER``.
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
        - ``"DDSRecorder"``