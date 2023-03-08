.. include:: ../../exports/alias.include

.. _tutorials_foxglove:

#####################################
Visualize recorded data with Foxglove
#####################################

.. contents::
    :local:
    :backlinks: none
    :depth: 2

Background
**********

This tutorial explains how to record data with |ddsrecorder| tool and visualize it with `Foxglove Studio <https://foxglove.dev/studio>`__.

Prerequisites
*************

It is required to have |eddsrecord| previously installed using one of the following installation methods:

* :ref:`installation_manual_windows`
* :ref:`installation_manual_linux`
* :ref:`docker`

Additionally, we will use `ShapesDemo <https://www.eprosima.com/index.php/products-all/eprosima-shapes-demo>`_ as a DDS Demo application to publish the data that will be recorded.
This application is already prepared to use Fast DDS DynamicTypes, which is required when using the |ddsrecorder| tool.
Download *eProsima Shapes Demo* from `eProsima website <https://www.eprosima.com/index.php/products-all/eprosima-shapes-demo>`_ or install it by following any of the methods described in the given links:

* `Windows installation from binaries <https://eprosima-shapes-demo.readthedocs.io/en/latest/installation/windows_binaries.html>`_
* `Linux installation from sources <https://eprosima-shapes-demo.readthedocs.io/en/latest/installation/linux_sources.html>`_
* `Docker Image <https://eprosima-shapes-demo.readthedocs.io/en/latest/installation/docker_image.html>`_

Configuring DDS Recorder
************************

The DDS Recorder runs with default configuration parameters, but can also be configured via a YAML file.
In this tutorial we will use a configuration file to change some default parameters and show how this file is loaded.
The configuration file to be used is the following:

.. literalinclude:: /resources/foxglove_tutorial/conf.yaml
    :language: yaml

The previous configuration file configures a recorder in DDS Domain ``0`` and save the output file as ``shapesdemo_data_<YYYY-MM-DD_hh-mm-ss>.mcap``, being ``<YYYY-MM-DD_hh-mm-ss>`` the timestamp of the time at which the |ddsrecorder| started recording.

Create a new file named ``conf.yaml`` and copy the above snippet into this file.

Running the application
***********************

Start ShapesDemo
================

Launch *eProsima Shapes Demo* application running the following command:

.. code-block:: bash

    ShapesDemo

Start publishing in topics ``Square``, ``Triangle``, and ``Circle`` with default settings:

.. figure:: /rst/figures/foxglove_shapesdemo.png
    :align: center

Recorder execution
==================

Launch the |ddsrecorder| tool passing the configuration file as an argument:

.. code-block:: bash

    ddsrecorder -c <path/to/config/file>/conf.yaml

Once you have all the desired data, close the |ddsrecorder| application with ``Ctrl+C``.

.. important::

    Please remember to close the |ddsrecorder| application before accessing the output file as the *.mcap* file needs to be properly closed.

Visualize data with Foxglove Studio
===================================

Finally, we will show how to load the generated MCAP file into Foxglove Studio in order to display the saved data.

1. Open `Foxglove Studio <https://studio.foxglove.dev/>`__ web application using Google Chrome or download the desktop application from their `Foxglove website <https://foxglove.dev/>`_.
   We recommend to use the web application as the it is usually up to date with the latest features.
2. Click ``Open local file`` and load the *.mcap* file previously created: ``shapesdemo_data.mcap``.
3. Once the *.mcap* file is loaded, create your own layout with custom panels to visualize the recorded data.
   The image below shows an example of a dashboard with several panels for data introspection.

.. figure:: /rst/figures/foxglove_studio_shapesdemo.png
    :align: center

Feel free to further explore the number of possibilities that |eddsrecord| and *Foxglove Studio* together have to offer.
