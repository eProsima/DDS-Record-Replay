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

.. note::
    This page is under maintenance and will be updated soon.

*************
Configuration
*************

DDS
===

.. literalinclude:: ../../../../configurations/conf-foxglove.yaml
    :language: yaml
    :lines: 1-2

Recorder output file
====================

.. literalinclude:: ../../../../configurations/conf-foxglove.yaml
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

Recorder execution
==================

Launch the *eProsima DDS Recorder* instance with the configuration file as an argument executing the following command:

.. code-block:: bash

    ddsrecorder -c /resources/configurations/conf-foxglove.yaml

.. figure:: /resources/tutorials/foxglove_recorder.png

Visualize with Foxglove
=======================

1. Open `Foxglove Studio<https://studio.foxglove.dev/>`_ .
2. Click in ``Òpen local file`` and select the ``mcap`` file previously created ``view_foxglove.macp``.

.. figure:: /resources/tutorials/foxglove_foxglove.png
