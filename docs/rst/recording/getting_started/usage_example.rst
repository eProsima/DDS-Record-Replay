.. include:: ../../exports/alias.include

.. _recorder_getting_started_usage_example:

################
Example of usage
################

This example will serve as a hands-on tutorial, aimed at introducing some of the key concepts and features that
|eddsrecord| recording application (|ddsrecorder| or ``ddsrecorder``) has to offer.

Prerequisites
=============

It is required to have |eddsrecord| previously installed using one of the following installation methods:

* :ref:`installation_manual_windows`
* :ref:`installation_manual_linux`
* :ref:`docker`

Additionally, `ShapesDemo <https://www.eprosima.com/index.php/products-all/eprosima-shapes-demo>`_ is required to publish and subscribe shapes of different colors and sizes.
ShapesDemo application is already prepared to use Fast DDS DynamicTypes, which is required when using the DDS Recorder.
Install it by following any of the methods described in the given links:

* `Windows installation from binaries <https://eprosima-shapes-demo.readthedocs.io/en/latest/installation/windows_binaries.html>`_
* `Linux installation from sources <https://eprosima-shapes-demo.readthedocs.io/en/latest/installation/linux_sources.html>`_
* `Docker Image <https://eprosima-shapes-demo.readthedocs.io/en/latest/installation/docker_image.html>`_

Start ShapesDemo
================

Let us launch a ShapesDemo instance and start publishing in topics ``Square`` with default settings.

.. figure:: /rst/figures/recorder_shapesdemo_publisher.png
    :align: center
    :scale: 75 %

Recorder configuration
======================

|ddsrecorder| runs with default configuration settings.
This default configuration records all messages of all DDS Topics found in DDS Domain ``0`` in the ``output_YYYY-MM-DD-DD_hh-mm-ss.mcap`` file.

Additionally, it is possible to change the default configuration parameters by means of a YAML configuration file.

.. note::
    Please refer to :ref:`Configuration <recorder_usage_configuration>` for more information on how to configure a |ddsrecorder|.

Recorder execution
==================

Launching a |ddsrecorder| instance is as easy as executing the following command:

.. code-block:: bash

    ddsrecorder

In order to know all the possible arguments supported by this tool, use the command:

.. code-block:: bash

    ddsrecorder --help

.. figure:: /rst/figures/recorder_shapesdemo_exec.png

Stop the recorder with ``Ctrl+C`` and check that the MCAP file exists.

Next Steps
==========

Explore section :ref:`Tutorials <tutorials_dynamic_types>` for more information on how to configure and set up a recorder, as well as to discover multiple scenarios where |ddsrecorder| may serve as a useful tool.
Also, feel free to check out :ref:`this <replayer_getting_started_usage_example>` example, where a |ddsreplayer| is used to reproduce the traffic recorded following the steps in this tutorial.

