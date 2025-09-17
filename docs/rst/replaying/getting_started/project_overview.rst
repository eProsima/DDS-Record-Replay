.. include:: ../../exports/alias.include

.. _replayer_getting_started_project_overview:

################
Project Overview
################

|eddsreplayer| is a cross-platform application that allows to play back messages recorded by a |ddsrecorder| instance.

A user can configure a |ddsreplayer| instance differently depending on the scenario and purpose, being able to tune parameters concerning the DDS layer as well as playback settings.

Among its many :ref:`configuration <replayer_usage_configuration>` options, the user is able to allow/block a set of topics, allow a set of partitions, and/or define specific QoS (other than the recorded ones) to be applied to certain topics.
It is also possible to publish samples at a rate different than the original one, filter messages according to its timestamp, or define a publication begin time, among others.

In addition, |eddsreplayer| is able to automatically send the type information recorded in a MCAP file, which might be required for applications relying on :term:`Dynamic Types<DynamicTypes>`.

Usage Description
=================

|ddsreplayer| is a terminal (non-graphical) application that creates a replay service given an input data file.
Although most use cases are covered by the default configuration, the |ddsreplayer| can be configured via a YAML file, whose format is very intuitive and human-readable.

* **Run**:
  Only the command that launches the application (``ddsreplayer``) needs to be executed to run a |ddsreplayer|.
  Please, read this :ref:`section <replayer_usage_configuration>` to apply a specific configuration, and this :ref:`section <replayer_usage_usage_application_arguments>` to see the supported arguments.
* **Interact**:
  Once the |ddsreplayer| application is running, the allowlist and blocklist topic lists could be changed in runtime by just changing the YAML configuration file.
* **Close**:
  To close the |ddsreplayer| application just send a `Ctrl+C` signal to terminate the process gracefully (see :ref:`Closing Replay Application <replayer_usage_close_replayer>`).

Common Use cases
================

To get started with |ddsreplayer|, please visit section :ref:`replayer_getting_started_usage_example`.