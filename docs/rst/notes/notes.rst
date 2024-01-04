.. include:: ../exports/alias.include

.. _notes:

##############
Version v0.3.0
##############

This release includes the following **Recording features**:

* New DDS Recorder suspended (active stopped) state (see :ref:`remote controller <recorder_remote_controller>` for more details).

This release includes the following **DDS Recorder & Replay internal adjustments**:

* Stores *DDS Record & Replay* version in metadata record of the generated MCAP files.
* Moves dynamic types storage from metadata to attachments MCAP section.
* Sets `app_id` and `app_metadata` attributes on  *DDS Record & Replay* participants.
* Stores schemas in OMG IDL and ROS 2 msg format.

.. warning::

    Types recorded with previous versions of *DDS Record & Replay* is no longer "replayable" after this update.

This release includes the following **DDS Recorder tool configuration features**:

* Supports :ref:`Compression Settings <recorder_usage_configuration_compression>`.
* Allows disabling the storage of received types (see :ref:`Record Types <recorder_usage_configuration_recordtypes>`).
* New configuration options (``timestamp-format`` and ``local-timestamp``) available for :ref:`output file <recorder_usage_configuration_outputfile>` settings.
* New configuration option (``topics``) to configure the :ref:`Manual Topics <recorder_manual_topics>`.
* Renames ``max-reception-rate`` to ``max-rx-rate``.
* Records data in either ROS 2 format or the raw DDS format (see :ref:`Topic Type Format <recorder_usage_configuration_topictypeformat>`).

This release includes the following **DDS Replayer tool configuration features**:

* New configuration option (``topics``) to configure the :ref:`Manual Topics <replayer_manual_topics>`.
* New configuration option (``max-tx-rate``) to configure the :ref:`Max transmission rate <replayer_max_tx_rate>`.
* Removes the support for `Built-in Topics <https://dds-recorder.readthedocs.io/en/v0.2.0/rst/replaying/usage/configuration.html#built-in-topics>`_.
* Reads data in either ROS 2 format or the raw DDS format.

#################
Previous Versions
#################

.. include:: previous_versions/v0.2.0.rst
.. include:: previous_versions/v0.1.0.rst
