.. include:: ../exports/alias.include

.. _tutorials_dynamic_types:

#################################
Configuring Fast DDS DynamicTypes
#################################

.. contents::
    :local:
    :backlinks: none
    :depth: 2

**********
Background
**********

As explained in :ref:`this section <recorder_getting_started_project_overview>`, a |ddsrecorder| instance stores (by default) all data regardless of whether their associated data type is received or not.
However, some applications require this information to be recorded and written in the resulting MCAP file, and for this to occur the publishing applications must send it via :term:`Dynamic Types<DynamicTypes>`.

This tutorial focuses on how to send the data type information using Fast DDS DynamicTypes and other relevant aspects of DynamicTypes.
More specifically, this tutorial implements a DDS Publisher configured to send its data type, a DDS Subscriber that collects the data type and is able to read the incoming data, and a DDS Recorder is launched to save all the data published on the network.
For more information about how to create the workspace with a basic DDS Publisher and a basic DDS Subscriber, please refer to `Writing a simple C++ publisher and subscriber application <https://fast-dds.docs.eprosima.com/en/latest/fastdds/getting_started/simple_app/simple_app.html>`_ .

The source code of this tutorial can be found in the public |eddsrecord| `GitHub repository <https://github.com/eProsima/DDS-Record-Replay/tree/v0.4.0/resources/dds/TypeLookupService>`_ with an explanation of how to build and run it.

.. warning::

    This tutorial works with `this <https://github.com/eProsima/Fast-DDS/tree/bugfix/complex-dynamic-types>`_ branch of Fast DDS.

*************
Prerequisites
*************

Ensure that |eddsrecord| is installed together with *eProsima* dependencies, i.e. *Fast DDS*, *Fast CDR* and *DDS Pipe*.

If |eddsrecord| was installed using the `recommended installation <https://dds-recorder.readthedocs.io/en/latest/rst/installation/docker.html>`_ the environment is sourced by default, otherwise, just remember to source it in every terminal in this tutorial:

.. code-block:: bash

    source <path-to-fastdds-installation>/install/setup.bash
    source <path-to-ddspipe-installation>/install/setup.bash
    source <path-to-ddsrecordreplay-installation>/install/setup.bash

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

The DDS publisher will be configured to act as a server of the data types of the data it publishes.

However, *Fast DDS* does not send the data type information by default, it must be configured to do so.

Data types
==========

At the moment, there are two data types that can be used:

* `HelloWorld.idl <https://github.com/eProsima/DDS-Record-Replay/blob/v0.4.0/resources/dds/TypeLookupService/types/hello_world/HelloWorld.idl>`_

.. literalinclude:: ../../../resources/dds/TypeLookupService/types/v2/hello_world/HelloWorld.idl

* `Complete.idl <https://github.com/eProsima/DDS-Record-Replay/blob/v0.4.0/resources/dds/TypeLookupService/types/complete/Complete.idl>`_

.. literalinclude:: ../../../resources/dds/TypeLookupService/types/v2/complete/Complete.idl

Examining the code
==================

This section explains the C++ source code of the DDS Publisher, which can also be found `here <https://github.com/eProsima/DDS-Record-Replay/blob/v0.4.0/resources/dds/TypeLookupService>`_.

The private data members of the class defines the DDS Topic, ``DataTypeKind``, DDS Topic type and DynamicType.
The ``DataTypeKind`` defines the type to be used by the application (``HelloWorld`` or ``Complete``).
For simplicity, this tutorial only covers the code related to the ``HelloWorld`` type.

.. literalinclude:: ../../../resources/dds/TypeLookupService/TypeLookupServicePublisher.h
    :language: C++
    :lines: 139-146

The next lines show the constructor of the ``TypeLookupServicePublisher`` class that implements the publisher.
The publisher is created with the topic and data type to use.

.. literalinclude:: ../../../resources/dds/TypeLookupService/TypeLookupServicePublisher.cpp
    :language: C++
    :lines: 45-54

Inside the ``TypeLookupServicePublisher`` constructor are defined the DomainParticipantQos.
As the publisher act as a server of types, its QoS must be configured to send this information.
Set ``use_client`` to ``false`` and ``use_server`` to ``true``.

.. literalinclude:: ../../../resources/dds/TypeLookupService/TypeLookupServicePublisher.cpp
    :language: C++
    :lines: 58-62

Next, we register the type in the participant:

1. Generate the dynamic type through ``generate_helloworld_type_()`` explained below.
2. Set the data type.
3. Create the ``TypeSupport`` with the dynamic type previously created.
4. Configure the ``type`` to fill automatically the ``TypeInformation`` and not ``TypeObject`` to be compliant with `DDS-XTypes 1.2. <https://www.omg.org/spec/DDS-XTypes/1.2>`_ standard.

.. literalinclude:: ../../../resources/dds/TypeLookupService/TypeLookupServicePublisher.cpp
    :language: C++
    :lines: 73-95

The function ``generate_helloworld_type_()`` returns the dynamic type generated with the ``TypeObject`` and ``TypeIdentifier`` of the type.

.. literalinclude:: ../../../resources/dds/TypeLookupService/TypeLookupServicePublisher.cpp
    :language: C++
    :lines: 256-271

Then we initialized the Publisher, DDS Topic and DDS DataWriter.

To make the publication, the public member function ``publish()`` is implemented:

1. It creates the variable that will contain the user data, ``dynamic_data_``.
2. Fill that variable with the function ``fill_helloworld_data_(msg)``, explained below.

.. literalinclude:: ../../../resources/dds/TypeLookupService/TypeLookupServicePublisher.cpp
    :language: C++
    :lines: 229-254

The function ``fill_helloworld_data_()`` returns the data to be sent with the information filled in.

First, the ``Dynamic_ptr`` that will be filled in and returned is created.
Using the ``DynamicDataFactory`` we create the data that corresponds to our data type.
Finally, data variables are assigned, in this case, ``index`` and ``message``.

.. literalinclude:: ../../../resources/dds/TypeLookupService/TypeLookupServicePublisher.cpp
    :language: C++
    :lines: 293-307

**************
DDS Subscriber
**************

The DDS Subscriber is acting as a client of types, i.e. the subscriber will not know the types beforehand and it will discovery the data type via the type lookup service implemented on the publisher side.

Examining the code
==================

This section explains the C++ source code of the DDS Subscriber, which can also be found `here <https://github.com/eProsima/DDS-Record-Replay/blob/v0.4.0/resources/dds/TypeLookupService>`_.

The private data members of the class defines the DDS Topic, DDS Topic type and DynamicType.

.. literalinclude:: ../../../resources/dds/TypeLookupService/TypeLookupServiceSubscriber.h
    :language: C++
    :lines: 105-110

The next lines show the constructor of the ``TypeLookupServiceSubscriber`` class that implements the subscriber setting the topic name as the one configured in the
publisher side.

.. literalinclude:: ../../../resources/dds/TypeLookupService/TypeLookupServiceSubscriber.cpp
    :language: C++
    :lines: 45-53

The ``DomainParticipantQos`` are defined inside the ``TypeLookupServiceSubscriber`` constructor.
As the subscriber act as a client of types, set the QoS in order to receive this information.
Set ``use_client`` to ``true`` and ``use_server`` to ``false``.

.. literalinclude:: ../../../resources/dds/TypeLookupService/TypeLookupServiceSubscriber.cpp
    :language: C++
    :lines: 57-61

Then, the Subscriber is initialized.

Inside ``on_data_available()`` callback function the ``DynamicData_ptr`` is created, which will be filled with the actual data received.

As in the subscriber, the ``DynamicDataFactory`` is used for the creation of the data that corresponds to our data type.

.. literalinclude:: ../../../resources/dds/TypeLookupService/TypeLookupServiceSubscriber.cpp
    :language: C++
    :lines: 143-170

The function ``on_type_information_received()`` detects if new topic information has been received in order to proceed to register the topic in case it has the same name as the expected one.
To register a remote topic, function ``register_remote_type_callback_()`` is used.
Once the topic has been discovered and registered, it is created a DataReader on this topic.

.. literalinclude:: ../../../resources/dds/TypeLookupService/TypeLookupServiceSubscriber.cpp
    :language: C++
    :lines: 172-212

The function ``register_remote_type_callback_()``, which is in charge of register the topic received, is explained below.
First, it creates a ``TypeSupport`` with the corresponding type and registers it into the participant.
Then, it creates the DDS Topic with the topic name set in the creation of the Subscriber and the topic type previously registered.
Finally, it creates the DataReader of that topic.

.. literalinclude:: ../../../resources/dds/TypeLookupService/TypeLookupServiceSubscriber.cpp
    :language: C++
    :lines: 278-304

***********************
Running the application
***********************

Open two terminals:

* In the first terminal, run the DDS Publisher:

    .. code-block:: bash

        source install/setup.bash
        cd DDS-Record-Replay/build/TypeLookupService
        ./TypeLookupService --entity publisher

* In the second terminal, run the DDS Subscriber:

.. code-block:: bash

        source install/setup.bash
        cd DDS-Record-Replay/build/TypeLookupService
        ./TypeLookupService --entity subscriber

At this point, we observe that the data published reach the subscriber and it can access to the content of the sample received.

.. figure:: /rst/figures/basic_publisher_subscriber.png
