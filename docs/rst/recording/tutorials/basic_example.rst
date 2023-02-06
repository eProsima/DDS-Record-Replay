.. include:: ../../exports/alias.include

.. _tutorials_basic_example:

###################
Basic Configuration
###################

Configuration
=============

The following YAML configuration file configures a DDS Recorder in :term:`Domain Id` ``0``.

.. literalinclude:: ../../../../resources/configurations/simple_configuration.yaml
    :language: yaml

Execute example
===============

Execute a Fast DDS TypeLookup example:

.. code-block:: bash

    ./TypeLookupExample publisher

Run DDS Recorder
================

In order to execute the ``DDS Recorder`` use the following command:

.. code-block:: bash

    ddsrecorder --config-path /ddsrecorder/resources/configurations/simple_configuration.yaml

.. figure:: /rst/figures/basic_example.png
