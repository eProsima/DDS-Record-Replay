.. include:: ../exports/alias.include

.. _notes:

.. .. include:: forthcoming_version.rst

##############
Version v0.5.0
##############

.. warning::

    This is the last release of the ``0.x`` series.
    This branch has reached its End-of-Life (EOL) and will receive no further releases, bugfixes or security updates.
    Users are encouraged to migrate to the latest stable version of |eddsrecord|.

This release includes the following **Recording features**:

* New :ref:`Resource Limits <recorder_usage_configuration_resource_limits>` configuration to limit the size of the output files.
* Keep rotating the output files if an old output file no longer exists.
* Report new log errors when the MCAP file cannot be created and when the disk is full.

This release includes the following **Bugfixes**:

* Keep the resource limits consistent after changing the recorder state.
* Accept IPv6 interfaces and interface names in the ``whitelist-interfaces`` configuration.
* Fix the reStructuredText warnings in the documentation.

This release includes the following **CI improvements**:

* Upgrade to Ubuntu Noble (24.04).
* Remove Ubuntu Focal (20.04) from the CI.
* Support multiple versions of Fast DDS in the CI.
* Fix the ``lz4`` and ``zstd`` vcpkg installation on Windows.
* Empty the XTSAN tests list.

This release includes the following **Dependencies Update**:

.. list-table::
    :header-rows: 1

    *   -
        - Repository
        - Old Version
        - New Version
    *   - Foonathan Memory Vendor
        - `eProsima/foonathan_memory_vendor <https://github.com/eProsima/foonathan_memory_vendor>`_
        - `v1.3.1 <https://github.com/eProsima/foonathan_memory_vendor/releases/tag/v1.3.1>`_
        - `v1.3.1 <https://github.com/eProsima/foonathan_memory_vendor/releases/tag/v1.3.1>`_
    *   - Fast CDR
        - `eProsima/Fast-CDR <https://github.com/eProsima/Fast-CDR>`_
        - `v2.2.0 <https://github.com/eProsima/Fast-CDR/releases/tag/v2.2.0>`_
        - `v2.2.8 <https://github.com/eProsima/Fast-CDR/releases/tag/v2.2.8>`_
    *   - Fast DDS
        - `eProsima/Fast-DDS <https://github.com/eProsima/Fast-DDS>`_
        - `v2.14.0 <https://github.com/eProsima/Fast-DDS/releases/tag/v2.14.0>`_
        - `v2.14.7 <https://github.com/eProsima/Fast-DDS/releases/tag/v2.14.7>`_
    *   - Dev Utils
        - `eProsima/dev-utils <https://github.com/eProsima/dev-utils>`_
        - `v0.6.0 <https://github.com/eProsima/dev-utils/releases/tag/v0.6.0>`_
        - `v0.7.0 <https://github.com/eProsima/dev-utils/releases/tag/v0.7.0>`_
    *   - DDS Pipe
        - `eProsima/DDS-Pipe <https://github.com/eProsima/DDS-Pipe.git>`_
        - `v0.4.0 <https://github.com/eProsima/DDS-Pipe/releases/tag/v0.4.0>`__
        - `v0.5.0 <https://github.com/eProsima/DDS-Pipe/releases/tag/v0.5.0>`__


#################
Previous Versions
#################

.. include:: previous_versions/v0.4.0.rst
.. include:: previous_versions/v0.3.0.rst
.. include:: previous_versions/v0.2.0.rst
.. include:: previous_versions/v0.1.0.rst
