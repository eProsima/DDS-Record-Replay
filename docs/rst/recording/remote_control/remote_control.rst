.. include:: ../../exports/alias.include
.. include:: ../../exports/roles.include

.. _recorder_remote_control:

##############
Remote Control
##############

The |ddsrecorder| application from |eddsrecord| allows remote control and monitoring of the tool via DDS.
Thus it is possible both to monitor the execution status of the |ddsrecorder| and to control the execution status of this application.

Moreover, eProsima provides a remote controlling tool that allows to visualize the status of a |ddsrecorder| and to send commands to it to change its current status.

This section explains the different execution states of a |ddsrecorder|, how to create your own tool using the DDS topics that the application defines to control its behavior, and the presentation of the eProsima user application for the remote control of the |ddsrecorder|.

DDS Recorder Statuses
=====================

The |ddsrecorder| application may have the following states:

* **CLOSED**: The application is not running.
  To start running the application it is required to launch it from the terminal by executing ``ddsrecorder``.
  Once the ``ddsrecorder`` application is executed, it will automatically go into recording mode (``RUNNING`` state), although this can be modified through the `.yaml` configuration file.
  Please refer to the |ddsrecorder| :ref:`remote controller configuration section <recorder_usage_configuration_remote_controller>` for more options on the initial state of the application.
* **RUNNING**: The application is running and recording data in the database.
* **PAUSED**: The application is running but not recording data in the database.
  In this state, the application stores the data it has received in a time window prior to the current time.
  The data will not be saved to the database until an event arrives from the remote controller.
* **SUSPENDED**: The application is running but not recording data. Internal entities are created and samples received but discarded (advantage: lower latency in transition to ``RUNNING/PAUSED`` states).
* **STOPPED**: The application is running but not recording data. Internal entities are not created and thus no samples are received.

To change from one state to another, commands can be sent to the application through the `Controller Command` DDS topic to be defined later.
The commands that the application accepts are as follows:

* **start**: Changes to ``RUNNING`` state if it was not in it.
* **pause**: Changes to ``PAUSED`` state if it was not in it.
* **event**: Triggers a recording event to save the data of the time window prior to the event.
  This command can take the next state as an argument, so it is possible to trigger an event and change the state with the same command.
  This is useful when the recorder is in a paused state, the user wants to record all the data collected in the current time window and then immediately switch to ``RUNNING`` state to start recording data.
  It could also be the case that the user wants to capture the event, save the data and then stop the recorder to inspect the output file.
  The arguments are sent as a serialized `json` in string format.
* **suspend**: Changes to ``SUSPENDED`` state if it was not in it.
* **stop**: Changes to ``STOPPED`` state if it was not in it.
* **close**: Closes the |ddsrecorder| application.

The following is the state diagram of the |ddsrecorder| application with all the available commands and the state change effect they cause.

.. figure:: /rst/figures/recorder_state_diagram.png
    :align: center


DDS Controller Data Types
=========================

The |ddsrecorder| contains a DDS subscriber in the `Controller Command` topic and a DDS publisher in the `Controller Status` topic.
These topics' names are by default ``/ddsrecorder/command`` and ``/ddsrecorder/status``, respectively, but can also be specified by users via the ``command-topic-name`` and ``status-topic-name`` configuration tags.
Therefore, any user can create his own application to control the |ddsrecorder| remotely by creating a publisher in the `Controller Command` topic, which sends commands to the recorder, and a subscriber in the `Controller Status` topic to monitor its status.

.. note::

    Status and command topics are not blocked by default, i.e. messages on this topics will be recorded if listening on the same domain the controller is launched.
    If willing to avoid this, include these topics in the :ref:`blocklist <recorder_topic_filtering>`:

    .. code-block:: yaml

      dds:
        blocklist:
          - type: DdsRecorderStatus
          - type: DdsRecorderCommand

The following is a description of the aforementioned control topics.

* Command topic:

  * Topic name: Specified in ``command-topic-name`` configuration parameter (Default: ``/ddsrecorder/command``)
  * Topic type name: ``DdsRecorderCommand``
  * Type description:

    * IDL definition

    .. code::

        struct DdsRecorderCommand
        {
            string command;
            string args;
        };

    * DdsRecorderCommand type description:

      .. list-table::
            :header-rows: 1

            *   - Argument
                - Description
                - Data type
                - Possible values
            *   - ``command``
                - Command to send.
                - ``string``
                - ``start`` |br|
                  ``pause`` |br|
                  ``event`` |br|
                  ``suspend`` |br|
                  ``stop`` |br|
                  ``close``
            *   - ``args``
                - Arguments of the command. This arguments |br|
                  should contain a JSON serialized string.
                - ``string``
                - * ``event`` command: |br|
                    ``{"next_state": "RUNNING"}`` |br|
                    ``{"next_state": "SUSPENDED"}`` |br|
                    ``{"next_state": "STOPPED"}``

* Status topic:

  * Topic name: Specified in ``status-topic-name`` configuration parameter (Default: ``/ddsrecorder/status``)
  * Topic type name: ``DdsRecorderStatus``
  * Type description:

    * IDL definition

    .. code::

        struct DdsRecorderStatus
        {
            string previous;
            string current;
            string info;
        };

    * DdsRecorderStatus type description:

      .. list-table::
            :header-rows: 1

            *   - Argument
                - Description
                - Data type
                - Possible values
            *   - ``previous``
                - Previous status of the |ddsrecorder|.
                - ``string``
                - ``RUNNING`` |br|
                  ``PAUSED`` |br|
                  ``SUSPENDED`` |br|
                  ``STOPPED``
            *   - ``current``
                - Current status of the |ddsrecorder|.
                - ``string``
                - ``RUNNING`` |br|
                  ``PAUSED`` |br|
                  ``SUSPENDED`` |br|
                  ``STOPPED``
            *   - ``info``
                - Additional information related to the state change. (Unused)
                - ``string``
                - \-

.. _recorder_remote_controller:

DDS Recorder remote controller application
==========================================

|eddsrecord| provides a graphical user application that implements a remote controller for the |ddsrecorder|.
Thus the user can control a |ddsrecorder| instance using this application without having to implement their own.

.. note::

    If installing |eddsrecord| from sources, compilation flag ``-DBUILD_DDSRECORDER_CONTROLLER=ON`` is required to build this application.

Its interface is quite simple and intuitive.
Once the application is launched, a layout as the following one should be visible:

.. figure:: /rst/figures/controller_init.png
    :align: center

If the controller should function in a domain different than the default one (``0``), change it by clicking ``File->DDS Domain`` and introducing the one desired:

.. figure:: /rst/figures/controller_domain.png
    :align: center

It is also possible to use non-default status and command topic names through the ``File->DDS Topics`` dialog.

When a |ddsrecorder| instance is found in the domain, a message is displayed in the logging panel:

.. figure:: /rst/figures/controller_found.png
    :align: center

From this point on, it is possible to interact with the recorder application by pushing any of the buttons appearing on the left.
Every command sent is reflected in the logging panel and, additionally, the recorder application publishes its current status with every state transition undergone.
This can be observed in the `eProsima DDS Recorder status` placeholder, located in the upper part of the layout:

.. figure:: /rst/figures/controller_interact.png
    :align: center

By clicking on ``Suspend`` / ``Stop`` button, the recorder application ceases recording, but can be commanded to ``Start`` / ``Pause`` whenever wished.
Once the user has finished all recording activity, it is possible to ``Close`` the recorder and free all resources used by the application:

.. figure:: /rst/figures/controller_close.png
    :align: center

Note that once ``CLOSED`` state has been reached, commands will no longer have an effect on the recorder application as its process is terminated when a ``close`` command is received.
