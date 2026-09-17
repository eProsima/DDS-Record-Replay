.. add orphan tag when new info added to this file

:orphan:

###################
Forthcoming Version
###################

Next release will include the following **features**:

* Add the ``include-existing-files`` configuration option, which makes the *DDS Recorder* account for the output files
  already present in the output directory when initializing the log rotation
  (see :ref:`Include Existing Files <recorder_usage_configuration_include_existing_files>`)

Next release will include the following **bugfixes**:

* Fix a crash in the SQL replayer when reading a recording that contains a topic with no samples.
* Accept the documented ``--input-file`` argument in the DDS Replayer.

Next release will include the following **improvements**:

* Require the ``enable`` tag within the ``mcap`` and ``sql`` configuration tags.

Next release will include the following **documentation updates**:

* Document the SQL output of the DDS Recorder, including its database schema and storage engine.
* Document that the DDS Replayer plays back SQL databases as well as MCAP files.
* Document the ``safety-margin``, ``size-tolerance`` and ``specs: rtps`` configuration tags.
* Document the resource limits separately from the MCAP output, as they apply to both outputs.
* Warn that the configuration file is validated against a schema and that unknown tags are rejected.
* Update the Docker image installation instructions with to use eProsima's Fast DDS Suite.
* The Foxglove tutorial will be updated to use the DDS Monitor tool instead.
* The Installation Manual will be merged with the Developer Manual, and the latter is removed.
