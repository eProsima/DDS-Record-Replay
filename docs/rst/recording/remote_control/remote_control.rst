.. include:: ../../exports/alias.include
.. include:: ../../exports/roles.include

.. _remote_control:

##############
Remote Control
##############

The |ddsrecorder| application from |eddsrecord| allows remote control and monitoring of the tool via DDS.
Thus it is possible both to monitor the execution status of the |ddsrecorder| and to control the execution status of this application.

Moreover, eProsima provides a remote controlling tool that allows to visualize the status of the |ddsrecorder| and to send commands to it to change its current status.

This section explains the different execution states of the |ddsrecorder|, how to create your own tool using the DDS topics that the application defines to control its behavior, and the presentation of the eProsima user application for the remote control of the |ddsrecorder|.


DDS Recorder Statuses
=====================

The |ddsrecorder| application may have the following states:

* **Closed**: The application is not running.
  To start running the application it is required to launch it from the terminal by executing ``ddsrecorder``.
  Once the ``ddsrecorder`` application is executed, it will automatically go into recording mode, although this can be modified through the `.yaml` configuration file.
  Please refer to the |ddsrecorder| :ref:`remote controller configuration section <usage_configuration_remote_controller>` for more options on the initial state of the application.
* **Running**: The application is running and recording data in the database.
* **Paused**: The application is running but not recording data in the database.
  In this state, the application stores the data it has received in a time window prior to the current time.
  The data will not be saved to the database until an event arrives from the remote controller.
* **Stopped**: The application is running but not receiving data.

To change from one state to another, commands can be sent to the application through the DDS ``ddsrecorder/controller`` topic to be defined later.
The commands that the application accepts are as follows:

* **START**: Changes to the ``Running`` state if it was not in it.
* **PAUSE**: Changes to the ``Pause`` state if it was not in it.
* **STOP**: Changes to the ``Stop`` state if it was not in it.
* **EVENT**: Triggers a recording event to save the data of the time window prior to the event.
  This command can take the next state as an argument, so it is possible to trigger an event and change the state with the same command.
  This is useful when the recorder is in a paused state, the user wants to record all the data collected in the current time window and then immediately switch to the ``Running`` state to start recording data.
  It could also be the case that the user wants to capture the event, save the data and then stop the recorder to inspect the output file.
  The arguments are sent as a serialized `json` in string format.
* **CLOSE**: Closes the |ddsrecorder| application.

The following is the state diagram of the |ddsrecorder| application with all the available commands and the state change effect they cause.

.. figure:: /rst/figures/recorder_state_diagram.png
    :align: center


DDS Controller Data Types
=========================

The |ddsrecorder| contains a DDS subscriber in the ``ddsrecorder/command`` topic and a DDS publisher in the ``ddsrecorder/status`` topic.
Therefore, any user can create his own application to control the |ddsrecorder| remotely by creating a publisher in the ``ddsrecorder/command`` topic, which sends commands to the recorder, and a subscriber in the ``ddsrecorder/status`` topic to monitor its status.

The following is a description of the aforementioned control topics.

* Command topic:

  * Topic name: ``ddsrecorder/command``
  * Topic type name: ``ControllerCommand``
  * Type description:

    * IDL definition

    .. code::

        struct ControllerCommand
        {
            string command;
            string args;
        };

    * ControllerCommand type description:

      .. list-table::
            :header-rows: 1

            *   - Argument
                - Description
                - Data type
                - Possible values
            *   - ``command``
                - Command to send.
                - ``string``
                - ``START`` |br|
                  ``PAUSE`` |br|
                  ``STOP`` |br|
                  ``EVENT`` |br|
                  ``CLOSE``
            *   - ``args``
                - Arguments of the command. This arguments |br|
                  should contain a JSON serialized string.
                - ``string``
                - * ``EVENT`` command: |br|
                    ``{"next_state": "running"}`` |br|
                    ``{"next_state": "stopped"}``

* Status topic:

  * Topic name: ``ddsrecorder/status``
  * Topic type name: ``Status``
  * Type description:

    * IDL definition

    .. code::

        struct Status
        {
            string previous;
            string current;
            string info;
        };

    * Status type description:

      .. list-table::
            :header-rows: 1

            *   - Argument
                - Description
                - Data type
                - Possible values
            *   - ``previous``
                - Previous status of the |ddsrecorder|.
                - ``string``
                - ``Running`` |br|
                  ``Paused`` |br|
                  ``Stopped``
            *   - ``current``
                - Current status of the |ddsrecorder|.
                - ``string``
                - ``Running`` |br|
                  ``Paused`` |br|
                  ``Stopped``
            *   - ``info``
                - Additional information related to the state change. (Unused)
                - ``string``
                - \-


DDS Recorder remote controller application
==========================================

|eddsrecord| provides a graphical user application that implements a remote controller for the |ddsrecorder|.
Thus the user can control the |ddsrecorder| using this application without having to implement their own.


.. note::

    This application is currently under development.
