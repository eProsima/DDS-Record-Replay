.. include:: ../exports/alias.include
.. include:: ../exports/roles.include

.. |commercial_support_form| raw:: html

    <a href="https://forms.eprosima.com/reach/form/CommercialSupportRequest/formperma/Ac8GwewD7PTDadQZIV92qDEzNFfMlJnYmA029mSJtJ8" target="_blank">Commercial Support form</a>

.. _tutorials_foxglove:

###################################
Visualize data with DDS Monitor Pro
###################################


Background
**********

This tutorial explains how to record data with |ddsrecorder| tool and visualize it using |eddsmonitorpro|.

Prerequisites
*************

It is required to have |eddsrecord| previously installed using one of the following installation methods:

* :ref:`installation_manual_windows`
* :ref:`installation_manual_linux`

We will also use `eProsima DDS Monitor Pro <https://dds-monitor.docs.eprosima.com/en/latest/rst/formalia/titlepage.html#dds-monitor-pro>`_ to visualize the recorded data.
This software is distributed with a *Fast DDS Pro* license, which can be requested through the |commercial_support_form|.

Additionally, we will use `ShapesDemo <https://www.eprosima.com/index.php/products-all/eprosima-shapes-demo>`_ as a DDS Demo application to publish the data that will be recorded.
This application is already prepared to use Fast DDS DynamicTypes, which is required when using the |ddsrecorder| tool.
Download *eProsima Shapes Demo* from `eProsima website <https://www.eprosima.com/index.php/products-all/eprosima-shapes-demo>`_ or install it by following any of the methods described in the given links:

* `Windows installation from binaries <https://eprosima-shapes-demo.readthedocs.io/en/latest/installation/windows_binaries.html>`_
* `Linux installation from sources <https://eprosima-shapes-demo.readthedocs.io/en/latest/installation/linux_sources.html>`_
* `Docker Image <https://eprosima-shapes-demo.readthedocs.io/en/latest/installation/docker_image.html>`_

.. _tutorials_foxglove_configuring_recorder:

Configuring DDS Recorder
************************

The DDS Recorder runs with default configuration parameters, but can also be configured via a YAML file.
In this tutorial we will use a configuration file to change some default parameters and show how this file is loaded.
The configuration file to be used is the following:

.. literalinclude:: /resources/dds_monitor_tutorial/conf.yaml
    :language: yaml

The previous configuration file configures a recorder in DDS Domain ``0`` and saves the output file as: |br|
``<yyyy-MM-dd_HH-mm-ss_zzz>_shapesdemo_data.mcap`` |br|
being ``<yyyy-MM-dd_HH-mm-ss_zzz>`` the timestamp of the time at which the |ddsrecorder| started recording.

Create a new file named ``conf.yaml`` and copy the above snippet into this file.

Running the application
***********************

Start ShapesDemo
================

Launch *eProsima Shapes Demo* application running the following command:

.. code-block:: bash

    ShapesDemo

Start publishing in topics ``Square``, ``Triangle``, and ``Circle`` with default settings:

.. figure:: /rst/figures/dds_monitor_shapesdemo.png
    :align: center

Recorder execution
==================

Launch the |ddsrecorder| tool passing the configuration file as an argument:

.. code-block:: bash

    ddsrecorder -c <path/to/config/file>/conf.yaml

Once you have all the desired data, close the |ddsrecorder| application with ``Ctrl+C``.

.. important::

    Please remember to close the |ddsrecorder| application before accessing the output file as the *.mcap* file needs to be properly closed.

Visualize data with DDS Monitor Pro
===================================

Finally, we will show how to load the generated MCAP file into DDS Monitor Pro in order to display the saved data. 

1. Open |eddsmonitorpro| and press the ``Start monitoring!`` button.
2. Click ``Open a recording instead...`` and load the *.mcap* file previously created: |br|
   ``<yyyy-MM-dd_HH-mm-ss_zzz>_shapesdemo_data.mcap``

.. figure:: /rst/figures/dds_monitor_open_recording.png
    :align: center

.. note::

    The recording can also be opened by going to the ``File`` menu and selecting ``Open Recording...``.

3. Once the *.mcap* file is loaded, create your own layout with custom panels to visualize the recorded data.
   The image below shows an example of a dashboard with several panels for data introspection, in particular a time chart of the *X* coordinates, a *X-Y* chart of the shapes, a panel to spy on the circle samples, and another one to inspect the IDL.

.. figure:: /rst/figures/dds_monitor_shapesdemo_dashboard.png
    :align: center

Feel free to further explore the number of possibilities that |eddsrecorder| and |eddsmonitorpro| together have to offer.
For a complete description of the available panels and visualization features, please refer to the |DDSMonitorDocs|.
