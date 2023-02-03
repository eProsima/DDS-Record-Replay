.. include:: ../exports/alias.include

.. _usage_example:

################
Example of usage
################

This example will serve as a hands-on tutorial, aimed at introducing some of the key concepts and features that
|eddsrecorder| has to offer.

Running Fast DDS Publisher
==========================

Run the Fast DDS Type Introspection Publisher executing the following commands:

.. code-block:: bash

    ./dds/TypeIntrospectionExample/bin/TypeIntrospectionExample publisher -o

Recorder configuration
======================

A configuration file is all that is required in order to run a |ddsrecorder| instance. In a nutshell, recorder will
track messages if their associated topics match the filters contained in its ``allowlist`` and ``blocklist``.

Following is an example of configuration file:

.. literalinclude:: ../../../resources/configurations/conf-ddsrecorder.yaml
    :language: yaml

Let us first add all topics:

.. literalinclude:: ../../../resources/configurations/conf-ddsrecorder.yaml
    :language: yaml
    :lines: 2-3

Let us block ``add_blocked_topics_list_here`` topic:

.. literalinclude:: ../../../resources/configurations/conf-ddsrecorder.yaml
    :language: yaml
    :lines: 6-7

The only configuration required for simple participants is the DDS ``domain`` identifier.

.. literalinclude:: ../../../resources/configurations/conf-ddsrecorder.yaml
    :language: yaml
    :lines: 10

The recorder output file does support the following configurations:

.. literalinclude:: ../../../resources/configurations/conf-ddsrecorder.yaml
    :language: yaml
    :lines: 13-16

Recorder execution
==================

Now, with the configuration files ready, launching a |ddsrecorder| instance is as easy as executing the following command:

.. code-block:: bash

    ddsrecorder --config-path /ddsrecorder/resources/configurations/conf-ddsrecorder.yaml

In order to know all the possible arguments supported by this tool, use the command:

.. code-block:: bash

    ddsrecorder     ddsrecorder --help` or `ddsrecorder -h


Please feel free to explore sections Examples and :ref:`Use Cases <ros_cloud>` for more
information on how to configure and set up a recorder, as well as to discover multiple scenarios where |ddsrecorder| may
serve as a useful tool.
