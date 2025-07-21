.. include:: ../../exports/alias.include
.. include:: ../../exports/roles.include

.. _recorder_usage_usage:

#####
Usage
#####

|eddsrecorder| is a user application executed from command line.


Starting Recording Application
------------------------------

Docker Image
^^^^^^^^^^^^

The recommended method to run the |ddsrecorder| is to instantiate a Docker container of the |ddsrecord| image.
:ref:`Here <docker>` are the instructions to download the compressed |ddsrecord| Docker image and load it locally.

To run the |ddsrecorder| from a Docker container execute the following command:

.. code-block:: bash

    docker run -it \
        --net=host \
        --ipc=host \
        -v /<dds_recorder_ws>/DDS_RECORDER_CONFIGURATION.yaml:/root/DDS_RECORDER_CONFIGURATION.yaml \
        ubuntu-ddsrecorder:v<X.X.X> ddsrecorder


Installation from sources
^^^^^^^^^^^^^^^^^^^^^^^^^

|eddsrecord| depends on ``fastdds``, ``fastcdr`` and ``ddspipe`` libraries.
In order to correctly execute the recorder, make sure that ``fastdds``, ``fastcdr`` and ``ddspipe`` are properly sourced.

.. code-block:: bash

    source <path-to-fastdds-installation>/install/setup.bash
    source <path-to-ddspipe-installation>/install/setup.bash
    source <path-to-ddsrecordreplay-installation>/install/setup.bash

.. note::

    If Fast DDS, DDS Pipe and DDS Record & Replay have been installed in the system, these libraries would be sourced by default.

To start |eddsrecorder| with a default configuration, enter:

.. code-block:: bash

    ddsrecorder


.. _recorder_usage_close_recorder:

Closing Recording Application
------------------------------

SIGINT
^^^^^^

To close |eddsrecorder|, press ``Ctrl+C``. |ddsrecorder| will perform a clean shutdown.

SIGTERM
^^^^^^^

Write command ``kill <pid>`` in a different terminal, where ``<pid>`` is the id of the process running the |ddsrecorder|.
Use ``ps`` or ``top`` programs to check the process ids.

TIMEOUT
^^^^^^^

Setting a maximum amount of seconds that the application will work using argument ``--timeout`` will close the application once the time has expired.

.. _recorder_usage_usage_application_arguments:

Recording Service Command-Line Parameters
-----------------------------------------

The |ddsrecorder| application supports several input arguments:

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
          of the |ddsrecorder| and the |br|
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
          ``0`` means that the application |br|
          will run forever (until kill |br|
          via signal).
        - ``-t`` |br|
          ``--timeout``
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
        - ``"DDSRECORDER"``
