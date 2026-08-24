.. include:: ../../exports/alias.include

.. _recorder_getting_started_project_overview:

#####################
DDS Recorder overview
#####################

Recorder is the DDS-to-storage half of DDS Record & Replay. It discovers topics
through DDS Pipe, applies topic, partition, and content filters, and writes MCAP
and/or SQLite. Dynamic type information is stored when it is available, so
Replayer and inspection tools can reconstruct the topic schema.

Start with :ref:`record_replay_quickstart`, then read
:ref:`recorder_usage_usage`, :ref:`recorder_usage_configuration`, and
:ref:`recorder_remote_control`.
