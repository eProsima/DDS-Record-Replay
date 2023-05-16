.. include:: ../../exports/alias.include

.. _replayer_getting_started_usage_example:

################
Example of usage
################

This example will serve as a hands-on tutorial, aimed at introducing some of the key concepts and features that
|eddsrecord| replay application (|ddsreplayer| or ``ddsreplayer``) has to offer.

Prerequisites
=============

It is required to have |eddsrecord| previously installed using one of the following installation methods:

* :ref:`installation_manual_windows`
* :ref:`installation_manual_linux`
* :ref:`docker`

Additionally, `ShapesDemo <https://www.eprosima.com/index.php/products-all/eprosima-shapes-demo>`_ is required to publish and subscribe shapes of different colors and sizes.
Install it by following any of the methods described in the given links:

* `Windows installation from binaries <https://eprosima-shapes-demo.readthedocs.io/en/latest/installation/windows_binaries.html>`_
* `Linux installation from sources <https://eprosima-shapes-demo.readthedocs.io/en/latest/installation/linux_sources.html>`_
* `Docker Image <https://eprosima-shapes-demo.readthedocs.io/en/latest/installation/docker_image.html>`_

This is a follow-up tutorial, and assumes that |ddsrecorder| :ref:`recorder_getting_started_usage_example` has already been completed.

Start ShapesDemo
================

Let us launch a ShapesDemo instance and create a subscription in the ``Square`` topic with default settings.

.. figure:: /rst/figures/replayer_shapesdemo_subscriber.png
    :align: center
    :scale: 75 %

Replayer configuration
======================

The only configuration option required by a |ddsreplayer| is the path to an input MCAP file, which can be provided both as a CLI argument or via YAML configuration.
By default, all messages stored in the provided input file are played back in DDS Domain ``0``, starting at the very moment the application is launched.

It is also possible to change the default configuration parameters by means of a YAML configuration file.

.. note::
    Please refer to :ref:`Configuration <replayer_usage_configuration>` for more information on how to configure a |ddsreplayer|.

Replayer execution
==================

Launching a |ddsreplayer| instance is as easy as executing the following command:

.. code-block:: bash

    ddsreplayer -i output_YYYY-MM-DD-DD_hh-mm-ss.mcap

In order to know all the possible arguments supported by this tool, use the command:

.. code-block:: bash

    ddsreplayer --help

.. figure:: /rst/figures/replayer_shapesdemo_exec.png

Execution will end once every message found in the given input file is played back, although it can also be terminated with ``Ctrl+C`` at any point.

Next Steps
==========

Feel free to experiment with the many :ref:`configuration <replayer_usage_configuration>` options available for a |ddsreplayer| instance.
For example, you may try to modify the playback rate, block/allow the ``Square`` topic in the middle of execution, or set a different topic QoS configuration via the builtin-topics list.
