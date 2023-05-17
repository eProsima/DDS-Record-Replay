.. include:: ../../exports/alias.include

.. _recorder_getting_started_project_overview:

################
Project Overview
################

|eddsrecord| is a cross-platform application developed by eProsima and powered by *Fast DDS* that contains a set of tools for debugging DDS networks.
Among these tools is a recording application, called |ddsrecorder|, which allows a user to capture data published in a DDS environment for later analysis or playback.

The |ddsrecorder| application automatically discovers all topics in the DDS network and saves the data published in each topic with the publication timestamp of the data.
Furthermore, by using the `DynamicTypes <https://fast-dds.docs.eprosima.com/en/latest/fastdds/dynamic_types/dynamic_types.html>`_ feature of *Fast DDS*, it is possible to record the type of the data in the MCAP file.
The benefit of this comes from the fact that the data is saved serialized according to the CDR format.
The registration of the data type in the file allows the reading of the data (deserialization) when loading the file.

Since the |ddsrecorder| needs to record the data types of the data to be saved in the database, the user application must communicate the data types with which it is working by means of the type lookup service.
This can be easily achieved by applying the configuration described in :ref:`this section <recorder_usage_fastdds_configuration>`.

Moreover, |ddsrecorder| is designed to ensure that internal communications are handled efficiently, from the reception of the data to its storage in the output database.
This is achieved through the internal implementation of a zero-copy communication mechanism implemented in one of the |ddsrecorder| base libraries.
It is also possible to configure the number of threads that execute these data reception and saving tasks, as well as the size of the internal buffers to avoid writing to disk with each received data.

Usage Description
=================

|ddsrecorder| is a terminal (non-graphical) application that creates a recording service as long as it is running.
Although most use cases are covered by the default configuration, the |ddsrecorder| can be configured via a YAML file,
whose format is very intuitive and human-readable.

* **Run**:
  Only the command that launches the application (``ddsrecorder``) needs to be executed to run a |ddsrecorder|.
  Please, read this :ref:`section <recorder_usage_configuration>` to apply a specific configuration, and this :ref:`section <recorder_usage_usage_application_arguments>` to see the supported arguments.
* **Interact**:
  Once the |ddsrecorder| application is running, the allowlist and blocklist topic lists could be changed in runtime by just changing the YAML configuration file.
  It is also possible to change the status of the recorder (``RUNNING``, ``PAUSED``, ``STOPPED`` or ``CLOSED``) by remote control of the application.
  This remote control is done by sending commands via DDS or by using the graphical remote control application provided with the |eddsrecord| software tool (see :ref:`Remote control <recorder_remote_control>`).
* **Close**:
  To close the |ddsrecorder| application just send a `Ctrl+C` signal to terminate the process gracefully (see :ref:`Closing Recording Application <recorder_usage_close_recorder>`) or close it remotely using the remote control application (see :ref:`Remote control <recorder_remote_control>`).

Common Use cases
================

To get started with |ddsrecorder|, please visit section :ref:`recorder_getting_started_usage_example`.
In addition, this documentation provides several tutorials on how to set up a |ddsrecorder|, a comprehensive Fast DDS application using DynamicTypes and how to read the generated MCAP file.
