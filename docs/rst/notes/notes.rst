.. include:: ../exports/alias.include

.. _notes:

.. include:: forthcoming_version.rst

##############
Version v1.5.4
##############

This release includes the following **features**:

* Add the ``include-existing-files`` configuration option, which makes the *DDS Recorder* account for the output files
  already present in the output directory when initializing the log rotation
  (see :ref:`Include Existing Files <recorder_usage_configuration_include_existing_files>`)
* Support XML endpoint profiles: a loaded ``data_reader`` (*DDS Recorder*) or ``data_writer`` (*DDS Replayer*) profile
  whose name matches the topic name is automatically applied to the endpoints of that topic.
  ``durability``, ``reliability``, ``ownership`` and ``history-depth`` explicitly set in the YAML configuration take
  precedence over the profile.
  A specific profile can be selected with the new ``endpoint-profile-name`` Topic QoS tag
  (see the :ref:`DDS Recorder <recorder_usage_configuration_xml_endpoint_profiles>` and
  :ref:`DDS Replayer <replayer_usage_configuration_xml_endpoint_profiles>` Endpoint Profiles sections)

This release includes the following **bugfixes**:

* Fix a crash in the SQL replayer when reading a recording that contains a topic with no samples
* Accept the documented ``--input-file`` argument in the DDS Replayer
* Fix ``DdsRecorderMonitor`` crash when ``monitor_status()`` runs more than once
* Fix SQL size accounting in WAL mode and report discarded samples
* Refactor partitions logic, fixing MCAP channel metadata on partition changes, SQL log rotation with partitions and preservation of partition changes during replay
* Add missing ``<cstdint>`` include to the bundled MCAP library to build with GCC 15

This release includes the following **improvements**:

* Require the ``enable`` tag within the ``mcap`` and ``sql`` configuration tags
* Suppress benign MCAP warning messages on Windows builds
* Solve flaky tests and Windows CI
* Run the ``Python linters`` workflow only in PRs
* Keep the documentation sidebar scroll position between pages

This release includes the following **documentation updates**:

* Document the SQL output of the DDS Recorder, including its database schema and storage engine
* Document that the DDS Replayer plays back SQL databases as well as MCAP files
* Document the ``safety-margin``, ``size-tolerance`` and ``specs: rtps`` configuration tags
* Document the resource limits separately from the MCAP output, as they apply to both outputs
* Warn that the configuration file is validated against a schema and that unknown tags are rejected
* Document the XML endpoint profiles of the DDS Recorder and the DDS Replayer
* Update the Docker image installation instructions to use eProsima's Fast DDS Suite
* The Foxglove tutorial is updated to use the DDS Monitor tool instead
* The Installation Manual is merged with the Developer Manual, and the latter is removed

This release includes the following **Dependencies Update**:

.. list-table::
    :header-rows: 1

    *   -
        - Repository
        - Old Version
        - New Version
    *   - Foonathan Memory Vendor
        - `eProsima/foonathan_memory_vendor <https://github.com/eProsima/foonathan_memory_vendor>`__
        - `v1.4.1 <https://github.com/eProsima/foonathan_memory_vendor/releases/tag/v1.4.1>`__
        - `v1.4.1 <https://github.com/eProsima/foonathan_memory_vendor/releases/tag/v1.4.1>`__
    *   - Fast CDR
        - `eProsima/Fast-CDR <https://github.com/eProsima/Fast-CDR>`__
        - `v2.3.6 <https://github.com/eProsima/Fast-CDR/releases/tag/v2.3.6>`__
        - `v2.3.6 <https://github.com/eProsima/Fast-CDR/releases/tag/v2.3.6>`__
    *   - Fast DDS
        - `eProsima/Fast-DDS <https://github.com/eProsima/Fast-DDS>`__
        - `v3.6.2 <https://github.com/eProsima/Fast-DDS/releases/tag/v3.6.2>`__
        - `v3.6.2 <https://github.com/eProsima/Fast-DDS/releases/tag/v3.6.2>`__
    *   - Dev Utils
        - `eProsima/dev-utils <https://github.com/eProsima/dev-utils>`__
        - `v1.5.3 <https://github.com/eProsima/dev-utils/releases/tag/v1.5.3>`__
        - `v1.5.4 <https://github.com/eProsima/dev-utils/releases/tag/v1.5.4>`__
    *   - DDS Pipe
        - `eProsima/DDS-Pipe <https://github.com/eProsima/DDS-Pipe>`__
        - `v1.5.3 <https://github.com/eProsima/DDS-Pipe/releases/tag/v1.5.3>`__
        - `v1.5.4 <https://github.com/eProsima/DDS-Pipe/releases/tag/v1.5.4>`__

#################
Previous Versions
#################

.. include:: previous_versions/v1.5.3.rst
.. include:: previous_versions/v1.5.2.rst
.. include:: previous_versions/v1.5.1.rst
.. include:: previous_versions/v1.5.0.rst
.. include:: previous_versions/v1.4.0.rst
.. include:: previous_versions/v1.3.0.rst
.. include:: previous_versions/v1.2.0.rst
.. include:: previous_versions/v1.1.0.rst
.. include:: previous_versions/v1.0.0.rst
.. include:: previous_versions/v0.4.0.rst
.. include:: previous_versions/v0.3.0.rst
.. include:: previous_versions/v0.2.0.rst
.. include:: previous_versions/v0.1.0.rst
