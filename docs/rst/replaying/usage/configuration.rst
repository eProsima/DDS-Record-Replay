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
This is the list of topics that |ddsreplayer| will replay, i.e. only recorded data under the topics matching the expressions in the ``allowlist`` will be published by |ddsreplayer|.

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

Built-in Topics
^^^^^^^^^^^^^^^

As seen in :ref:`recorder_topic_filtering`, a |ddsrecorder| uses the QoS of the first Publisher/Subscriber found in every recorded topic, unless manually defined in the :ref:`built-in topics <recorder_builtin_topics>` list.
This QoS information is stored in the MCAP file along with the user data, and thus a |ddsreplayer| instance is able to publish recorded data preserving the original QoS.

However, the user has the option to manually set the QoS of any topic to be played back through the replayer's builtin-topics list.
The builtin-topics list is defined in the same form as the ``allowlist`` and ``blocklist``.

This feature also allows to manually force the QoS of a specific topic, so the entities created in such topic follows the specified QoS and not the one first discovered.

Topic Quality of Service
""""""""""""""""""""""""

For every topic contained in this list, both ``name`` and ``type`` must be specified and contain no wildcard characters.
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

    *   - Key
        - ``keyed``
        - *bool*
        - ``false``
        - Topic with / without key

**Example of usage:**

    .. code-block:: yaml

        builtin-topics:
          - name: HelloWorldTopic
            type: HelloWorld
            qos:
              reliability: true       # Use QoS RELIABLE
              durability: true        # Use QoS TRANSIENT_LOCAL
              partitions: true        # Topic with partitions
              ownership: false        # Use QoS SHARED_OWNERSHIP_QOS
              keyed: true             # Topic with key


.. _replayer_usage_configuration_domain_id:

DDS Domain
^^^^^^^^^^

Tag ``domain`` configures the :term:`Domain Id`.

.. code-block:: yaml

    domain: 101


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

        - Whether to intrepet the provided datetime |br| as local (``true``)
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

End Time
^^^^^^^^

As with ``begin-time``, a user can discard messages recorded/sent after a specific timepoint set through the ``end-time`` tag, which follows the format described in :ref:`Begin Time <replayer_replay_configuration_begintime>`.

Start Replay Time
^^^^^^^^^^^^^^^^^

This configuration option (``start-replay-time``) allows to start replaying data at a certain timepoint following the format described in :ref:`Begin Time <replayer_replay_configuration_begintime>`.
If the provided timepoint already expired, the replayer starts publishing messages right away.

Playback Rate
^^^^^^^^^^^^^

By default, data is replayed at the same rate it was published/received.
However, a user might be interested in playing messages back at a rate different than the original one.
This can be accomplished through the playback ``rate`` tag, which accepts positive float values (e.g. 0.5 <--> half speed || 2 <--> double speed).

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
            keyed: false
            partitions: true
            ownership: false

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

    specs:
      threads: 8
      wait-all-acked-timeout: 10
