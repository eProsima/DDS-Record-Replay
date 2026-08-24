.. include:: ../../../exports/alias.include

.. _developer_manual_installation_sources_linux:
.. _requirements:
.. _dependencies:

########################
Developer build on Linux
########################

Follow :ref:`installation_manual_linux` to import the complete source
workspace. The developer build enables tests and documentation in the same
colcon tree:

.. code-block:: console

   $ cd ~/DDS-Record-Replay
   $ python3 -m venv .venv-docs
   $ . .venv-docs/bin/activate
   $ python -m pip install -r src/ddsrecordreplay/docs/requirements.txt
   $ colcon build --cmake-args \
       -DCMAKE_BUILD_TYPE=Debug \
       -DBUILD_TESTS=ON \
       -DBUILD_DOCS=ON
   $ colcon test
   $ colcon test-result --verbose

For a documentation-only iteration from the repository root:

.. code-block:: console

   $ sphinx-build -nW --keep-going -b html docs docs/_build/html
   $ sphinx-build -W --keep-going -b spelling docs docs/_build/spelling
   $ doc8 --ignore D001 docs

Use :ref:`cmake_options` for sanitizers, logging, and the optional Remote
Controller. colcon is recommended because the repository is a set of dependent
CMake packages rather than one top-level CMake project.

.. _colcon_installation:
.. _cmake_installation:
.. _local_installation_sl:
.. _global_installation_sl:
.. _run_app_colcon_sl:
.. _cmake_gcc_pip_wget_git_sl:
.. _colcon_install:
.. _fastdds_python:
.. _gtest_sl:
.. _asiotinyxml2_sl:
.. _openssl_sl:
.. _yaml_cpp:
.. _swig_sl:
.. _pyqt6_linux_dependencies:
.. _mcap_dependencies:
.. _eprosima_dependencies:

The former manual per-dependency CMake sequence is intentionally replaced by
the versioned ``.repos`` manifest and colcon build, which encode the dependency
order without duplicating it in documentation.

.. _colcon: https://colcon.readthedocs.io/en/released/
.. _CMake: https://cmake.org
.. _pip: https://pypi.org/project/pip/
.. _wget: https://www.gnu.org/software/wget/
.. _git: https://git-scm.com/
.. _OpenSSL: https://www.openssl.org/
.. _Gtest: https://github.com/google/googletest
.. _vcstool: https://pypi.org/project/vcstool/
