.. include:: ../../../exports/alias.include
.. include:: ../../../exports/roles.include

.. _cmake_options:

#############
CMake options
#############

|eddsrecord| provides numerous CMake options for changing the behavior and configuration of |eddsrecord|.
These options allow the developer to enable/disable certain |eddsrecord| settings by defining these options to ``ON``/``OFF`` at the CMake execution, or set the required path to certain dependencies.

.. warning::
    These options are only for developers who installed |eddsrecord| following the compilation steps described in :ref:`developer_manual_installation_sources_linux`.

.. list-table::
    :header-rows: 1

    *   - Option
        - Description
        - Possible values
        - Default
    *   - ``CMAKE_BUILD_TYPE``
        - CMake optimization build type.
        - ``Release`` |br|
          ``Debug``
        - ``Release``
    *   - ``BUILD_DDSRECORDER_CONTROLLER``
        - Build the |ddsrecorder| remote |br| controller application. |br|
        - ``OFF`` |br|
          ``ON``
        - ``OFF``
    *   - ``BUILD_DOCS``
        - Build the |eddsrecord| |br| documentation. |br|
        - ``OFF`` |br|
          ``ON``
        - ``OFF``
    *   - ``BUILD_TESTS``
        - Build the |eddsrecord| tools and |br| documentation tests.
        - ``OFF`` |br|
          ``ON``
        - ``OFF``
    *   - ``LOG_INFO``
        - Activate |eddsrecord| logs. It is |br|
          set to ``ON`` if ``CMAKE_BUILD_TYPE`` is set |br|
          to ``Debug``.
        - ``OFF`` |br|
          ``ON``
        - ``ON`` if ``Debug`` |br|
          ``OFF`` otherwise
    *   - ``ASAN_BUILD``
        - Activate address sanitizer build.
        - ``OFF`` |br|
          ``ON``
        - ``OFF``
    *   - ``TSAN_BUILD``
        - Activate thread sanitizer build.
        - ``OFF`` |br|
          ``ON``
        - ``OFF``
