.. include:: ../exports/alias.include

.. _notes:

.. include:: forthcoming_version.rst

##############
Version v1.5.0
##############

This release includes the following **features**:

* Change default participant **RTPS** to **DDS**
* Add `ContentFilteredTopic` filter
* Add `--domain <num>` support

This release includes the following **documentation updates**:

* Update Windows documentation (new **Asio** version + `vcs` command)

This release includes the following **bugfixes**:

* Fix replay behavior to avoid runtime aborts and keyed-topic issues
* Fix ddsrecorder controller compilation in both OS

This release includes the following **CI improvements**:

* Upgrade `Linters CI` runner image to `ubuntu-24.04`
* Update `foonathan` version to 1.4.1


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
        - `v1.4.1 <https://github.com/eProsima/foonathan_memory_vendor/releases/tag/v1.4.1>`_
    *   - Fast CDR
        - `eProsima/Fast-CDR <https://github.com/eProsima/Fast-CDR>`_
        - `v2.3.4 <https://github.com/eProsima/Fast-CDR/releases/tag/v2.3.4>`_
        - `v2.3.5 <https://github.com/eProsima/Fast-CDR/releases/tag/v2.3.5>`_
    *   - Fast DDS
        - `eProsima/Fast-DDS <https://github.com/eProsima/Fast-DDS>`_
        - `v3.5.0 <https://github.com/eProsima/Fast-DDS/releases/tag/v3.5.0>`_
        - `v3.6.0 <https://github.com/eProsima/Fast-DDS/releases/tag/v3.6.0>`_
    *   - Dev Utils
        - `eProsima/dev-utils <https://github.com/eProsima/dev-utils>`_
        - `v1.4.0 <https://github.com/eProsima/dev-utils/releases/tag/v1.4.0>`__
        - `v1.5.0 <https://github.com/eProsima/dev-utils/releases/tag/v1.5.0>`__
    *   - DDS Pipe
        - `eProsima/DDS-Pipe <https://github.com/eProsima/DDS-Pipe.git>`_
        - `v1.4.0 <https://github.com/eProsima/DDS-Pipe/releases/tag/v1.4.0>`__
        - `v1.5.0 <https://github.com/eProsima/DDS-Pipe/releases/tag/v1.5.0>`__


#################
Previous Versions
#################

.. include:: previous_versions/v1.4.0.rst
.. include:: previous_versions/v1.3.0.rst
.. include:: previous_versions/v1.2.0.rst
.. include:: previous_versions/v1.1.0.rst
.. include:: previous_versions/v1.0.0.rst
.. include:: previous_versions/v0.4.0.rst
.. include:: previous_versions/v0.3.0.rst
.. include:: previous_versions/v0.2.0.rst
.. include:: previous_versions/v0.1.0.rst
