.. include:: ../../exports/alias.include

.. _replayer_getting_started_project_overview:

#####################
DDS Replayer overview
#####################

Replayer is the storage-to-DDS half of DDS Record & Replay. It reads MCAP or a
replayable SQLite database, selects messages by topic, partition, and time, and
publishes them through DDS Pipe with recorded or overridden QoS.

Start with :ref:`record_replay_quickstart`, then read
:ref:`replayer_usage_usage` and :ref:`replayer_usage_configuration`.
