.. include:: ../exports/alias.include

.. _notes:

.. TODO uncomment when there are forthcoming notes
.. include:: forthcoming_version.rst

##############
Version v0.2.0
##############

This release includes *DDS Replay tool*, supporting the following **Replay features**:

* Supports setting :ref:`begin <replayer_replay_configuration_begintime>` and :ref:`end <replayer_replay_configuration_endtime>` times (``begin-time`` / ``end-time``).
* Supports setting a replay :ref:`start <replayer_replay_configuration_startreplaytime>` time (``start-replay-time``).
* Supports playing stored data at a specific playback :ref:`rate <replayer_replay_configuration_playbackrate>` (``rate``).
* Supports sending dynamic types stored in input MCAP file.

This release includes the following **User Interface features**:

* :ref:`Replay Service Command-Line Parameters <replayer_usage_usage_application_arguments>`.

This release includes the following (*DDS Replay tool*) **Configuration features**:

* Support YAML :ref:`configuration file <replayer_usage_configuration>`.
* Support for allow and block topic filters at execution time and in run-time.
* Support configuration related to DDS communication.
* Support configuration of playback settings.
* Support configuration of the internal operation of the DDS Replayer.
* Support enabling/disabling dynamic types dispatch (see :ref:`Only With Type <replayer_replay_configuration_replaytypes>`).
* Support :ref:`Interface Whitelisting <replayer_interface_whitelist>`.
* Support :ref:`Custom Transport Descriptors <replayer_custom_transport_descriptors>` (UDP or Shared Memory only).
* Support :ref:`Ignore Participant Flags <replayer_ignore_participant_flags>`.

This release includes the following **Recording features**:

* Supports recording messages with unknown (dynamic) data type, and to only record data whose type is known (see :ref:`Only With Type <recorder_usage_configuration_onlywithtype>`).

This release includes the following (*DDS Recorder tool*) **Configuration features**:

* Support record only data whose (dynamic) type is known: ``only-with-type: true`` (see :ref:`Only With Type <recorder_usage_configuration_onlywithtype>`).
* Support :ref:`Interface Whitelisting <recorder_interface_whitelist>`.
* Support :ref:`Custom Transport Descriptors <recorder_custom_transport_descriptors>` (UDP or Shared Memory only).
* Support :ref:`Ignore Participant Flags <recorder_ignore_participant_flags>`.

This release includes the following **Documentation features**:

* Updated documentation with Replay service configuration and usage instructions.

This release includes the following **Dependencies Update**:

.. list-table::
    :header-rows: 1

    *   -
        - Repository
        - Old Version
        - New Version
    *   - Foonathan Memory Vendor
        - `eProsima/foonathan_memory_vendor <https://github.com/eProsima/foonathan_memory_vendor>`_
        - `v1.3.0 <https://github.com/eProsima/foonathan_memory_vendor/releases/tag/v1.3.0>`_
        - `v1.3.1 <https://github.com/eProsima/foonathan_memory_vendor/releases/tag/v1.3.1>`_
    *   - Fast CDR
        - `eProsima/Fast-CDR <https://github.com/eProsima/Fast-CDR>`_
        - `v1.0.27 <https://github.com/eProsima/Fast-CDR/releases/tag/v1.0.27>`_
        - `v1.1.0 <https://github.com/eProsima/Fast-CDR/releases/tag/v1.1.0>`_
    *   - Fast DDS
        - `eProsima/Fast-DDS <https://github.com/eProsima/Fast-DDS>`_
        - `v2.10.1 <https://github.com/eProsima/Fast-DDS/releases/tag/v2.10.1>`_
        - `v2.11.0 <https://github.com/eProsima/Fast-DDS/releases/tag/v2.11.0>`_
    *   - Dev Utils
        - `eProsima/dev-utils <https://github.com/eProsima/dev-utils>`_
        - `v0.3.0 <https://github.com/eProsima/dev-utils/releases/tag/v0.3.0>`_
        - `v0.4.0 <https://github.com/eProsima/dev-utils/releases/tag/v0.4.0>`_
    *   - DDS Pipe
        - `eProsima/DDS-Pipe <https://github.com/eProsima/DDS-Pipe.git>`_
        - `v0.1.0 <https://github.com/eProsima/DDS-Pipe/releases/tag/v0.1.0>`_
        - `v0.2.0 <https://github.com/eProsima/DDS-Pipe/releases/tag/v0.2.0>`_

#################
Previous Versions
#################

.. include:: previous_versions/v0.1.0.rst
