.. include:: ../../exports/alias.include

.. _remote_control:

##############
Remote control
##############

The |ddsrecorder| application from |eddsrecord| allows remote control and monitoring of the tool via DDS.
Thus it is possible both to monitor the execution status of the |ddsrecorder| and to control the execution status of this application.

Moreover, eProsima provides a remote controlling tool that allows to visualize the status of the |ddsrecorder| and to send commands to it to change its current status.

This section explains the different execution states of the |ddsrecorder|, how to create your own tool using the DDS topics that the application defines to control its behaviour, and the presentation of the eProsima user application for the remote control of the |ddsrecorder|.

DDS Recorder statuses
=====================

The |ddsrecorder| application may have the following states:

* **Closed**: The application is not running.
  To start running the application it is required to launch it from the terminal by executing ``ddsrecorder``.
  Once the ``ddsrecorder`` application is executed, it will automatically go into recording mode, although this can be modified through the `.yaml` configuration file.
  Please refer to the |ddsrecorder| :ref:`remote controller configuration section <usage_configuration_remote_controller>` for more options on the initial state of the application.
* **Running**: The application is running and recording data in the database.
* **Paused**: The application is running but not recording data in the database.
  In this state, the application stores the data it has received in a time window prior to the current time.
  The data will not be saved to the database until a EVENT event arrives from the remote controller.
* **Stopped**: The application is running but not receiving data.

To change from one state to the other, commands can be sent to the application through the DDS ``ddsrecorder/controller`` topic to be defined later.
The commands that the application accepts are as follows:

* **START**: Changes to the Running state if it was not in it.
* **PAUSE**: Changes to the Pause state if it was not in it.
* **STOP**: Changes to the Stop state if it was not in it.
* **EVENT**: Triggers a recording event to save the data of the time window prior to the event.
* **CLOSE**: Closes the |ddsrecorder| application.

The following is the state diagram of the |ddsrecorder| application with all available commands and the state change effect they cause.

.. figure:: /rst/figures/recorder_state_diagram.png
    :align: center


