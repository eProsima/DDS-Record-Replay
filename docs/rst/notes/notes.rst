.. include:: ../exports/alias.include

.. _notes:

.. include:: forthcoming_version.rst

##############
Version v1.5.4
##############

This release includes the following **features**:

* Add the ``include-existing-files`` option to the DDS Recorder log rotation, so output files already present in the output directory are taken into account
* Support XML endpoint profiles in the DDS Recorder and DDS Replayer, selectable through the new ``endpoint-profile-name`` Topic QoS tag

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
* Run the Python linters workflow only in PRs
* Update the Record & Replay configurations in the documentation
* Document the XML endpoint profiles and topic profile lookup
* Keep the documentation sidebar scroll position between pages

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
