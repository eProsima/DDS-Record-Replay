.. include:: ../../../exports/alias.include
.. include:: ../../../exports/roles.include

.. _cmake_options:

#############
CMake options
#############

*eProsima DDS Recorder* provides numerous CMake options for changing the behavior and configuration of
*DDS Recorder*.
These options allow the developer to enable/disable certain *DDS Recorder* settings by defining these options to
``ON``/``OFF`` at the CMake execution, or set the required path to certain dependencies.

.. warning::
    These options are only for developers who installed *eProsima DDS Recorder* following the compilation steps
    described in :ref:`developer_manual_installation_sources_linux`.

.. list-table::
    :header-rows: 1

    *   - Option
        - Description
        - Possible values
        - Default
    *   - :class:`CMAKE_BUILD_TYPE`
        - CMake optimization build type.
        - ``Release`` |br|
          ``Debug``
        - ``Release``
