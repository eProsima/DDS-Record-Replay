.. include:: ../../exports/alias.include

.. _tutorials_basic_example:

#################################################
Recording DynamicType data from a DDS application
#################################################

.. contents::
    :local:
    :backlinks: none
    :depth: 2

**********
Background
**********

Since the DDS Recorder only supports dynamic types, DDS Publisher using DynamicTypes for the data type definition.
Thus, the user will be able to record the published data using the |ddsrecorder| tool of the |eddsrecord| software.
Moreover, the example implements a DDS Subscriber that will receive any kind of data published by the Publisher.
For that task, the DDS Publisher and the DDS Subscriber implement a TypeLookupService that send the TypeInformation of the data types used by the publisher.

The source code of this tutorial is provided `here <https://github.com/eProsima/DDS-Recorder/tree/main/resources/dds/TypeLookupService>`_ with an explanation of how to build and run it.

This tutorial focuses on how to send the data type information using Fast DDS DynamicTypes and other relevant aspects of DynamicTypes.
For more information about how to create the workspace with a basic DDS Publisher and a basic DDS Subscriber, please refer to `Writing a simple C++ publisher and subscriber application <https://fast-dds.docs.eprosima.com/en/latest/fastdds/getting_started/simple_app/simple_app.html>`_ .

*************
Prerequisites
*************

Ensure that |eddsrecord| is installed together with *eProsima* dependencies, i.e. *Fast DDS*, *Fast CDR* and *DDS Pipe*.

If |eddsrecord| was installed using the `recommended installation <https://dds-recorder.readthedocs.io/en/latest/rst/installation/docker.html>`_ the environment is source by default, otherwise, just remember to source it in every terminal in this tutorial:

.. code-block:: bash

    source <path-to-fastdds-installation>/install/setup.bash
    source <path-to-ddspipe-installation>/install/setup.bash
    source <path-to-ddsrecorder-installation>/install/setup.bash

*********************
Generating data types
*********************

`eProsima Fast DDS-Gen <https://fast-dds.docs.eprosima.com/en/latest/fastddsgen/introduction/introduction.html>`_ is a Java application that generates *eProsima Fast DDS* source code using the data types defined in an IDL (Interface Definition Language) file.
When generating the Types using *eProsima Fast DDS Gen*, the option ``-typeobject`` must be added in order to generate the needed code to fill the ``TypeInformation`` data.

The expected argument list of the application is:

.. code-block:: bash

    fastddsgen -typeobject MyType.idl

*************
DDS Publisher
*************

The DDS publisher is acting as a server of types.

Fast DDS does not send the DataType information by default, it must be configured to do so.

Complex types
============

For complex types, it is required to use ``TypeInformation`` mechanism. In the *eProsima Fast DDS* ``DomainParticipant`` set the following QoS in order to send this information:

.. code-block:: bash

    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.typelookup_config.use_server = true;

Native types
============

For native types *eProsima Fast DDS* will send the ``TypeObject`` by default.

Data types
==========

At the moment, there are two data types that can be used:

* `HelloWorld.idl <https://github.com/eProsima/DDS-Recorder/blob/main/resources/dds/TypeLookupService/types/hello_world/HelloWorld.idl>`_

.. literalinclude:: ../../../../resources/dds/TypeLookupService/types/hello_world/HelloWorld.idl

* `Complete.idl <https://github.com/eProsima/DDS-Recorder/blob/main/resources/dds/TypeLookupService/types/complete/Complete.idl>`_

.. literalinclude:: ../../../../resources/dds/TypeLookupService/types/complete/Complete.idl

Examining the code
==================

This section explains the C++ source code of the DDS Publisher, which can also be found `here <https://github.com/eProsima/DDS-Recorder/blob/main/resources/dds/TypeLookupService>`_.

For simplicity is going to be explain just the code related to the ``HelloWorld`` type.

The private data members of the class defines the DDS Topic, DataTypeKind, DDS Topic type and DynamicType.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServicePublisher.h
    :language: C++
    :lines: 139-146

The next lines show the constructor of the ``TypeLookupServicePublisher`` class that implements the publisher.
The publisher is created with the topic and data type to use.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServicePublisher.cpp
    :language: C++
    :lines: 45-54

Inside the ``TypeLookupServicePublisher`` constructor are defined the DomainParticipantQos.
As the publisher act as a server of types, set the QoS in order to send this information.
Set ``use_client`` to ``false`` and ``use_server`` to ``true``.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServicePublisher.cpp
    :language: C++
    :lines: 58-62

Next, we register the type in the participant:

1. Generate the dynamic type through ``generate_helloworld_type_()`` explained below.
2. Set the data type.
3. Create the ``TypeSupport`` with the dynamic type previously created.
4. Set the ``type`` to ``TypeInformation`` and not to ``TypeObject`` since we want to send the information of the type and not the object.

* This answer `DDS-XTypes 1.2. <https://www.omg.org/spec/DDS-XTypes/1.2>`_

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServicePublisher.cpp
    :language: C++
    :lines: 73-95

The function ``generate_helloworld_type_()`` returns the dynamic type generated with the object and id of the type.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServicePublisher.cpp
    :language: C++
    :lines: 256-271

Then are initialized the Publisher, DDS Topic and DDS DataWriter.

To make the publication, the public member function ``publish()`` is implemented:

1. It create the variable that will contain the user data, ``dynamic_data_``.
2. Fill that variable with the function ``fill_helloworld_data_(msg)``, explained below.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServicePublisher.cpp
    :language: C++
    :lines: 229-254

The function ``fill_helloworld_data_()`` returns the data to be sent with the information filled in.

First, it is created the ``Dynamic_ptr`` that will be filled in and return.
With the use of ``DynamicDataFactory`` we create the data that corresponds to our data type.
Finally, data variables are assigned, in this case, ``index`` and ``message``.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServicePublisher.cpp
    :language: C++
    :lines: 293-307

**************
DDS Subscriber
**************

The DDS Subscriber is acting as a client of types.
Thus, the subscriber does not need to know the type.

Examining the code
==================

This section explains the C++ source code of the DDS Publisher, which can also be found `here <https://github.com/eProsima/DDS-Recorder/blob/main/resources/dds/TypeLookupService>`_.

The private data members of the class defines the DDS Topic, DDS Topic type and DynamicType.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServiceSubscriber.h
    :language: C++
    :lines: 105-110

The next lines show the constructor of the ``TypeLookupServiceSubscriber`` class that implements the subscriber setting the topic name as the one configured in the
publisher side.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServiceSubscriber.cpp
    :language: C++
    :lines: 45-53

Inside the ``TypeLookupServiceSubscriber`` constructor are defined the DomainParticipantQos.
As the publisher act as a client of types, set the QoS in order to receive this information.
Set ``use_client`` to ``true`` and ``use_server`` to ``false``.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServiceSubscriber.cpp
    :language: C++
    :lines: 57-61

Then is initialized the Subscriber.

Inside ``on_data_available()`` it is created the ``DynamicData_ptr`` where the samples received will be read.

As in the publisher, it is used ``DynamicDataFactory`` for the creation of the data that corresponds to our data type.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServiceSubscriber.cpp
    :language: C++
    :lines: 143-170

The function ``on_type_information_received()`` detect the topics received and discover it if they have the same as the topic that we are waiting for.
Discover the topic means set the topic as discover and register it using the function ``register_remote_type_callback_()`` explained below.
Once the topic has been discovered and registered, it is created a DataReader on this topic.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServiceSubscriber.cpp
    :language: C++
    :lines: 172-212

Finally, the function ``register_remote_type_callback_()`` is in charge of register the topic received.
First of all, it creates a ``TypeSupport`` with the corresponding type and register it into the participant.
Then, it creates the DDS Topic with the topic name set in the creation of the Subscriber and the topic type previously register.
Finally, it creates the DataReader of that topic.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServiceSubscriber.cpp
    :language: C++
    :lines: 278-304

***********************
Running the application
***********************

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
