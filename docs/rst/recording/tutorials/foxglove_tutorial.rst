.. include:: ../../exports/alias.include

.. _tutorials_foxglove:

#####################################
Visualize recorded data with Foxglove
#####################################

.. contents::
    :local:
    :backlinks: none
    :depth: 2

**********
Background
**********

.. note::
    This page is under maintenance and will be updated soon.

*************
Prerequisites
*************

It is required to have |eddsrecordreplay| previously installed using one of the following installation methods:

* :ref:`installation_manual_windows`
* :ref:`installation_manual_linux`
* :ref:`docker`

Additionally, `ShapesDemo <https://www.eprosima.com/index.php/products-all/eprosima-shapes-demo>`_ is required to publish and subscribe shapes of different colors and sizes.
The ShapesDemo application is already prepared to use Fast DDS DynamicTypes, which is required when using the DDS Recorder tool.
Install it by following any of the methods described in the given links:

* `Windows installation from binaries <https://eprosima-shapes-demo.readthedocs.io/en/latest/installation/windows_binaries.html>`_
* `Linux installation from sources <https://eprosima-shapes-demo.readthedocs.io/en/latest/installation/linux_sources.html>`_
* `Docker Image <https://eprosima-shapes-demo.readthedocs.io/en/latest/installation/docker_image.html>`_

*************
Configuration
*************

The following YAML configuration file configures a recorder in domain ``0`` and save the output file as ``view_foxglove.mcap`` without timestamp.

DDS
===

.. literalinclude:: ../../../../resources/configurations/conf-foxglove.yaml
    :language: yaml
    :lines: 1-2

Recorder output file
====================

.. literalinclude:: ../../../../resources/configurations/conf-foxglove.yaml
    :language: yaml
    :lines: 4-9


***********************
Running the application
***********************

Start ShapesDemo
================

Initiate *eProsima Shapes Demo* running the following command:

.. code-block:: bash

    ShapesDemo

Let us launch a *eProsima Shapes Demo* instance in one of the DDS networks, and start publishing in topics ``Square`` and ``Triangle`` with default settings.

.. figure:: /resources/tutorials/foxglove_shapes_demo.png
    :align: center

Recorder execution
==================

Launch the *eProsima DDS Recorder* instance with the configuration file as an argument executing the following command:

.. code-block:: bash

    ddsrecorder -c /resources/configurations/conf-foxglove.yaml

.. figure:: /resources/tutorials/foxglove_ddsrecorder.png

Visualize with Foxglove
=======================

1. Open `Foxglove Studio <https://studio.foxglove.dev/>`_ .
2. Click ``Òpen local file`` and select the ``mcap`` file previously created: ``view_foxglove.macp``.

.. figure:: /resources/tutorials/foxglove_foxglove.png
