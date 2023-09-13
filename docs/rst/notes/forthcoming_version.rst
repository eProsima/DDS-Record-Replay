.. add orphan tag when new info added to this file

.. :orphan:

###################
Forthcoming Version
###################

Next release will include the following **DDS Recorder & Replay internal adjustments**:

* Store *DDS Record & Replay* version in metadata record of the generated MCAP files.
* Move dynamic types storage from metadata to attachments MCAP section.

.. warning::

    Types recorded with previous versions of *DDS Record & Replay* will no longer be "replayable" after this update.

Next release will include the following **DDS Recorder tool configuration features**:

* Support :ref:`Compression Settings <recorder_usage_configuration_compression>`.
* Allow disabling the storage of received types (see :ref:`Record Types <recorder_usage_configuration_recordtypes>`).
