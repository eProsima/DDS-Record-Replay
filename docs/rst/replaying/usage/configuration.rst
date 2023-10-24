.. include:: ../../exports/alias.include
.. include:: ../../exports/roles.include

.. _replayer_usage_configuration:

#############
Configuration
#############

.. contents::
    :local:
    :backlinks: none
    :depth: 2


DDS Replayer Configuration
==========================

A |ddsreplayer| is configured by a *.yaml* configuration file.
This *.yaml* file contains all the information regarding the DDS interface configuration, playback parameters, and |ddsreplayer| specifications.
Thus, this file has four major configuration groups:

* ``dds``: configuration related to DDS communication.
* ``replayer``: configuration with data playback parameters.
* ``specs``: configuration of the internal operation of the |ddsreplayer|.


.. _replayer_dds_recorder_configuration_dds_configuration:

DDS Configuration
-----------------

Configuration related to DDS communication.

.. _replayer_builtin_topics:

Built-in Topics
^^^^^^^^^^^^^^^

The discovery phase can be accelerated by listing topics under the ``builtin-topics`` tag.
The |ddsreplayer| will create the DataWriters and DataReaders for these topics in the |ddsreplayer| initialization.
The :ref:`Topic QoS <replayer_topic_qos>` for these topics can be manually configured with a :ref:`Manual Topic <replayer_manual_topics>`; if a :ref:`Topic QoS <replayer_topic_qos>` is not configured, it will take its default value.

The ``builtin-topics`` must specify a ``name`` and ``type`` without wildcard characters.

.. code-block:: yaml

    builtin-topics:
      - name: HelloWorldTopic
        type: HelloWorld

.. _replayer_topic_filtering:

Topic Filtering
^^^^^^^^^^^^^^^

As seen in :ref:`DDS Recorder topic filtering <recorder_topic_filtering>`, a user can define a set of rules to only record DDS :term:`Topics<Topic>` of interest.

In addition to the filters applied to |ddsrecorder| when recording, |ddsreplayer| also allows filtering of DDS :term:`Topics<Topic>`.
That is, it allows to define the DDS Topics' data that is going to be replayed by the application.
This way, it is possible to define a set of rules in |ddsreplayer| to filter those data samples the user does not wish to replay.

It is not mandatory to define such set of rules in the configuration file.
In this case, a |ddsreplayer| will publish all data stored in the provided input MCAP file.

To define these data filtering rules based on the Topics to which they belong, the following lists are available:

* Allowed topics list (``allowlist``)
* Block topics list (``blocklist``)

These lists of topics stated above are defined by a tag in the *YAML* configuration file, which defines a *YAML* vector (``[]``).
This vector contains the list of topics for each filtering rule.
Each Topic is determined by its entries ``name`` and ``type``, with only the first one being mandatory.

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

See :term:`Topic` section for further information about the topic.

.. note::

    Placing quotation marks around values in a YAML file is generally optional.
    However, values containing wildcard characters must be enclosed by single or double quotation marks.

Allow topic list
""""""""""""""""
This is the list of topics that the |ddsreplayer| will replay, i.e. only recorded data under the topics matching the expressions in the ``allowlist`` will be published by |ddsreplayer|.

.. note::

    If no ``allowlist`` is provided, data will be replayed for all topics (unless filtered out in ``blocklist``).

.. _replayer_topic_filtering_blocklist:

Block topic list
""""""""""""""""
This is the list of topics that the |ddsreplayer| will block, that is, all input data under the topics matching the filters specified in the ``blocklist`` will be discarded by the |ddsreplayer| and therefore will not be published.

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

.. _replayer_topic_qos:

Topic QoS
^^^^^^^^^

The following is the set of QoS that are configurable for a topic.
For more information on topics, please read the `Fast DDS Topic <https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/topic/topic.html>`_ section.

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

    *   - Ownership
        - ``ownership``
        - *bool*
        - ``false``
        - ``EXCLUSIVE_OWNERSHIP_QOS`` / ``SHARED_OWNERSHIP_QOS``

    *   - Partitions
        - ``partitions``
        - *bool*
        - ``false``
        - Topic with / without partitions

    *   - Key
        - ``keyed``
        - *bool*
        - ``false``
        - Topic with / without key

    *   - Max Transmission Rate
        - ``max-tx-rate``
        - *float*
        - ``0`` (unlimited)
        - :ref:`replayer_max_tx_rate`

.. _replayer_history_depth:

History Depth
^^^^^^^^^^^^^

The ``history-depth`` tag configures the history depth of the Fast DDS internal entities.
By default, the depth of every RTPS History instance is :code:`5000`, which sets a constraint on the maximum number of samples a |ddsreplayer| instance can deliver to late joiner Readers configured with ``TRANSIENT_LOCAL`` `DurabilityQosPolicyKind <https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/core/policy/standardQosPolicies.html#durabilityqospolicykind>`_.
Its value should be decreased when the sample size and/or number of created endpoints (increasing with the number of topics and |ddsreplayer| participants) are big enough to cause memory exhaustion issues.
If enough memory is available, however, the ``history-depth`` could be increased to deliver a greater number of samples to late joiners.

.. _replayer_max_tx_rate:

Max Transmission Rate
"""""""""""""""""""""

The ``max-tx-rate`` tag limits the frequency [Hz] at which samples are sent by discarding messages transmitted before :code:`1/max-tx-rate` seconds have passed since the last sent message.
It only accepts non-negative numbers.
By default it is set to ``0``; it sends samples at an unlimited transmission rate.

.. note::

    The ``max-tx-rate`` tag can be set (in order of precedence) for topics, for participants, and globally in specs.

.. _replayer_manual_topics:

Manual Topics
^^^^^^^^^^^^^

A subset of QoSs can be manually configured for a specific topic under the tag ``topics``.
The tag ``topics`` has a required ``name`` tag that accepts wildcard characters.
It also has two optional tags: a ``type`` tag that accepts wildcard characters, and a ``qos`` tag with the QoSs that the user wants to manually configure.
If a ``qos`` is not manually configured, it will get its value by discovery.

**Example of usage**

.. code-block:: yaml

    topics:
      - name: temperature/*
        type: temperature/types/*
        qos:
          max-tx-rate: 15

.. _replayer_usage_configuration_domain_id:

DDS Domain
^^^^^^^^^^

Tag ``domain`` configures the :term:`Domain Id`.

.. code-block:: yaml

    domain: 101


.. _replayer_ignore_participant_flags:

Ignore Participant Flags
^^^^^^^^^^^^^^^^^^^^^^^^

A set of discovery traffic filters can be defined in order to add an extra level of isolation.
This configuration option can be set through the ``ignore-participant-flags`` tag:

.. code-block:: yaml

    ignore-participant-flags: no_filter                          # No filter (default)
    # or
    ignore-participant-flags: filter_different_host              # Discovery traffic from another host is discarded
    # or
    ignore-participant-flags: filter_different_process           # Discovery traffic from another process on same host is discarded
    # or
    ignore-participant-flags: filter_same_process                # Discovery traffic from own process is discarded
    # or
    ignore-participant-flags: filter_different_and_same_process  # Discovery traffic from own host is discarded

See `Ignore Participant Flags <https://fast-dds.docs.eprosima.com/en/latest/fastdds/discovery/general_disc_settings.html?highlight=ignore%20flags#ignore-participant-flags>`_ for more information.


.. _replayer_custom_transport_descriptors:

Custom Transport Descriptors
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

By default, |ddsreplayer| internal participants are created with enabled `UDP <https://fast-dds.docs.eprosima.com/en/latest/fastdds/transport/udp/udp.html>`_ and `Shared Memory <https://fast-dds.docs.eprosima.com/en/latest/fastdds/transport/shared_memory/shared_memory.html>`_ transport descriptors.
The use of one or the other for communication will depend on the specific scenario, and whenever both are viable candidates, the most efficient one (Shared Memory Transport) is automatically selected.
However, a user may desire to force the use of one of the two, which can be accomplished via the ``transport`` configuration tag.

.. code-block:: yaml

    transport: builtin    # UDP & SHM (default)
    # or
    transport: udp        # UDP only
    # or
    transport: shm        # SHM only

.. warning::

    When configured with ``transport: shm``, |ddsreplayer| will only communicate with applications using Shared Memory Transport exclusively (with disabled UDP transport).


.. _replayer_interface_whitelist:

Interface Whitelist
^^^^^^^^^^^^^^^^^^^

Optional tag ``whitelist-interfaces`` allows to limit the network interfaces used by UDP and TCP transport.
This may be useful to only allow communication within the host (note: same can be done with :ref:`replayer_ignore_participant_flags`).
Example:

.. code-block:: yaml

    whitelist-interfaces:
      - "127.0.0.1"    # Localhost only

See `Interface Whitelist <https://fast-dds.docs.eprosima.com/en/latest/fastdds/transport/whitelist.html>`_ for more information.

.. warning::

    When providing an interface whitelist, external participants with which communication is desired must also be configured with interface whitelisting.


Replay Configuration
--------------------

Configuration of data playback settings.


Input File
^^^^^^^^^^

The path to the file, set through the ``input-file`` configuration tag.
When the input file is specified both through CLI argument and YAML configuration file, the former takes precedence.

.. _replayer_replay_configuration_begintime:

Begin Time
^^^^^^^^^^

By default, all data stored in the provided MCAP file is played back.
However, a user might be interested in only replaying data relative to a specific time frame.
``begin-time`` and ``end-time`` configuration options can be leveraged for this purpose, and their format is as follows:

.. list-table::
    :header-rows: 1

    *   - Parameter
        - Tag
        - Description
        - Data type
        - Default value

    *   - Use local time zone
        - ``local``

        - Whether to interpret the provided datetime |br| as local (``true``)
          or as a Greenwich Mean Time |br| (GMT/UTC +0)
          without Daylight Saving Time |br| (DST) considerations (``false``).
        - ``bool``
        - ``true``

    *   - Datetime Format
        - ``format``
        - Format followed by the provided datetime.
        - ``string``
        - ``"%Y-%m-%d_%H-%M-%S"``

    *   - Datetime
        - ``datetime``
        - Datetime (seconds precision).
        - ``string``
        -

    *   - Milliseconds
        - ``milliseconds``
        - Milliseconds.
        - ``integer``
        - ``0``

    *   - Microseconds
        - ``microseconds``
        - Microseconds.
        - ``integer``
        - ``0``

    *   - Nanoseconds
        - ``nanoseconds``
        - Nanoseconds.
        - ``integer``
        - ``0``

Messages recorded/sent (see :ref:`Log Publish Time <recorder_usage_configuration_logpublishtime>`) before ``begin-time`` will not be played back by a |ddsreplayer| instance.

.. _replayer_replay_configuration_endtime:

End Time
^^^^^^^^

As with ``begin-time``, a user can discard messages recorded/sent after a specific timepoint set through the ``end-time`` tag, which follows the format described in :ref:`Begin Time <replayer_replay_configuration_begintime>`.

.. _replayer_replay_configuration_startreplaytime:

Start Replay Time
^^^^^^^^^^^^^^^^^

This configuration option (``start-replay-time``) allows to start replaying data at a certain timepoint following the format described in :ref:`Begin Time <replayer_replay_configuration_begintime>`.
If the provided timepoint already expired, the replayer starts publishing messages right away.

.. _replayer_replay_configuration_playbackrate:

Playback Rate
^^^^^^^^^^^^^

By default, data is replayed at the same rate it was published/received.
However, a user might be interested in playing messages back at a rate different than the original one.
This can be accomplished through the playback ``rate`` tag, which accepts positive float values (e.g. 0.5 <--> half speed || 2 <--> double speed).

.. _replayer_usage_configuration_max_tx_rate:

Max Transmission Rate
---------------------

The ``max-tx-rate`` tag limits the frequency [Hz] at which samples are sent by discarding messages transmitted before :code:`1/max-tx-rate` seconds have passed since the last sent message.
It only accepts non-negative numbers.
By default it is set to ``0``; it sends samples at an unlimited transmission rate.

.. note::

    The ``max-tx-rate`` tag can be set for topics and globally under the ``replayer`` tag.
    If both are set, the configuration under topics prevails.

.. _replayer_replay_configuration_replaytypes:

Replay Types
^^^^^^^^^^^^

By default, a |ddsreplayer| instance automatically sends all type information found in the provided MCAP file, which might be required for applications relying on :term:`Dynamic Types<DynamicTypes>`.
Nonetheless, a user can choose to avoid this by setting ``replay-types: false``, so only data samples are sent while their associated type information is disregarded.

Specs Configuration
-------------------

The internals of a |ddsreplayer| can be configured using the ``specs`` optional tag that contains certain options related with the overall configuration of the |ddsreplayer| instance to run.
The values available to configure are:

Number of Threads
^^^^^^^^^^^^^^^^^

``specs`` supports a ``threads`` optional value that allows the user to set a maximum number of threads for the internal :code:`ThreadPool`.
This ThreadPool allows to limit the number of threads spawned by the application.
This improves the performance of the internal data communications.

This value should be set by each user depending on each system characteristics.
In case this value is not set, the default number of threads used is :code:`12`.

Wait-for-acknowledgement Timeout
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The execution of a |ddsreplayer| instance ends when the last message contained in the provided input file is published (or the user manually aborts the process, see :ref:`replayer_usage_close_replayer`).
Note that this last message might be lost after publication, and if reliable `Reliability QoS <https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/core/policy/standardQosPolicies.html#reliabilityqospolicy>`__ is being used, a mechanism should be established to avoid this problematic situation.
For this purpose, the user can specify the maximum amount of milliseconds (``wait-all-acked-timeout``) to wait on closure until published messages are acknowledged by matched readers.
Its value is set to ``0`` by default (no wait).

.. _replayer_usage_configuration_general_example:

General Example
---------------

A complete example of all the configurations described on this page can be found below.

.. warning::

    This example can be used as a quick reference, but it may not be correct due to incompatibility or exclusive properties. **Do not take it as a working example**.

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

      topics:
        - name: temperature/*
          type: temperature/types/*
          qos:
            max-tx-rate: 15

      ignore-participant-flags: no_filter
      transport: builtin
      whitelist-interfaces:
        - "127.0.0.1"

    replayer:
      input-file: my_input.mcap

      begin-time:
        local: true
        datetime: 2023-04-10_10-37-50
        milliseconds: 100
        nanoseconds: 50

      end-time:
        format: "%H-%M-%S_%Y-%m-%d"
        local: true
        datetime: 10-39-11_2023-04-10
        milliseconds: 200

      start-replay-time:
        local: true
        datetime: 2023-04-12_12-00-00
        milliseconds: 500

      rate: 1.4
      replay-types: true

    specs:
      threads: 8
      wait-all-acked-timeout: 10

      qos:
        max-tx-rate: 20
