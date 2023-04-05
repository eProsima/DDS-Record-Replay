.. include:: ../exports/alias.include

.. _notes:

#####
Notes
#####

..
    TODO uncomment when there are forthcoming notes
    .. include:: forthcoming_version.rst

Version v0.1.0
==============

This is the first release of |eddsrecord|.

This release includes several **features** regarding the recording of DDS data, configuration and user interaction.

This release includes the following **Recording features**:

* Supports DynamicTypes.
* Supports saves the data in a MCAP database.
* Supports for ``downsampling`` that reduces the sampling rate of the received data.
* Supports for ``buffer-size`` that indicates the number of samples to be stored in the process memory before the dump to disk.

This release includes the following **User Interface features**:

* :ref:`Recording Service Command-Line Parameters <usage_usage_application_arguments>`.
* :ref:`Remote Control <remote_control>`.

This release includes the following **Configuration features**:

* Support YAML :ref:`configuration file <usage_configuration>`.
* Support for allow and block topic filters at execution time and in run-time.
* Support configuration related to DDS communication.
* Support configuration of data writing in the database.
* Support configuration of the remote controller of the DDS Recorder.
* Support configuration of the internal operation of the DDS Recorder.

This release includes the following **Tutorials**:

* :ref:`Configuring Fast DDS DynamicTypes for data recording <tutorials_basic_example>`.
* :ref:`Visualize recorded data with Foxglove <tutorials_foxglove>`.

This release includes the following **Documentation features**:

* This same documentation.
