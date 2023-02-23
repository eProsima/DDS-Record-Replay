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

This examples configures a DDS Publisher using DynamicTypes for the data type definition.
Thus, the user will be able to record the published data using the |ddsrecorder| tool of the |eddsrecord| software.
Moreover, the example implements a DDS Subscriber that will receive any kind of data published by the Publisher as long as the Publisher sends the data type information.

The source code of this tutorial is provided `here <https://github.com/eProsima/DDS-Recorder/tree/main/resources/dds/TypeLookupService>`_ with an explanation of how to build and run it.

*************
Prerequisites
*************

Ensure that |eddsrecord| is installed together with *eProsima* dependencies, i.e. *Fast DDS*, *Fast CDR* and *DDS Pipe*.

If |eddsrecord| was installed using the `recommended installation <https://dds-recorder.readthedocs.io/en/latest/rst/installation/docker.html>`_ the environment is source by default, otherwise, just remember to source it in every terminal in this tutorial.

.. code-block:: bash

    source <path-to-fastdds-installation>/install/setup.bash
    source <path-to-ddspipe-installation>/install/setup.bash
    source <path-to-ddsrecorder-installation>/install/setup.bash

This tutorial focuses on how to send the data type information using Fast DDS DynamicTypes and other relevant aspects of DynamicTypes.
For more information about how to create the workspace with a basic DDS Publisher and a basic DDS Subscriber, please refer to `Writing a simple C++ publisher and subscriber application <https://fast-dds.docs.eprosima.com/en/latest/fastdds/getting_started/simple_app/simple_app.html>`_ .

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

Fast DDS does not send the Data Type information by default, it must be configured to do so.

Complex types
============

For complex types, it is required to use ``TypeInformation`` mechanism. In the *eProsima Fast DDS* ``DomainParticipant`` set the following QoS in order to send this information:

.. code-block:: bash

    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.typelookup_config.use_server = true;

Native types
============

For native types *eProsima Fast DDS* will send the ``TypeObject`` by default.

At the moment, there are two data types that can be used:

* `HelloWorld.idl <https://github.com/eProsima/DDS-Recorder/blob/main/resources/dds/TypeLookupService/types/hello_world/HelloWorld.idl>`_

.. literalinclude:: ../../../../resources/dds/TypeLookupService/types/hello_world/HelloWorld.idl

* `Complete.idl <https://github.com/eProsima/DDS-Recorder/blob/main/resources/dds/TypeLookupService/types/complete/Complete.idl>`_

.. literalinclude:: ../../../../resources/dds/TypeLookupService/types/complete/Complete.idl

Examining the code
==================

This section explains the C++ source code of the DDS Publisher, which can also be found `here <https://github.com/eProsima/DDS-Recorder/blob/main/resources/dds/TypeLookupService>`_.

For simplicity is going to be explain the code related to the ``HelloWorld``type.

The private data members of the class defines the DDS Topic, data type, DDS Topic type and DynamicType.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServicePublisher.h
    :language: C++
    :lines: 139-146

The next line creates the ``TypeLookupServicePublisher`` class that implements the publisher.
The publisher is created with the topic and data type to use.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServicePublisher.cpp
    :language: C++
    :lines: 45-54

Inside the ``TypeLookupServicePublisher`` are defined the QoS.
As the publisher act as a server of types, the qos set ``use_client`` to ``false`` and ``use_server`` to ``true``.

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

To make the publication, the public member function ``publish()`` is implemented.
It is created the variable that will contain the user data, ``dynamic_data_``.
To fill that variable it is used ``fill_helloworld_data_(msg)``, explained below.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServicePublisher.cpp
    :language: C++
    :lines: 229-254

The function ``fill_helloworld_data_()`` returns the data to be sent with the information filled in.
First it is created the ``Dynamic_ptr`` that will be filled in and return.
Then, with the use of the ``DynamicDataFactory`` we create the data that corresponds to our data type.
Finally, data variables are assigned, in this case, ``index`` and ``message``.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServicePublisher.cpp
    :language: C++
    :lines: 293-307

**************
DDS Subscriber
**************

The DDS Subscriber is acting as a client of types.
Thus, the subscriber does not need to know the type.
In order to look-up a data type, just launch the subscriber setting the same topic name as the one configured in the
publisher side.

Examining the code
==================

This section explains the C++ source code of the DDS Publisher, which can also be found `here <https://github.com/eProsima/DDS-Recorder/blob/main/resources/dds/TypeLookupService>`_.


The next line creates the ``TypeLookupServiceSubscriber`` class that implements the subscriber.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServiceSubscriber.cpp
    :language: C++
    :lines: 45-53

.. todo:

    .. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServiceSubscriber.cpp
        :language: C++
        :lines: 60-61

Inside the ``on_data_available()`` function it is created the ``DynamicData_ptr`` where the samples received will be read.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServiceSubscriber.cpp
    :language: C++
    :lines: 143-170

The function ``on_type_information_received()`` detect the topic and the data type received and create the callback using the function explained below.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServiceSubscriber.cpp
    :language: C++
    :lines: 172-212

Finally, the function ``register_remote_type_callback_()`` is in charge of create a callback that register the topic received.
This function performs several actions:

1. Register the type from the topic received.
2. Create a ``DDS Topic`` with the same topic name as the one received from the publisher.
3. Create a ``DataReader`` that identifies the data to be read and accesses that data.
4. Update the ``TypeLookupServiceSubscriber`` members and notify all that the type has been discovered and registered.

.. literalinclude:: ../../../../resources/dds/TypeLookupService/TypeLookupServiceSubscriber.cpp
    :language: C++
    :lines: 278-320

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
