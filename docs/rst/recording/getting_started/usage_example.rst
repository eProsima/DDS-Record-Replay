.. include:: ../../exports/alias.include

.. _getting_started_usage_example:

################
Example of usage
################

This example will serve as a hands-on tutorial, aimed at introducing some of the key concepts and features that
|eddsrecorder| has to offer.

Start ShapesDemo
================

`ShapesDemo <https://www.eprosima.com/index.php/products-all/eprosima-shapes-demo>`_ is an application that publishes
and subscribes to shapes of different colors and sizes moving on a board. This is nothing more than a graphical tool to
test the correctness of a specific DDS protocol implementation, as well as to prove interoperability with other
implementations.

Let us launch a ShapesDemo instance and start publishing in topics ``Square`` with default settings.

.. figure:: /rst/figures/shapesdemo_publisher.png
    :scale: 75 %

Recorder configuration
======================

A |ddsrecorder| requires one YAML configuration file as the operation of this application is configured via this YAML configuration file.
|ddsrecorder| provide a configutation file named ``DDS_RECORDER_CONFIGURATION.yaml`` used by default if no configuration file is provided as argument, that must be in the same directory where the application is executed.


Recorder execution
==================

Launching a |ddsrecorder| instance is as easy as executing the following command:

.. code-block:: bash

    ddsrecorder

In order to know all the possible arguments supported by this tool, use the command:

.. code-block:: bash

    ddsrecorder --help or ddsrecorder -h

.. figure:: /rst/figures/shapesdemo_exec.png

Please feel free to explore section :ref:`Tutorials <tutorials_basic_example>` for more
information on how to configure and set up a recorder, as well as to discover multiple scenarios where |ddsrecorder| may
serve as a useful tool.
