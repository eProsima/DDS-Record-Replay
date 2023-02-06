.. include:: ../../exports/alias.include

.. _tutorials_custom_config:

####################
Custom Configuration
####################

Configuration
=============

The following YAML configuration file configures a DDS Recorder:

.. literalinclude:: ../../../../resources/configurations/conf-ddsrecorder.yaml
    :language: yaml

Let us first add all topics:

.. literalinclude:: ../../../../resources/configurations/conf-ddsrecorder.yaml
    :language: yaml
    :lines: 2-3

Let us block ``add_blocked_topics_list_here`` topic:

.. literalinclude:: ../../../../resources/configurations/conf-ddsrecorder.yaml
    :language: yaml
    :lines: 6-7

Configure the DDS ``domain`` identifier.

.. literalinclude:: ../../../../resources/configurations/conf-ddsrecorder.yaml
    :language: yaml
    :lines: 10

The recorder output file does support the following configurations:

.. literalinclude:: ../../../../resources/configurations/conf-ddsrecorder.yaml
    :language: yaml
    :lines: 13-16

Execute example
===============

Execute a Fast DDS TypeLookup example:

.. code-block:: bash

    ./TypeLookupExample publisher

Run DDS Recorder
================

In order to execute the ``DDS Recorder`` use the following command:

.. code-block:: bash

    ddsrecorder --config-path /ddsrecorder/resources/configurations/conf-ddsrecorder.yaml
