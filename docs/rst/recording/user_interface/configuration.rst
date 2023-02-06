.. include:: ../../exports/alias.include

.. _user_manual_configuration:

#############
Configuration
#############

A |ddsrecorder| is configured by a *.yaml* configuration file.
This *.yaml* file contains all the information regarding the |ddsrecorder| configuration, such as topics filtering
and :term:`Participants <Participant>` configurations.

.. contents::
    :local:
    :backlinks: none
    :depth: 2


.. _thread_configuration:

Specs Configuration
===================

The YAML Configuration supports a ``specs`` **optional** tag that contains certain options related with the
overall configuration of the DDS Recorder instance to run.
The values available to configure are:

Number of Threads
-----------------

``specs`` supports a ``threads`` **optional** value that allows the user to set a maximum number of threads
for the internal :code:`ThreadPool`.
This ThreadPool allows to limit the number of threads spawned by the application.
This improves the performance of the data transmission between Participants.

This value should be set by each user depending on each system characteristics.
In case this value is not set, the default number of threads used is :code:`12`.

.. _history_depth_configuration:

Maximum History Depth
---------------------

``specs`` supports a ``max-depth`` **optional** value that configures the history size
of the Fast DDS internal entities.
By default, the depth of every RTPS History instance is :code:`5000`, which sets a constraint on the maximum number of
samples a |ddsrecorder| instance can deliver to late joiner Readers configured with ``TRANSIENT_LOCAL``
`DurabilityQosPolicyKind <https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/core/policy/standardQosPolicies.html#durabilityqospolicykind>`_.
Its value should be decreased when the sample size and/or number of created endpoints (increasing with the number of
topics and |ddsrecorder| participants) are as big as to cause memory exhaustion issues.
Likewise, one may choose to increase this value if wishing to deliver a greater number of samples to late joiners and
enough memory is available.

.. _topic_filtering:

Built-in Topics
===============

Apart from the dynamic creation of Endpoints in DDS Topics discovered,
the discovery phase can be accelerated by using the builtin topic list (``builtin-topics``).
By defining topics in this list, the DDS recorder will create the DataWriters and DataReaders in recorder initialization.
This feature also allows to manually force the QoS of a specific topic, so the entities created in such topic
follows the specified QoS and not the one first discovered.

Topic Quality of Service
------------------------

For every topic contained in this list, both ``name`` and ``type`` must be specified and contain no wildcard
characters. The entry ``keyed`` is optional, and defaults to ``false``.
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


Topic Filtering
===============

|ddsrecorder| includes a mechanism to automatically detect which topics are being used in a DDS network.
By automatically detecting these topics, a |ddsrecorder| creates internal DDS :term:`Writers<DataWriter>`
and :term:`Readers<DataReader>` for each topic and for each Participant in order to relay the data published on each
discovered topic.

.. note::

    DDS Recorder entities are created with the QoS of the first Subscriber found in this Topic.

|ddsrecorder| allows filtering of DDS :term:`Topics<Topic>`, that is, it allows to define which DDS Topics are going to be
relayed by the application.
This way, it is possible to define a set of rules in |ddsrecorder| to filter those data samples the user does not wish to
forward.

It is not mandatory to define such set of rules in the configuration file. In this case, a DDS Recorder will forward all
the data published under the topics that it automatically discovers within the DDS network to which it connects.

To define these data filtering rules based on the Topics to which they belong, three lists are available:

* Allowed topics list (``allowlist``)
* Block topics list (``blocklist``)
* Builtin topics list (``builtin-topics``)

These three lists of topics listed above are defined by a tag in the *YAML* configuration file, which defines a
*YAML* vector (``[]``).
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
        - Both ``true`` and ``false``

The entry ``keyed`` determines whether the corresponding topic is `keyed <https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/topic/typeSupport/typeSupport.html#data-types-with-a-key>`_
or not. See :term:`Topic` section for further information about the topic.

.. note::

    Tags ``allowlist``, ``blocklist`` and ``builtin-topics`` must be at yaml base level (it must not be inside any
    other tag).

.. note::

    Placing quotation marks around values in a YAML file is generally optional. However, values containing wildcard
    characters must be enclosed by single or double quotation marks.

Allow topic list (``allowlist``)
--------------------------------
This is the list of topics that |ddsrecorder| will forward, i.e. the data published under the topics matching the
expressions in the ``allowlist`` will be relayed by |ddsrecorder|.

.. note::

    If no ``allowlist`` is provided, data will be forwarded for all topics (unless filtered out in ``blocklist``).


Block topic list (``blocklist``)
--------------------------------
This is the list of topics that the |ddsrecorder| will block, that is, all data published under the topics matching the
filters specified in the ``blocklist`` will be discarded by the |ddsrecorder| and therefore will not be relayed.

This list takes precedence over the ``allowlist``.
If a topic matches an expression both in the ``allowlist`` and in the ``blocklist``, the ``blocklist`` takes precedence,
causing the data under this topic to be discarded.


Examples of usage
-----------------

The following is an example of how to use the ``allowlist``, ``blocklist`` and ``builtin-topics`` configurations to
setup the |ddsrecorder| filtering rules.

Dynamic topic discovery example
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This example shows how the |ddsrecorder| is initially configured to forward the ``rt/chatter`` topic (default ROS 2
topic for ``talker`` and ``listener``) with type name ``std_msgs::msg::dds_::String_``, while the rest of the
topics in the DDS network are expected to be dynamically discovered.
Additionally, two rules are specified in the ``blocklist`` in order to filter out messages of no interest to the user
(in this case ROS2 services related topics).

.. code-block:: yaml

    builtin-topics:
      - name: rt/chatter
        type: std_msgs::msg::dds_::String_

    blocklist:
      - name: "rq/*"
      - name: "rr/*"


Allowlist and blocklist collision
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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

.. _user_manual_configuration_domain_id:

Domain Id
=========

Tag ``domain`` configures the :term:`Domain Id` of a specific Participant.
Be aware that some Participants (e.g. Discovery Servers) does not need a Domain Id configuration.

.. code-block:: yaml

    domain: 101


.. _user_manual_configuration_general_example:

General Example
===============

A complete example of all the configurations described on this page can be found below.

.. code-block:: yaml

    # Version Latest
    version: v3.0

    specs:
      threads: 10
      max-depth: 1000

    # Relay topic rt/chatter and type std_msgs::msg::dds_::String_
    # Relay topic HelloWorldTopic and type HelloWorld

    builtin-topics:

      - name: rt/chatter
        type: std_msgs::msg::dds_::String_

      - name: HelloWorldTopic
        type: HelloWorld
        qos:
          reliability: true
          durability: true

    # Do not allow ROS2 services

    blocklist:
      - name: "rr/*"
      - name: "rq/*"

    # Simple DDS Participant in domain 3

    domain: 3                       # DomainId = 3
