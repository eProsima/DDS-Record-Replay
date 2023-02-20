.. include:: ../../exports/alias.include

.. _tutorials_basic_example:

#################################################
Recording DynamicType data from a DDS application
#################################################

.. contents::
    :local:
    :backlinks: none
    :depth: 1

**********
Background
**********

This examples configures a DDS Publisher using DynamicTypes for the data type definition.
Thus, the user will be able to record the published data using the *eProsima DDS Recorder* tool.
Moreover, the example implements a DDS Subscriber that will receive any kind of data published by the Publisher
as long as the Publisher sends the data type information.

*************
Prerequisites
*************

Ensure that *eProsima DDS Recorder* is installed together with *eProsima* dependencies, i.e. *Fast DDS*, *Fast CDR* and *DDS Router*.
Also, remember to source the environment in every terminal in this tutorial.

.. code-block:: bash

    source <path-to-fastdds-installation>/install/setup.bash
    source <path-to-ddsrouter-installation>/install/setup.bash
    source <path-to-ddsrecorder-installation>/install/setup.bash

**********************
Creating the workspace
**********************

.. code-block:: bash

    mkdir -p recorder_ws/src
    cd recorder_ws/src

**********************************
Configure DynamicTypes in Fast DDS
**********************************

*eProsima Fast DDS* does not send the Data Type information by default, it must be configured to do so.

Complex types
============

For complex types, it is required to use ``TypeInformation`` mechanism. In the *eProsima Fast DDS* ``DomainParticipant`` set the following QoS in order to send this information:

.. code-block:: bash

    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.typelookup_config.use_server = true;

Native types
============

For native types Fast DDS will send the ``TypeObject`` by default.

IDL file
========

`eProsima Fast DDS-Gen <https://fast-dds.docs.eprosima.com/en/latest/fastddsgen/introduction/introduction.html>`_ is a Java application that generates *eProsima Fast DDS* source code using the data types defined in an IDL (Interface Definition Language) file.
When generating the Types using Fast DDS Gen, the option ``-typeobject`` must be added in order to generate the needed code to fill the ``TypeObject`` data.

The expected argument list of the application is:

.. code-block:: bash

    fastddsgen -typeobject MyType.idl

This example allows the user to configure the type of the data published.
At the moment, there are two data types that can be used in this example:

* `HelloWorld <https://github.com/eProsima/DDS-Recorder/blob/main/resources/dds/TypeLookupService/types/hello_world/HelloWorld.idl>`_

.. literalinclude:: ../../../../resources/dds/TypeLookupService/types/hello_world/HelloWorld.idl

* `Complete <https://github.com/eProsima/DDS-Recorder/blob/main/resources/dds/TypeLookupService/types/complete/Complete.idl>`_

.. literalinclude:: ../../../../resources/dds/TypeLookupService/types/complete/Complete.idl


This tutorial files are generated using use the ``HelloWorld`` message.

*********************
DynamicType Publisher
*********************

This is the C++ source code for the application. This source code can also be found `here <https://github.com/eProsima/DDS-Recorder/blob/main/resources/dds/TypeLookupService/TypeLookupServicePublisher.cpp>`_.

**********************
DynamicType Subscriber
**********************

This is the C++ source code for the application. This source code can also be found `here <https://github.com/eProsima/DDS-Recorder/blob/main/resources/dds/TypeLookupService/TypeLookupServicePublisher.cpp>`_.

The Subscriber will detect the data type name and will register it using the type information sent by the
publisher. Thus, the subscriber does not need to know the type.
In order to look-up a data type, just launch the Subscriber setting the same topic name as the one configured in the
publisher side.

***********************
Running the application
***********************

At this point the project is ready for building, compiling and running the application.

.. code-block:: bash

    colcon build
    source install/setup.bash

Recording samples with DDS Recorder
===================================

Open two terminals:

In the first terminal, run:

.. code-block:: bash

    source install/setup.bash
    cd DDS-Recorder/build/TypeLookupService
    ./TypeLookupService

In the second terminal, run the ddsrecorder:

.. code-block:: bash

    source install/setup.bash
    ddsrecorder

Publisher <-> Subscriber
========================

Open two terminals:

In the first terminal, run the DDS Publisher:

.. code-block:: bash

    source install/setup.bash
    cd DDS-Recorder/build/TypeLookupService
    ./TypeLookupService --entity publisher

In the second terminal, run the DDS Subscriber:

.. code-block:: bash

    source install/setup.bash
    cd DDS-Recorder/build/TypeLookupService
    ./TypeLookupService --entity subscriber

.. figure:: /resources/tutorials/basic_publisher_subscriber.png
