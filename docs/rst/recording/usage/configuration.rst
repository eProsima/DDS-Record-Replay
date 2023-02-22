.. include:: ../../exports/alias.include

.. _usage_configuration:

#############
Configuration
#############

.. contents::
    :local:
    :backlinks: none
    :depth: 2


DDS Recorder Configuration
==========================

A |ddsrecorder| is configured by a *.yaml* configuration file.
This *.yaml* file contains all the information regarding the DDS interface configuration, recording parameters, and |ddsrecorder| specifications.
Thus, this file has four major configuration groups:

* ``dds``: configuration related to DDS communication.
* ``recorder``: configuration of data writing in the database.
* ``remote-controller``: configuration of the remote controller of the |ddsrecorder|.
* ``specs``: configuration of the internal operation of the |ddsrecorder|.


.. _dds_recorder_configuration_dds_configuration:

DDS Configuration
-----------------

Configuration related to DDS communication.

.. _topic_filtering:

Topic Filtering
^^^^^^^^^^^^^^^

|ddsrecorder| includes a mechanism to automatically detect which topics are being used in a DDS network.
By automatically detecting these topics, a |ddsrecorder| creates internal DDS :term:`Readers<DataReader>` for each topic in order to record the data published on each discovered topic.

.. note::

    |ddsrecorder| entities are created with the QoS of the first Subscriber found in this Topic.

|ddsrecorder| allows filtering of DDS :term:`Topics<Topic>`, that is, it allows to define the DDS Topics' data that is going to be recorder by the application.
This way, it is possible to define a set of rules in |ddsrecorder| to filter those data samples the user does not wish to save.

It is not mandatory to define such set of rules in the configuration file.
In this case, a |ddsrecorder| will save all the data published under the topics that it automatically discovers within the DDS network to which it connects.

To define these data filtering rules based on the Topics to which they belong, the following lists are available:

* Allowed topics list (``allowlist``)
* Block topics list (``blocklist``)

These lists of topics stated above are defined by a tag in the *YAML* configuration file, which defines a *YAML* vector (``[]``).
This vector contains the list of topics for each filtering rule.
Each Topic is determined by its entries ``name``, ``type`` and ``keyed``, with only the first one being mandatory.

.. list-table::
    :header-rows: 1

    *   - Topic entries
        - Data type
        - Default value

    *   - ``name``
        - ``string``
        - \-

    *   - ``type``
        - ``string``
        - ``"*"``

    *   - ``keyed``
        - ``bool``
        - ``false``

The entry ``keyed`` determines whether the corresponding topic is `keyed <https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/topic/typeSupport/typeSupport.html#data-types-with-a-key>`_ or not.
See :term:`Topic` section for further information about the topic.

.. note::

    Placing quotation marks around values in a YAML file is generally optional.
    However, values containing wildcard characters must be enclosed by single or double quotation marks.

Allow topic list
""""""""""""""""
This is the list of topics that |ddsrecorder| will record, i.e. the data published under the topics matching the expressions in the ``allowlist`` will be saved by |ddsrecorder|.

.. note::

    If no ``allowlist`` is provided, data will be recorded for all topics (unless filtered out in ``blocklist``).


Block topic list
""""""""""""""""
This is the list of topics that the |ddsrecorder| will block, that is, all data published under the topics matching the filters specified in the ``blocklist`` will be discarded by the |ddsrecorder| and therefore will not be recorded.

This list takes precedence over the ``allowlist``.
If a topic matches an expression both in the ``allowlist`` and in the ``blocklist``, the ``blocklist`` takes precedence, causing the data under this topic to be discarded.

**Example of usage - Allowlist and blocklist collision:**

    In the following example, the ``HelloWorldTopic`` topic is both in the ``allowlist`` and (implicitly) in the
    ``blocklist``, so according to the ``blocklist`` preference rule this topic is blocked.
    Moreover, only the topics present in the allowlist are relayed, regardless of whether more topics are dynamically
    discovered in the DDS network.
    In this case the forwarded topics are ``AllowedTopic1`` with data type ``Allowed``
    and ``AllowedTopic2`` regardless of its data type.

    .. code-block:: yaml

        allowlist:
        - name: AllowedTopic1
            type: Allowed
        - name: AllowedTopic2
            type: "*"
        - name: HelloWorldTopic
            type: HelloWorld

        blocklist:
        - name: "*"
            type: HelloWorld

Built-in Topics
^^^^^^^^^^^^^^^

Apart from the dynamic DDS topics discovered in the network, the discovery phase can be accelerated by using the builtin topic list (``builtin-topics``).
By defining topics in this list, the |ddsrecorder| will create the DataWriters and DataReaders in recorder initialization.

The builtin-topics list is defined in the same form as the ``allowlist`` and ``blocklist``.

This feature also allows to manually force the QoS of a specific topic, so the entities created in such topic follows the specified QoS and not the one first discovered.

Topic Quality of Service
""""""""""""""""""""""""

For every topic contained in this list, both ``name`` and ``type`` must be specified and contain no wildcard characters.
The entry ``keyed`` is optional, and defaults to ``false``.
Apart from these values, the tag ``qos`` under each topic allows to configure the following values:

.. list-table::
    :header-rows: 1

    *   - Quality of Service
        - Yaml tag
        - Data type
        - Default value
        - QoS set

    *   - Reliability
        - ``reliability``
        - *bool*
        - ``false``
        - ``RELIABLE`` / ``BEST_EFFORT``

    *   - Durability
        - ``durability``
        - *bool*
        - ``false``
        - ``TRANSIENT_LOCAL`` / ``VOLATILE``

    *   - History Depth
        - ``depth``
        - *integer*
        - *default value*
        - -

    *   - Partitions
        - ``partitions``
        - *bool*
        - ``false``
        - Topic with / without partitions

    *   - Ownership
        - ``ownership``
        - *bool*
        - ``false``
        - ``EXCLUSIVE_OWNERSHIP_QOS`` / ``SHARED_OWNERSHIP_QOS``

**Example of usage:**

    .. code-block:: yaml

        builtin-topics:
          - name: HelloWorldTopic
            type: HelloWorld
            qos:
              reliability: true  # Use QoS RELIABLE
              durability: true   # Use QoS TRANSIENT_LOCAL
              depth: 100         # Use History Depth 100
              partitions: true   # Topic with partitions
              ownership: false   # Use QoS SHARED_OWNERSHIP_QOS


.. _usage_configuration_domain_id:

DDS Domain
^^^^^^^^^^

Tag ``domain`` configures the :term:`Domain Id`.

.. code-block:: yaml

    domain: 101


Recorder Configuration
----------------------

Configuration of data writing in the database.


Output File
^^^^^^^^^^^

The recorder output file does support the following configurations:

.. list-table::
    :header-rows: 1

    *   - Parameter
        - Tag
        - Description
        - Data type
        - Default value

    *   - File extension
        - ``extension``
        - Configure the file extension for the output file.
        - ``string``
        - ``.mcap``

    *   - File path
        - ``path``
        - Configure the path to save the output file.
        - ``string``
        - ``.``

    *   - File name
        - ``filename``
        - Configure the name of the output file.
        - ``string``
        -

Downsampling
^^^^^^^^^^^^

Reduces the sampling rate of the received data by keeping the first sample and then all *n-th* samples after the first sample, where *n* is the value specified in ``downsampling``.

Buffer size
^^^^^^^^^^^

``buffer-size`` indicates the number of samples to be stored in the process memory before the dump to disk.
This avoids disk access each time a sample is received.
This parameter applies to each topic independently, that is, ``buffer-size`` samples of each topic will be buffered before writing them to the database.

Event Duration
^^^^^^^^^^^^^^

The |ddsrecorder| can be configured to continue saving data when it is in pause mode.
Thus, when the |ddsrecorder| receives the event from the remote controller, it will save the samples received in the time period configured in ``event-duration`` in the database and restart the timer.

In other words, the ``event-duration`` acts as a sliding time window that allows to save the collected samples in this time window only when the remote controller event is received.
This way, a |ddsrecorder| is working in pause mode, recording samples in the database only when the event from the remote controller occurs.

Remote Controller
-----------------

Configuration of the DDS remote control system.
Please refer to `remote_control` for further information on how to use the |ddsrecorder| remotely.
The supported configurations are:

.. list-table::
    :header-rows: 1

    *   - Parameter
        - Tag
        - Description
        - Data type
        - Default value

    *   - Enable
        - ``enable``
        - Enable DDS remote control system topics.
        - ``boolean``
        - ``false``

    *   - DDS Domain
        - ``domain``
        - DDS Domain of the DDS remote control system.
        - ``string``
        - ``.``


Specs Configuration
-------------------

The internals of the |ddsrecorder| can be configured using the ``specs`` optional tag that contains certain options related with the overall configuration of the |ddsrecorder| instance to run.
The values available to configure are:

Number of Threads
^^^^^^^^^^^^^^^^^

``specs`` supports a ``threads`` optional value that allows the user to set a maximum number of threads for the internal :code:`ThreadPool`.
This ThreadPool allows to limit the number of threads spawned by the application.
This improves the performance of the internal data communications.

This value should be set by each user depending on each system characteristics.
In case this value is not set, the default number of threads used is :code:`12`.

Maximum Number of Pending Samples
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

It is possible that the |ddsrecorder| starts receiving data from a topic that it has not yet registered, i.e. a topic for which it does not know the data type.
In order not to discard the samples received from this topic, it is possible to keep a limited number of samples in an internal circular buffer that stores those samples that do not yet have a known data type.
The ``max-pending-samples`` parameter allows to configure the size of this circular buffer for each topic that is discovered.
The default value is equal to ``10`` samples.

.. _history_depth_configuration:

Maximum History Depth
^^^^^^^^^^^^^^^^^^^^^

``specs`` supports a ``max-depth`` optional value that configures the history size of the *Fast DDS* internal entities.
By default, the depth of every RTPS History instance is :code:`5000`, which sets a constraint on the maximum number of samples a |ddsrecorder| instance can deliver to late joiner Readers configured with ``TRANSIENT_LOCAL`` `DurabilityQosPolicyKind <https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/core/policy/standardQosPolicies.html#durabilityqospolicykind>`_.
Its value should be decreased when the sample size and/or number of created endpoints (increasing with the number of topics and |ddsrecorder| participants) are as big as to cause memory exhaustion issues.
Likewise, one may choose to increase this value if wishing to deliver a greater number of samples to late joiners and enough memory is available.


.. _usage_configuration_general_example:

General Example
---------------

A complete example of all the configurations described on this page can be found below.

.. code-block:: yaml

    dds:
      domain: 0

      allowlist:
        - name: "topic_name"
          type: "topic_type"

      blocklist:
        - name: "topic_name"
          type: "topic_type"

      builtin-topics:
        - name: "HelloWorldTopic"
          type: "HelloWorld"
          qos:
            reliability: true
            durability: true
            depth: 100
            partitions: true
            ownership: false

    recorder:
      output:
        extension: ".mcap"
        filename: "output"
        path: "."

      downsampling: 0.1
      buffer-size: 50
      event-duration: 60

    remote-controller:
      enable: true
      domain: 10

    specs:
      threads: 8
      max-depth: 100
      max-pending-samples: 10


.. _usage_fastdds_configuration:

Fast DDS Configuration
======================

As mentioned before, the |ddsrecorder| requires the topic types in order to be able to record the data of such topics.
This requires that the user application needs to be configured to send the required type information.
However, *Fast DDS* does not send the data type information by default, it must be configured to do so.
First of all, when generating the topic types using *eProsima Fast DDS Gen*, the option ``-typeobject`` must be added in order to generate the needed code to fill the ``TypeObject`` data.

For native types (data types that does not rely in other data types) this is enough, as *Fast DDS* will send the ``TypeObject`` by default.
However, for more complex types, it is required to use ``TypeInformation`` mechanism.
In the *Fast DDS* ``DomainParticipant`` set the following QoS in order to send this information:

.. code-block:: c

    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.typelookup_config.use_server = true;

.. TODO: add a reference to dynamic types tutorial
