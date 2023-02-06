.. include:: ../../exports/alias.include

.. _user_manual_user_interface:

#####
Usage
#####

|eddsrecorder| is a user application executed from command line.

.. contents::
    :local:
    :backlinks: none
    :depth: 1

Starting Recording Application
---------------------------

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

To stop |eddsrecorder|, press Ctrl-c. |eddsrecorder| will perform a clean shutdown.

.. _user_manual_user_interface_application_arguments:

Recording Service Command-Line Parameters
-----------------------------------------

The |ddsrecorder| application supports several input arguments:

.. list-table::
    :header-rows: 1

    *   - Command
        - Option
        - Long option
        - Value
        - Default Value
        - Description

    *   - Help Argument
        - ``-h``
        - ``--help``
        -
        -
        - It shows the usage information of the application.

    *   - Version Argument
        - ``-v``
        - ``--version``
        -
        -
        - It shows the current version of the DDS Recorder and the hash of the last commit of the compiled code.


    *   - Configuration File Argument
        - ``-c``
        - ``--config-path``
        - Readable File Path
        - ``./DDS_RECORDER_CONFIGURATION.yaml``
        - Configuration file path.

    *   - Reload Timer
        - ``-r``
        - ``--reload-time``
        - Unsigned Integer
        - ``0``
        - The configuration file will be automatically reloaded according to the specified time period.

    *   - Timeout Argument
        - ``-r``
        - ``--reload-time``
        - Unsigned Integer
        - ``0``
        - Set a maximum time while the application will be running. `0`` means that the application will run forever (until kill via signal).

    *   - Debug Argument
        - ``-d``
        - ``--debug``
        -
        -
        - Enables the |ddsrecorder| logs so the execution can be followed by internal debugging information. Sets ``Log Verbosity Argument`` to ``info`` and
            ``Log Filter Argument`` to ``DDSRECORDER``.

    *   - Log Verbosity Argument
        -
        - ``--log-verbosity``
        - ``info`` ``warning`` ``error``
        - ``warning``
        - Set the verbosity level so only log messages with equal or higher importance level are shown.

    *   - Log Filter Argument
        -
        - ``--log-filter``
        - String
        - ``"DDSRecorder"``
        - Set a regex string as filter.
