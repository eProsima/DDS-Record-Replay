.. include:: ../../exports/alias.include
.. include:: ../../exports/roles.include

.. _user_interface_usage:

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

|eddsrecorder| depends on |fastdds| ``fastrtps``, ``fastcdr`` and ``ddsrecorder`` libraries.
In order to correctly execute the Recorder, make sure that ``fastrtps``, ``fastcdr`` and ``ddsrecorder`` are properly sourced.

.. code-block:: bash

    source <path-to-fastdds-installation>/install/setup.bash
    source <path-to-ddsrecorder-installation>/install/setup.bash

.. note::

    If Fast DDS and DDS Recorder have been installed in the system, these libraries would be sourced by default.

To start |eddsrecorder| with a default configuration, enter:

.. code-block:: bash

    ddsrecorder --config-path /ddsrecorder/resources/configurations/conf-ddsrecorder.yaml

Stopping Recording Application
------------------------------

SIGINT
^^^^^^

To stop |eddsrecorder|, press ``Ctrl-c``. |eddsrecorder| will perform a clean shutdown.

SIGTERM
^^^^^^^

Write command ``kill <pid>`` in a different terminal, where ``<pid>`` is the id of the process running the DDS Router. Use ``ps`` or ``top`` programs to check the process ids.

TIMEOUT
^^^^^^^

Setting a maximum amount of seconds that the application will work using argument ``--timeout`` will close the application once the time has expired.

.. _user_interface_usage_application_arguments:

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

    *   - Help Argument
        - It shows the usage information of the application.
        - ``-h``, ``--help``
        -
        -

    *   - Version Argument
        - It shows the current version of the DDS Recorder and |br|
          the hash of the last commit of the compiled code.
        - ``-v``, ``--version``
        -
        -


    *   - Configuration File Argument
        - Configuration file path.
        - ``-c``, ``--config-path``
        -
        - ``./DDS_RECORDER_CONFIGURATION.yaml``

    *   - Reload Timer
        - The configuration file will be automatically reloaded |br|
          according to the specified time period.
        - ``-r``, ``--reload-time``
        - Unsigned Integer
        - ``0``

    *   - Timeout Argument
        - Set a maximum time while the application will be running. |br|
          `0`` means that the application will run forever |br|
          (until kill via signal).
        - ``-r``, ``--reload-time``
        - Unsigned Integer
        - ``0``

    *   - Debug Argument
        - Enables the |ddsrecorder| logs so the execution can be |br|
          followed by internal debugging information. Sets |br|
          ``Log Verbosity Argument`` to ``info`` and |br|
          ``Log Filter Argument`` to ``DDSRECORDER``.
        - ``-d``, ``--debug``
        -
        -

    *   - Log Verbosity Argument
        - Set the verbosity level so only log messages with equal |br|
          or higher importance level are shown.
        - ``--log-verbosity``
        - ``info`` ``warning`` ``error``
        - ``warning``

    *   - Log Filter Argument
        - Set a regex string as filter.
        - ``--log-filter``
        - String
        - ``"DDSRecorder"``
