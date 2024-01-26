.. include:: ../exports/alias.include

.. _notes:

.. TODO uncomment when there are forthcoming notes
.. .. include:: forthcoming_version.rst

##############
Version v0.3.0
##############

This release includes the following **Recording features**:

* New DDS Recorder suspended (active stopped) state (see :ref:`remote controller <recorder_remote_controller>` for more details).

This release includes the following **DDS Recorder & Replay internal adjustments**:

* Store *DDS Record & Replay* version in metadata record of the generated MCAP files.
* Move dynamic types storage from metadata to attachments MCAP section.
* Set `app_id` and `app_metadata` attributes on  *DDS Record & Replay* participants.
* Store schemas in OMG IDL and ROS 2 msg format.

.. warning::

    Types recorded with previous versions of *DDS Record & Replay* is no longer "replayable" after this update.

This release includes the following **DDS Recorder tool configuration features**:

* Support :ref:`Compression Settings <recorder_usage_configuration_compression>`.
* Allow disabling the storage of received types (see :ref:`Record Types <recorder_usage_configuration_recordtypes>`).
* New configuration options (``timestamp-format`` and ``local-timestamp``) available for :ref:`output file <recorder_usage_configuration_outputfile>` settings.
* New configuration option (``topics``) to configure the :ref:`Manual Topics <recorder_manual_topics>`.
* Rename ``max-reception-rate`` to ``max-rx-rate``.
* Record data in either ROS 2 format or the raw DDS format (see :ref:`Topic Type Format <recorder_usage_configuration_topictypeformat>`).

This release includes the following **DDS Replayer tool configuration features**:

* New configuration option (``topics``) to configure the :ref:`Manual Topics <replayer_manual_topics>`.
* New configuration option (``max-tx-rate``) to configure the :ref:`Max transmission rate <replayer_max_tx_rate>`.
* Remove the support for `Built-in Topics <https://dds-recorder.readthedocs.io/en/v0.2.0/rst/replaying/usage/configuration.html#built-in-topics>`_.
* Read data in either ROS 2 format or the raw DDS format.

#################
Previous Versions
#################

.. include:: previous_versions/v0.2.0.rst
.. include:: previous_versions/v0.1.0.rst
