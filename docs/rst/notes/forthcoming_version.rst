.. add orphan tag when new info added to this file

:orphan:

###################
Forthcoming Version
###################

Next release will include the following **features**:

* Add the ``include-existing-files`` configuration option, which makes the *DDS Recorder* account for the output files
  already present in the output directory when initializing the log rotation
  (see :ref:`Include Existing Files <recorder_usage_configuration_include_existing_files>`)
* Support XML endpoint profiles: a loaded ``data_reader`` (*DDS Recorder*) or ``data_writer`` (*DDS Replayer*) profile
  whose name matches the topic name is automatically applied to the endpoints of that topic.
  ``durability``, ``reliability``, ``ownership`` and ``history-depth`` explicitly set in the YAML configuration take
  precedence over the profile.
  A specific profile can be selected with the new ``endpoint-profile-name`` Topic QoS tag
  (see the :ref:`DDS Recorder <recorder_usage_configuration_xml_endpoint_profiles>` and
  :ref:`DDS Replayer <replayer_usage_configuration_xml_endpoint_profiles>` Endpoint Profiles sections).

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
* Document the XML endpoint profiles of the DDS Recorder and the DDS Replayer.
* Update the Docker image installation instructions with to use eProsima's Fast DDS Suite.
* The Foxglove tutorial will be updated to use the DDS Monitor tool instead.
* The Installation Manual will be merged with the Developer Manual, and the latter is removed.
