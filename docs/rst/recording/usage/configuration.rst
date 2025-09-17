.. include:: ../../exports/alias.include
.. include:: ../../exports/roles.include

.. _recorder_usage_configuration:

#############
Configuration
#############


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

Load XML Configuration
----------------------

Fast DDS supports configuration of its internal entities (DomainParticipant, DataWriter, etc.) via XML Profiles.
These XML files contain different profiles that set specific QoS, and entities can be created following such profiles.

The way to load these XML configurations is using the *DDS Recorder* YAML configuration.
The YAML Configuration supports an ``xml`` optional tag that contains certain options to load Fast DDS XML configurations.
XML configurations are then used to configure the internal DomainParticipant.

To specify which profile to use, the ``dds-profile`` tag should be set with the name of the desired profile.

Load XML Files
^^^^^^^^^^^^^^

The ``files`` optional tag allows specifying a list of file paths from which XML configurations can be loaded.

Raw XML
^^^^^^^

The ``raw`` optional tag allows specifying an XML configuration directly as a string.
The XML content must follow the same format as an XML file and will be loaded as part of the configuration.

.. code-block:: yaml

    xml:

      files:
          - "./xml_configuration.xml"

      raw: |
          <?xml version="1.0" encoding="UTF-8" ?>
          <profiles xmlns="http://www.eprosima.com">
              <participant profile_name="custom_participant_configuration">
                  <domainId>1</domainId>
                  <rtps></rtps>
              </participant>
          </profiles>

    recorder-profile: "participant_profile"

.. _recorder_usage_configuration_domain_id:

DDS Domain
^^^^^^^^^^

Tag ``domain`` configures the :term:`Domain Id`.

.. code-block:: yaml

    domain: 101

.. _recorder_builtin_topics:

Built-in Topics
^^^^^^^^^^^^^^^

The discovery phase can be accelerated by listing topics under the ``builtin-topics`` tag.
The |ddsrecorder| will create the DataWriters and DataReaders for these topics in the |ddsrecorder| initialization.
The :ref:`Topic QoS <recorder_topic_qos>` for these topics can be manually configured with the :ref:`Manual Topic <recorder_manual_topics>` and with the :ref:`Specs Topic QoS <recorder_specs_topic_qos>`; if a :ref:`Topic QoS <recorder_topic_qos>` is not configured, it will take its default value.

The ``builtin-topics`` must specify a ``name`` and ``type`` without wildcard characters.

**Example of usage:**

    .. code-block:: yaml

        builtin-topics:
          - name: HelloWorldTopic
            type: HelloWorld

.. _recorder_topic_filtering:

Topic Filtering
^^^^^^^^^^^^^^^

The |ddsrecorder| automatically detects the topics that are being used in a DDS Network.
The |ddsrecorder| then creates internal DDS :term:`Readers<DataReader>` to record the data published on each topic.
The |ddsrecorder| allows filtering DDS :term:`Topics<Topic>` to allow users to configure the DDS :term:`Topics<Topic>` that must be recorded.
These data filtering rules can be configured under the ``allowlist`` and ``blocklist`` tags.
If the ``allowlist`` and ``blocklist`` are not configured, the |ddsrecorder| will recorded the data published on every topic it discovers.
If both the ``allowlist`` and ``blocklist`` are configured and a topic appears in both of them, the ``blocklist`` has priority and the topic will be blocked.

Topics are determined by the tags ``name`` (required) and ``type``, both of which accept wildcard characters.

.. note::

    Placing quotation marks around values in a YAML file is generally optional, but values containing wildcard characters do require single or double quotation marks.

Consider the following example:

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

In this example, the data published in the topic ``AllowedTopic1`` with type ``Allowed`` and in the topic ``AllowedTopic2`` with any type will be recorded by the |ddsrecorder|.
The data published in the topic ``HelloWorldTopic`` with type ``HelloWorld`` will be blocked, since the ``blocklist`` is blocking all topics with any name and with type ``HelloWorld``.

.. _recorder_partition_filtering:

Partition Filtering
^^^^^^^^^^^^^^^^^^^

The |ddsrecorder| automatically detects the partitions of all DataWriters in a DDS Network.
The |ddsrecorder| then creates internal DDS :term:`Readers<DataReader>` to record the data published on each topic.
The |ddsrecorder| allows filtering DDS :term:`Partitions<Partition>` to allow users to configure
the DDS :term:`Partitions<Partition>` that must be recorded.
These data filtering rules can be configured under the ``partitions`` tag.

Consider the following example:

.. code-block:: yaml

    partitions:
      - "A"
      - "B"


In this example, the data published by DataWriters that use partitions ``A`` or ``B`` will be recorded by the |ddsrecorder|.
Any data published by DataWriters without these partitions will be ignored.

.. _recorder_topic_qos:

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
        - Topic with / without `key <https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/topic/typeSupport/typeSupport.html#data-types-with-a-key>`_

    *   - History Depth
        - ``history-depth``
        - *unsigned integer*
        - ``5000``
        - :ref:`recorder_history_depth`

    *   - Max Reception Rate
        - ``max-rx-rate``
        - *float*
        - ``0`` (unlimited)
        - :ref:`recorder_max_rx_rate`

    *   - Downsampling
        - ``downsampling``
        - *unsigned integer*
        - ``1``
        - :ref:`recorder_downsampling`

.. warning::

    Manually configuring ``TRANSIENT_LOCAL`` durability may lead to incompatibility issues when the discovered reliability is ``BEST_EFFORT``.
    Please ensure to always configure the ``reliability`` when configuring the ``durability`` to avoid the issue.

.. _recorder_history_depth:

History Depth
"""""""""""""

The ``history-depth`` tag configures the history depth of the Fast DDS internal entities.
By default, the depth of every RTPS History instance is :code:`5000`, which sets a constraint on the maximum number of samples a |ddsrecorder| instance can deliver to late joiner Readers configured with ``TRANSIENT_LOCAL`` `DurabilityQosPolicyKind <https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/core/policy/standardQosPolicies.html#durabilityqospolicykind>`_.
Its value should be decreased when the sample size and/or number of created endpoints (increasing with the number of topics) are big enough to cause memory exhaustion issues.
If enough memory is available, however, the ``history-depth`` could be increased to deliver a greater number of samples to late joiners.

.. _recorder_max_rx_rate:

Max Reception Rate
""""""""""""""""""

The ``max-rx-rate`` tag limits the frequency [Hz] at which samples are processed by discarding messages received before :code:`1/max-rx-rate` seconds have passed since the last processed message.
It only accepts non-negative numbers.
By default it is set to ``0``; it processes samples at an unlimited reception rate.

.. _recorder_downsampling:

Downsampling
""""""""""""

The ``downsampling`` tag reduces the sampling rate of the received data by only keeping *1* out of every *n* samples received (per topic), where *n* is the value specified under the ``downsampling`` tag.
When the ``max-rx-rate`` tag is also set, downsampling only applies to messages that have passed the ``max-rx-rate`` filter.
It only accepts positive integers.
By default it is set to ``1``; it accepts every message.

.. _recorder_manual_topics:

Manual Topics
^^^^^^^^^^^^^

A subset of :ref:`Topic QoS <recorder_topic_qos>` can be manually configured for a specific topic under the tag ``topics``.
The tag ``topics`` has a required ``name`` tag that accepts wildcard characters.
It also has two optional tags: a ``type`` tag that accepts wildcard characters, and a ``qos`` tag with the :ref:`Topic QoS <recorder_topic_qos>` that the user wants to manually configure.
If a ``qos`` is not manually configured, it will get its value by discovery.

.. code-block:: yaml

    topics:
      - name: "temperature/*"
        type: "temperature/types/*"
        qos:
          max-rx-rate: 15
          downsampling: 2

.. note::

    The :ref:`Topic QoS <recorder_topic_qos>` configured in the Manual Topics take precedence over the :ref:`Specs Topic QoS <recorder_specs_topic_qos>`.

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


.. _recorder_easy_mode:

ROS 2 Easy Mode Configuration
-----------------------------

|ddsrecorder| allows configuring the address of a remote discovery server when using
`ROS 2 Easy Mode <https://docs.vulcanexus.org/en/latest/rst/enhancements/easy_mode/easy_mode.html>`__
through the ``ros2-easy-mode`` tag:

.. code-block:: yaml

    ros2-easy-mode: "2.2.2.2"       # Remote discovery server address

.. warning::
    This configuration is incompatible with the ``transports`` tag.
    Setting ``ros2-easy-mode`` other than ``transports: builtin``
    will prevent Easy Mode from being configured.

    For now, only IPv4 addresses are supported.

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

Recorder Configuration
----------------------

Configuration of data writing in the database.

.. _recorder_usage_configuration_outputfile:

Output File
^^^^^^^^^^^

The recorder output file does support the following configuration settings under the ``output`` tag:

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

    *   - Timestamp format
        - ``timestamp-format``
        - Configure the format of the output file |br|
          timestamp (as in ``std::put_time``).
        - ``string``
        - ``%Y-%m-%d_%H-%M-%S_%Z``

    *   - Local timestamp
        - ``local-timestamp``
        - Whether to use a local or global (GMT) |br|
          timestamp.
        - ``boolean``
        - ``true``

When DDS Recorder application is launched (or when remotely controlled, every time a ``start/pause`` command is received while in ``SUSPENDED/STOPPED`` state), a temporary file with ``filename`` name (+timestamp prefix) and ``.mcap.tmp~`` extension is created in ``path``.
This file is not readable until the application terminates, receives a ``suspend/stop/close`` command, or the file reaches its maximum size (see :ref:`Resource Limits <recorder_usage_configuration_resource_limits>`).
On such event, the temporal file is renamed to have ``.mcap`` extension in the same location, and is then ready to be processed.

Buffer size
^^^^^^^^^^^

``buffer-size`` indicates the number of samples to be stored in the process memory before the dump to disk.
This avoids disk access each time a sample is received.
By default, its value is set to ``100``.

.. _recorder_usage_configuration_cleanup_period:

Cleanup Period
^^^^^^^^^^^^^^

As explained in :ref:`Event Window <recorder_usage_configuration_event_window>`, a |ddsrecorder| in paused mode awaits for an event command to write in disk all samples received in the last ``event-window`` seconds.
To accomplish this, received samples are stored in memory until the aforementioned event is triggered and, in order to limit memory consumption, outdated (received more than ``event-window`` seconds ago) samples are removed from this buffer every ``cleanup-period`` seconds.
By default, its value is equal to twice the ``event-window``.

.. _recorder_usage_configuration_event_window:

Event Window
^^^^^^^^^^^^

|ddsrecorder| can be configured to continue saving data when it is in paused mode.
Thus, when an event is triggered from the remote controller, samples received in the last ``event-window`` seconds are stored in the database.

In other words, the ``event-window`` acts as a sliding time window that allows to save the collected samples in this time window only when the remote controller event is received.
By default, its value is set to ``20`` seconds.

.. _recorder_usage_configuration_max_number_pending_samples:

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

.. _recorder_usage_configuration_onlywithtype:

Only With Type
^^^^^^^^^^^^^^

By default, all (allowed) received messages are recorded regardless of whether their associated type information has been received.
However, a user can enforce that **only** samples whose type is received are recorded by setting ``only-with-type: true``.

.. _recorder_usage_configuration_recordtypes:

Record Types
^^^^^^^^^^^^

By default, all type information received during execution is stored in a dedicated MCAP file section.
This information is then leveraged by |ddsreplayer| on playback, publishing recorded types in addition to data samples, which may be required for receiver applications relying on :term:`Dynamic Types<DynamicTypes>` (see :ref:`Replay Types <replayer_replay_configuration_replaytypes>`).
However, a user may choose to disable this feature by setting ``record-types: false``.

.. _recorder_usage_configuration_topictypeformat:

Topic type format
^^^^^^^^^^^^^^^^^

The optional ``ros2-types`` tag enables specification of the format for storing schemas.
When set to ``true``, schemas are stored in ROS 2 message format (.msg).
If set to ``false``, schemas are stored in OMG IDL format (.idl).
By default it is set to ``false``.

.. _recorder_usage_configuration_mcap:

MCAP Configuration
^^^^^^^^^^^^^^^^^^

The ``enable`` tag allows users to enable or disable whether to record data in an MCAP file.

.. _recorder_usage_configuration_logpublishtime:

Log Publish Time
""""""""""""""""

By default (``log-publish-time: false``) received messages are stored in the MCAP file with ``logTime`` value equals to the reception timestamp.
Additionally, the timestamp corresponding to when messages were initially published (``publishTime``) is also included in the information dumped to MCAP files.
In some applications, it may be required to use the ``publishTime`` as ``logTime``, which can be achieved by providing the ``log-publish-time: true`` configuration option.

.. _recorder_usage_configuration_compression:

Compression
"""""""""""

Compression settings for writing to an MCAP file can be specified under the ``compression`` configuration tag.
The supported compression options are:

.. list-table::
    :header-rows: 1

    *   - Parameter
        - Tag
        - Description
        - Data type
        - Default value
        - Possible values

    *   - Compression Algorithm
        - ``algorithm``
        - Compression algorithm to |br|
          use when writing Chunks.
        - ``string``
        - ``zstd``
        - ``none`` |br|
          ``lz4`` |br|
          ``zstd``

    *   - Compression Level
        - ``level``
        - Compression level to use |br|
          when writing Chunks.
        - ``string``
        - ``default``
        - ``fastest`` |br|
          ``fast`` |br|
          ``default`` |br|
          ``slow`` |br|
          ``slowest``

    *   - Force Compression
        - ``force``
        - Force compression on all |br|
          Chunks (even for those |br|
          that do not benefit from |br|
          compression).
        - ``boolean``
        - ``false``
        - ``true`` |br|
          ``false``

.. _recorder_usage_configuration_resource_limits:

Resource Limits
"""""""""""""""

The ``resource-limits`` tag allows users to control the size of the *DDS Recorder's* output by setting limits on disk usage. This configuration allows distinct limits for the MCAP and SQL outputs while maintaining a shared safety margin to ensure stable memory usage.

- **``max-file-size``**: Specifies the maximum size of each output file. Applicable only to the MCAP recorder, as the SQL recorder uses a single database file.
- **``max-size``**: Specifies the maximum aggregate size of all output files. For the SQL recorder, this defines the maximum size of the database file. For the MCAP recorder, this determines the total size of all generated files.

Safety Margin
"""""""""""""

The ``safety-margin`` property is shared between the SQL and MCAP outputs and is configured in the ``output`` section. This parameter reserves a buffer of free disk space, ensuring that at least ``safety-margin`` bytes remain available to prevent the system from running out of memory. This applies regardless of whether one or both recorders are enabled.
By default, the safety margin is set to ``10MB``.

MCAP Recorder Behavior
""""""""""""""""""""""

If the ``max-size`` is greater than the ``max-file-size``, the |ddsrecorder| will create multiple files, each with a size up to the value of ``max-file-size``, until the total size reaches ``max-size``.

SQL Recorder Behavior
"""""""""""""""""""""

For the SQL recorder:
- The database is always stored in a single file.
- **Both ``max-file-size`` and ``max-size`` control the same parameter, i.e., total size of the database**. This is why setting just one of them is sufficient as the other will be automatically set to the same value. If both are set to different values, an error will be returned.

Default Behavior
""""""""""""""""

By default:
- ``max-file-size`` is unlimited (``0B``).
- ``max-size`` is equal to ``max-file-size``, which means the |ddsrecorder| creates a single output file of unlimited size.

Resource Limits Configuration Rules
"""""""""""""""""""""""""""""""""""

The relation between ``max-size`` and ``safety-margin`` introduces resource limits that dictate memory usage. The behavior depends on the enabled recorders:

A. If only one recorder is enabled

   * **No resource limits set**: The recorder will use all available disk space.
   * **Resource limits set**: Ensure the available space is sufficient to meet the specified limits.

B. If both recorders are enabled

   * **No resource limits set**: Available disk space is divided equally between the recorders.
   * **One recorder with resource limits set**: The other recorder will use the remaining disk space.
   * **Both recorders with resource limits set**: Ensure the combined limits do not exceed the available disk space, returning an error otherwise.

Size Margin
"""""""""""

The ``size-margin`` property is an optional parameter that establish the margin of error for the size of the output files.


.. warning::

    If the ``max-file-size`` or the ``max-size`` are set to a value higher than the available space in the disk (counting for the safety-margin), an error will be returned.

To keep the |ddsrecorder| recording after reaching the ``max-size``, users can set the ``log-rotation`` tag to ``true``.
Enabling ``log-rotation`` allows the |ddsrecorder| to overwrite old files to free space for new ones.

MCAP Log-Rotation Behavior
""""""""""""""""""""""""""

When the MCAP ``log-rotation`` is enabled, the |ddsrecorder| will remove the oldest file whenever ``max-size`` is reached.

SQL Log-Rotation Behavior
"""""""""""""""""""""""""

When the SQL ``log-rotation`` is enabled, the |ddsrecorder| will remove the oldest entries of the database whenever ``max-size`` is reached.

.. note::

    To keep the |ddsrecorder| from overwriting previous output files, users can set ``{"avoid_overwriting_output": true}`` as the argument (``"args"``) of the ``stop`` command (see :ref:`Control Commands <recorder_remote_controller_data_types>`).
    If set, the |ddsrecorder| will not overwrite its output files, at the cost of possibly exceeding the ``max-size``.

.. note::

    If an output file is moved, deleted, or renamed, the |ddsrecorder| will keep the size of the file reserved and rotate between the remaining files.

**Example of usage**

.. code-block:: yaml

  recorder:
    output:
      safety-margin: "1GB"

    mcap:
      enable: true
      resource-limits:
        max-file-size: 25MB
        max-size: 200MB
        log-rotation: true
        size-margin: 2MB

    sql:
      enable: true
      resource-limits:
        max-size: 20MiB
        log-rotation: true
        size-margin: 1MB

.. _recorder_usage_configuration_sql:

SQL Configuration
^^^^^^^^^^^^^^^^^

The ``enable`` tag allows users to enable or disable whether to record data in an SQL database.

.. _recorder_usage_configuration_sql_data_format:

Data Format
"""""""""""

The ``data-format`` tag allows users to specify the format in which data is stored in the SQL database.
The data can be stored in ``cdr`` (which makes the data replayable by the |ddsreplayer|), in ``json`` (which makes the data human-readable), or in ``both`` (default).

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
          control system.
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
          ``SUSPENDED`` |br|
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

.. _recorder_specs_topic_qos:

QoS
^^^

``specs`` supports a ``qos`` **optional** tag to configure the default values of the :ref:`Topic QoS <recorder_topic_qos>`.

.. note::

    The :ref:`Topic QoS <recorder_topic_qos>` configured in ``specs`` can be overwritten by the :ref:`Manual Topics <recorder_manual_topics>`.

.. _recorder_specs_logging:

Logging
^^^^^^^

``specs`` supports a ``logging`` **optional** tag to configure the |ddsrecorder| logs.
Under the ``logging`` tag, users can configure the type of logs to display and filter the logs based on their content and category.
When configuring the verbosity to ``info``, all types of logs, including informational messages, warnings, and errors, will be displayed.
Conversely, setting it to ``warning`` will only show warnings and errors, while choosing ``error`` will exclusively display errors.
By default, the filter allows all errors to be displayed, while selectively permitting warning and informational messages from ``DDSRECORDER`` category.

.. note::

    Configuring the logs via the Command-Line is still active and takes precedence over YAML configuration when both methods are used simultaneously.

.. list-table::
    :header-rows: 1

    *   - Logging
        - Yaml tag
        - Description
        - Data type
        - Default value
        - Possible values

    *   - Verbosity
        - ``verbosity``
        - Show messages of equal |br|
          or higher importance.
        - *enum*
        - ``error``
        - ``info`` / ``warning`` / ``error``

    *   - Filter
        - ``filter``
        - Regex to filter the category  |br|
          or message of the logs.
        - *string*
        - info : ``DDSRECORDER`` |br|
          warning : ``DDSRECORDER`` |br|
          error : ``""``
        - Regex string

.. note::

    For the logs to function properly, the ``-DLOG_INFO=ON`` compilation flag is required.

The |ddsrecorder| prints the logs by default (warnings and errors in the standard error and info traces in the standard output).
The |ddsrecorder|, however, can also publish the logs in a DDS topic.
To publish the logs, under the tag ``publish``, set ``enable: true`` and set a ``domain`` and a ``topic-name``.
The type of the logs published is defined as follows:

**LogEntry.idl**

.. code-block:: idl

    const long UNDEFINED = 0x10000000;
    const long SAMPLE_LOST = 0x10000001;
    const long TOPIC_MISMATCH_TYPE = 0x10000002;
    const long TOPIC_MISMATCH_QOS = 0x10000003;
    const long FAIL_MCAP_CREATION = 0x12000001;
    const long FAIL_MCAP_WRITE = 0x12000002;

    enum Kind {
      Info,
      Warning,
      Error
    };

    struct LogEntry {
      @key long event;
      Kind kind;
      string category;
      string message;
      string timestamp;
    };

**Example of usage**

.. code-block:: yaml

    logging:
      verbosity: info
      filter:
        error: "DDSPIPE|DDSRECORDER"
        warning: "DDSPIPE|DDSRECORDER"
        info: "DDSRECORDER"
      publish:
        enable: true
        domain: 84
        topic-name: "DdsRecorderLogs"
      stdout: true

.. _recorder_specs_monitor:

Monitor
^^^^^^^

``specs`` supports a ``monitor`` **optional** tag to publish internal data from the |ddsrecorder|.
If the monitor is enabled, it publishes (and logs under the ``MONITOR_DATA`` :ref:`log filter <recorder_specs_logging>`) the *DDS Recorder's* internal data on a ``domain``, under a ``topic-name``, once every ``period`` (in milliseconds).
If the monitor is not enabled, the |ddsrecorder| will not collect or publish any data.

.. note::

    The data published is relative to each period.
    The |ddsrecorder| will reset its tracked data after publishing it.

In particular, the |ddsrecorder| can monitor its internal status and its topics.
When monitoring its internal status, the |ddsrecorder| will track different errors of the |ddsrecorder|.
The type of the data published is defined as follows:

**DdsRecorderMonitoringStatus.idl**

.. code-block:: idl

    struct MonitoringErrorStatus {
        boolean type_mismatch;
        boolean qos_mismatch;
    };

    struct MonitoringStatus {
        MonitoringErrorStatus error_status;
        boolean has_errors;
    };

    struct DdsRecorderMonitoringErrorStatus {
        boolean mcap_file_creation_failure;
        boolean disk_full;
    };

    struct DdsRecorderMonitoringStatus : MonitoringStatus {
        DdsRecorderMonitoringErrorStatus ddsrecorder_error_status;
    };

When monitoring its topics, the |ddsrecorder| will track the number of messages lost, received, and the message reception rate [Hz] of each topic.
It will also track if a topic's type is discovered, if there is a type mismatch, and if there is a QoS mismatch.
The type of the data published is defined as follows:

**MonitoringTopics.idl**

.. code-block:: idl

    struct DdsTopicData
    {
        string participant_id;
        unsigned long msgs_lost;
        unsigned long msgs_received;
        double msg_rx_rate;
    };

    struct DdsTopic
    {
        string name;
        string type_name;
        boolean type_discovered;
        boolean type_mismatch;
        boolean qos_mismatch;
        sequence<DdsTopicData> data;
    };

    struct MonitoringTopics
    {
        sequence<DdsTopic> topics;
    };

**Example of usage**

.. code-block:: yaml

    monitor:
      domain: 10
      status:
        enable: true
        domain: 11
        period: 2000
        topic-name: "DdsRecorderStatus"

      topics:
        enable: true
        domain: 12
        period: 1500
        topic-name: "DdsRecorderTopics"

.. _recorder_usage_configuration_general_example:

General Example
---------------

A complete example of all the configurations described on this page can be found below.

.. warning::

    This example can be used as a quick reference, but it may not be correct due to incompatibility or exclusive properties. **Do not take it as a working example**.

.. code-block:: yaml

    dds:

      xml:

        files:
            - "./xml_configuration.xml"

        raw: |
            <?xml version="1.0" encoding="UTF-8" ?>
            <profiles xmlns="http://www.eprosima.com">
                <participant profile_name="custom_participant_configuration">
                    <domainId>1</domainId>
                    <rtps></rtps>
                </participant>
            </profiles>

      recorder-profile: "participant_profile"

      domain: 0

      allowlist:
        - name: "topic_name"
          type: "topic_type"

      blocklist:
        - name: "topic_name"
          type: "topic_type"

      partitions:
        - "*"

      builtin-topics:
        - name: "HelloWorldTopic"
          type: "HelloWorld"

      topics:
        - name: "temperature/*"
          type: "temperature/types/*"
          qos:
            max-rx-rate: 15
            downsampling: 2

      ignore-participant-flags: no_filter
      transport: builtin
      ros2-easy-mode: "2.2.2.2"
      whitelist-interfaces:
        - "127.0.0.1"

    recorder:
      buffer-size: 50
      cleanup-period: 90
      event-window: 60
      max-pending-samples: 10
      only-with-type: false
      record-types: true
      ros2-types: false

      output:
        filename: "output"
        path: "."
        timestamp-format: "%Y-%m-%d_%H-%M-%S_%Z"
        local-timestamp: false
        safety-margin: 10GB

      mcap:
        enable: true
        log-publish-time: false

        resource-limits:
          max-file-size: 250KB
          max-size: 2MiB
          log-rotation: true
          size-tolerance: 10KB

        compression:
          algorithm: lz4
          level: slowest
          force: true

      sql:
        enable: false
        data-format: "json"

        resource-limits:
          max-size: 2MiB
          log-rotation: true
          size-tolerance: 10KB

    remote-controller:
      enable: true
      domain: 10
      initial-state: "PAUSED"
      command-topic-name: "/ddsrecorder/command"
      status-topic-name: "/ddsrecorder/status"

    specs:
      threads: 8

      qos:
        max-rx-rate: 20
        downsampling: 3

      logging:
        verbosity: info
        filter:
          error: "DDSPIPE|DDSRECORDER"
          warning: "DDSPIPE|DDSRECORDER"
          info: "DDSRECORDER"
        publish:
          enable: true
          domain: 84
          topic-name: "DdsRecorderLogs"
        stdout: true

      monitor:
        domain: 10
        topics:
          enable: true
          domain: 11
          period: 1000
          topic-name: "DdsRecorderTopics"

        status:
          enable: true
          domain: 12
          period: 2000
          topic-name: "DdsRecorderStatus"

.. _recorder_usage_fastdds_configuration:

Fast DDS Configuration
======================

|ddsrecorder| instance stores (by default) all data regardless of whether their associated data type is received or not.
Some applications rely on this information being recorded and written in the resulting MCAP file, which requires that the user application is configured to send the necessary type information.
By default, *Fast DDS* automatically sends the data type information, so no additional configuration is required.

Feel free to review `this <https://github.com/eProsima/Fast-DDS/tree/master/examples/cpp/xtypes>`_ section, where it is explained in detail how to configure a Fast DDS Publisher/Subscriber leveraging :term:`Dynamic Types<DynamicTypes>`.
