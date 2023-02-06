.. include:: ../../exports/alias.include

.. _tutorials_ros_cloud:

##################
ROS 2 and FoxGlove
##################

Configuration
=============

The following YAML configuration file configures a DDS Recorder in :term:`Domain Id` ``0``.

.. literalinclude:: ../../../../resources/configurations/ros2_configuration.yaml
    :language: yaml

Run ROS 2
=========

Execute a ROS 2 ``demo_nodes_cpp`` *talker* in domain ``100``:

.. code-block:: bash

    ROS_DOMAIN_ID=100 ros2 run demo_nodes_cpp talker

Run DDS Recorder
================

In order to execute the ``DDS Recorder`` use the following command:

.. code-block:: bash

    ddsrecorder --config-path /ddsrecorder/resources/configurations/ros2_configuration.yaml

Visualizing with FoxGlove Studio
================================

Open `FoxGlove Studio <https://studio.foxglove.dev/>_` with the given ``output.mcap`` file.
