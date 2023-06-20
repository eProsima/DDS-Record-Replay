.. include:: ../../exports/alias.include
.. include:: ../../exports/roles.include

.. _recorder_usage_configuration:

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


.. _recorder_dds_recorder_configuration_dds_configuration:

DDS Configuration
-----------------

Configuration related to DDS communication.

.. _recorder_topic_filtering:

Topic Filtering
^^^^^^^^^^^^^^^

|ddsrecorder| includes a mechanism to automatically detect which topics are being used in a DDS network.
By automatically detecting these topics, a |ddsrecorder| creates internal DDS :term:`Readers<DataReader>` for each topic in order to record the data published on each discovered topic.

.. note::

    |ddsrecorder| entities are created with the QoS of the first Publisher/Subscriber found in this Topic, unless manually set in the :ref:`built-in topics <recorder_builtin_topics>` list.

|ddsrecorder| allows filtering of DDS :term:`Topics<Topic>`, that is, it allows to define the DDS Topics' data that is going to be recorded by the application.
This way, it is possible to define a set of rules in |ddsrecorder| to filter those data samples the user does not wish to save.

It is not mandatory to define such set of rules in the configuration file.
In this case, a |ddsrecorder| will save all the data published under the topics that it automatically discovers within the DDS network to which it connects.

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
This is the list of topics that |ddsrecorder| will record, i.e. the data published under the topics matching the expressions in the ``allowlist`` will be saved by |ddsrecorder|.

.. note::

    If no ``allowlist`` is provided, data will be recorded for all topics (unless filtered out in ``blocklist``).

.. _recorder_topic_filtering_blocklist:

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

.. _recorder_builtin_topics:

Built-in Topics
^^^^^^^^^^^^^^^

Apart from the dynamic DDS topics discovered in the network, the discovery phase can be accelerated by using the builtin topic list (``builtin-topics``).
By defining topics in this list, the |ddsrecorder| will create the DataWriters and DataReaders in recorder initialization.

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

    *   - Key
        - ``keyed``
        - *bool*
        - ``false``
        - Topic with / without key

    *   - Downsampling
        - ``downsampling``
        - *integer*
        - *default value*
        - Downsampling factor

    *   - Max Reception Rate
        - ``max-reception-rate``
        - *float*
        - *default value*
        - Maximum sample reception rate [Hz]

**Example of usage:**

    .. code-block:: yaml

        builtin-topics:
          - name: HelloWorldTopic
            type: HelloWorld
            qos:
              reliability: true       # Use QoS RELIABLE
              durability: true        # Use QoS TRANSIENT_LOCAL
              depth: 100              # Use History Depth 100
              partitions: true        # Topic with partitions
              ownership: false        # Use QoS SHARED_OWNERSHIP_QOS
              keyed: true             # Topic with key
              downsampling: 4         # Keep 1 of every 4 samples
              max-reception-rate: 10  # Discard messages if less than 100ms elapsed since the last sample was processed


.. _recorder_usage_configuration_domain_id:

DDS Domain
^^^^^^^^^^

Tag ``domain`` configures the :term:`Domain Id`.

.. code-block:: yaml

    domain: 101


.. _recorder_ignore_participant_flags:

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


.. _recorder_custom_transport_descriptors:

Custom Transport Descriptors
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

By default, |ddsrecorder| internal participants are created with enabled `UDP <https://fast-dds.docs.eprosima.com/en/latest/fastdds/transport/udp/udp.html>`_ and `Shared Memory <https://fast-dds.docs.eprosima.com/en/latest/fastdds/transport/shared_memory/shared_memory.html>`_ transport descriptors.
The use of one or the other for communication will depend on the specific scenario, and whenever both are viable candidates, the most efficient one (Shared Memory Transport) is automatically selected.
However, a user may desire to force the use of one of the two, which can be accomplished via the ``transport`` configuration tag.

.. code-block:: yaml

    transport: builtin    # UDP & SHM (default)
    # or
    transport: udp        # UDP only
    # or
    transport: shm        # SHM only

.. warning::

    When configured with ``transport: shm``, |ddsrecorder| will only communicate with applications using Shared Memory Transport exclusively (with disabled UDP transport).


.. _recorder_interface_whitelist:

Interface Whitelist
^^^^^^^^^^^^^^^^^^^

Optional tag ``whitelist-interfaces`` allows to limit the network interfaces used by UDP and TCP transport.
This may be useful to only allow communication within the host (note: same can be done with :ref:`recorder_ignore_participant_flags`).
Example:

.. code-block:: yaml

    whitelist-interfaces:
      - "127.0.0.1"    # Localhost only

See `Interface Whitelist <https://fast-dds.docs.eprosima.com/en/latest/fastdds/transport/whitelist.html>`_ for more information.

.. warning::

    When providing an interface whitelist, external participants with which communication is desired must also be configured with interface whitelisting.


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

    *   - File path
        - ``path``
        - Configure the path to save the output file.
        - ``string``
        - ``.``

    *   - File name
        - ``filename``
        - Configure the name of the output file.
        - ``string``
        - ``output``

When DDS Recorder application is launched (or when remotely controlled, every time a ``start`` command is received), a temporary file with ``filename`` name and ``.mcap.tmp~`` extension is created in ``path``.
This file is not readable until the application is terminated (or a ``stop`` / ``close`` command is received).
On such event, the temporal file is renamed to ``filename`` with ``.mcap`` extension in the same location, and is then ready to be processed.

Buffer size
^^^^^^^^^^^

``buffer-size`` indicates the number of samples to be stored in the process memory before the dump to disk.
This avoids disk access each time a sample is received.
By default, its value is set to ``100``.

.. _recorder_usage_configuration_event_window:

Event Window
^^^^^^^^^^^^

|ddsrecorder| can be configured to continue saving data when it is in paused mode.
Thus, when an event is triggered from the remote controller, samples received in the last ``event-window`` seconds are stored in the database.

In other words, the ``event-window`` acts as a sliding time window that allows to save the collected samples in this time window only when the remote controller event is received.
By default, its value is set to ``20`` seconds.

.. _recorder_usage_configuration_logpublishtime:

Log Publish Time
^^^^^^^^^^^^^^^^

By default (``log-publish-time: false``) received messages are stored in the MCAP file with ``logTime`` value equals to the reception timestamp.
Additionally, the timestamp corresponding to when messages were initially published (``publishTime``) is also included in the information dumped to MCAP files.
In some applications, it may be required to use the ``publishTime`` as ``logTime``, which can be achieved by providing the ``log-publish-time: true`` configuration option.

Max Reception Rate
^^^^^^^^^^^^^^^^^^

Limits the frequency [Hz] at which samples are processed, by discarding messages received before :code:`1/max-reception-rate` seconds have elapsed since the last processed message was received.
When specified, ``max-reception-rate`` is set for all topics without distinction, but a different value can also set for a particular topic under the ``qos`` configuration tag within the builtin-topics list.
This parameter only accepts non-negative values, and its default value is ``0`` (no limit).

Downsampling
^^^^^^^^^^^^

Reduces the sampling rate of the received data by keeping *1* out of every *n* samples received (per topic), where *n* is the value specified in ``downsampling``.
If ``max-reception-rate`` is also set, downsampling applies to messages that already managed to pass this filter.
When specified, this downsampling factor is set for all topics without distinction, but a different value can also set for a particular topic under the ``qos`` configuration tag within the builtin-topics list.
This parameter only accepts positive integer values, and its default value is ``1`` (no downsampling).

.. _recorder_usage_configuration_onlywithtype:

Only With Type
^^^^^^^^^^^^^^

By default, all (allowed) received messages are recorded regardless of whether their associated type information has been received.
However, a user can enforce that **only** samples whose type is received are recorded by setting ``only-with-type: true``.

.. _recorder_usage_configuration_remote_controller:

Remote Controller
-----------------

Configuration of the DDS remote control system.
Please refer to :ref:`Remote Control <recorder_remote_control>` for further information on how to use |ddsrecorder| remotely.
The supported configurations are:

.. list-table::
    :header-rows: 1

    *   - Parameter
        - Tag
        - Description
        - Data type
        - Default value
        - Possible values

    *   - Enable
        - ``enable``
        - Enable DDS remote |br|
          control system topics.
        - ``boolean``
        - ``true``
        - ``true`` |br|
          ``false``

    *   - DDS Domain
        - ``domain``
        - DDS Domain of the |br|
          DDS remote control |br|
          system.
        - ``integer``
        - DDS domain being |br|
          recorded
        - From ``0`` to ``255``

    *   - Initial state
        - ``initial-state``
        - Initial state of |br|
          |ddsrecorder|.
        - ``string``
        - ``RUNNING``
        - ``RUNNING`` |br|
          ``PAUSED`` |br|
          ``STOPPED``

    *   - Command Topic Name
        - ``command-topic-name``
        - Name of Controller |br|
          Command DDS Topic.
        - ``string``
        - ``/ddsrecorder/command``
        -

    *   - Status Topic Name
        - ``status-topic-name``
        - Name of Controller |br|
          Status DDS Topic.
        - ``string``
        - ``/ddsrecorder/status``
        -

Specs Configuration
-------------------

The internals of a |ddsrecorder| can be configured using the ``specs`` optional tag that contains certain options related with the overall configuration of the |ddsrecorder| instance to run.
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

It is possible that a |ddsrecorder| starts receiving data from a topic that it has not yet registered, i.e. a topic for which it does not know the data type.
In this case, messages are kept in an internal circular buffer until their associated type information is received, event on which they are written to disk.

However, the recorder execution might end before this event ever occurs.
Depending on configuration (see :ref:`recorder_usage_configuration_onlywithtype`), messages kept in the pending samples buffer will be stored or not on closure.
Hence, note that memory consumption would continuously grow whenever a sample with unknown type information is received.

To avoid the exhaustion of memory resources in such scenarios, a configuration option is provided which lets the user set a limit on memory usage.
The ``max-pending-samples`` parameter allows to configure the size of the aforementioned circular buffers **for each topic** that is discovered.
The default value is equal to ``5000`` samples, with ``-1`` meaning no limit, and ``0`` no pending samples.

Depending on the combination of this configuration option and the value of ``only-with-type``, the following situations may arise when a message with unknown type is received:

* If ``max-pending-samples`` is ``-1``, or if it is greater than ``0`` and the circular buffer is not full, the sample is added to the collection.
* If ``max-pending-samples`` is greater than ``0`` and the circular buffer reaches its maximum capacity, the oldest sample with same type as the received one is popped, and either written without type (``only-with-type: false``) or discarded (``only-with-type: true``).
* If ``max-pending-samples`` is ``0``, the message is written without type if ``only-with-type: false``, and discarded otherwise.

Cleanup Period
^^^^^^^^^^^^^^

As explained in :ref:`Event Window <recorder_usage_configuration_event_window>`, a |ddsrecorder| in paused mode awaits for an event command to write in disk all samples received in the last ``event-window`` seconds.
To accomplish this, received samples are stored in memory until the aforementioned event is triggered and, in order to limit memory consumption, outdated (received more than ``event-window`` seconds ago) samples are removed from this buffer every ``cleanup-period`` seconds.
By default, its value is equal to twice the ``event-window``.

.. _recorder_usage_configuration_general_example:

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
          qos:
            reliability: true
            durability: true
            keyed: false
            partitions: true
            ownership: false
            downsampling: 4
            max-reception-rate: 10

      ignore-participant-flags: no_filter
      transport: builtin
      whitelist-interfaces:
        - "127.0.0.1"

    recorder:
      output:
        filename: "output"
        path: "."

      buffer-size: 50
      event-window: 60
      log-publish-time: false
      downsampling: 3
      max-reception-rate: 20
      only-with-type: false

    remote-controller:
      enable: true
      domain: 10
      initial-state: "PAUSED"
      command-topic-name: "/ddsrecorder/command"
      status-topic-name: "/ddsrecorder/status"

    specs:
      threads: 8
      max-pending-samples: 10
      cleanup-period: 90


.. _recorder_usage_fastdds_configuration:

Fast DDS Configuration
======================

As explained in :ref:`this section <recorder_getting_started_project_overview>`, a |ddsrecorder| instance stores (by default) all data regardless of whether their associated data type is received or not.
Some applications rely on this information being recorded and written in the resulting MCAP file, which requires that the user application is configured to send the necessary type information.
However, *Fast DDS* does not send the data type information by default, it must be configured to do so.

First of all, when generating the topic types using *eProsima Fast DDS Gen*, the option ``-typeobject`` must be added in order to generate the needed code to fill the ``TypeObject`` data.

For native types (data types that does not rely in other data types) this is enough, as *Fast DDS* will send the ``TypeObject`` by default.
However, for more complex types, it is required to use ``TypeInformation`` mechanism.
In the *Fast DDS* ``DomainParticipant`` set the following QoS in order to send this information:

.. code-block:: c

    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.typelookup_config.use_server = true;

Feel free to review :ref:`this <tutorials_dynamic_types>` section, where it is explained in detail how to configure a Fast DDS Publisher/Subscriber leveraging :term:`Dynamic Types<DynamicTypes>`.
