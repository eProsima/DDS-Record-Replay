.. include:: ../../exports/alias.include

.. _tutorials_basic_example:

#####################################
Recording data from a DDS application
#####################################

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

mkdir -p recorder_ws/src
cd recorder_ws/src

***********************************
Configure Dynamic Types in Fast DDS
***********************************

Fast DDS does not send the Data Type information by default, it must be configured to do so.

For native types (Data Types that does not rely in other Data Types) this is enough, as Fast DDS will send the ``TypeObject`` by default. However, for more complex types, it is required to use ``TypeInformation`` mechanism. In the Fast DDS ``DomainParticipant`` set the following QoS in order to send this information:

.. code-block:: bash

    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.typelookup_config.use_server = true;

When generating the Types using Fast DDS Gen, the option ``-typeobject`` must be added in order to generate the needed code to fill the ``TypeObject`` data.

IDL file
========

`eProsima Fast DDS-Gen <https://fast-dds.docs.eprosima.com/en/latest/fastddsgen/introduction/introduction.html>`_ is a Java application that generates *eProsima Fast DDS* source code using the data types defined in an IDL (Interface Definition Language) file.

The expected argument list of the application is:

.. code-block:: bash

    fastddsgen -typeobject MyType.idl

This example allows the user to configure the type of the data published.
At the moment, there are two data types that can be used in this example:

``HelloWorld``
.. literalinclude:: /../resources/dds/TypeLookupService/types/hello_world/HelloWorld.idl
    :language: IDL

``Complete``
.. literalinclude:: /../resources/dds/TypeLookupService/types/complete/Complete.idl
    :language: IDL


This tutorial will use the ``HelloWorld`` message.

*********************************
Writing the DynamicType Publisher
*********************************

This is the C++ source code for the application. This source code can also be found `here <https://github.com/eProsima/DDS-Recorder/blob/main/resources/dds/TypeLookupService/TypeLookupServicePublisher.cpp>`_.

**********************************
Writing the DynamicType Subscriber
**********************************

This is the C++ source code for the application. This source code can also be found `here <https://github.com/eProsima/DDS-Recorder/blob/main/resources/dds/TypeLookupService/TypeLookupServicePublisher.cpp>`_.

The Subscriber will detect the data type name and will register it using the type information sent by the
publisher. Thus, the subscriber does not need to know the type.
In order to look-up a data type, just launch the Subscriber setting the same topic name as the one configured in the
publisher side.

***********************
Running the application
***********************

At this point the project is ready for building, compiling and running the application. From the base workspace directory (recorder_ws), run the following command:

.. code-block:: bash
    source install/setup.bash

Open two terminals:
In the first terminal, run:

.. code-block:: bash
    source install/setup.bash
    ./TypeLookupService

In the second terminal, run the DDS Recorder:

.. code-block:: bash
    source install/setup.bash
    ddsrecorder
