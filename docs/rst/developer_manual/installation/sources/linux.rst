.. include:: ../../../exports/alias.include
.. include:: ../../../exports/roles.include

.. _developer_manual_installation_sources_linux:

###############################
Linux installation from sources
###############################

The instructions for installing the |ddsrecorder| application from sources and its required
dependencies are provided in this page.
It is organized as follows:

.. contents::
    :local:
    :backlinks: none
    :depth: 2


Dependencies installation
=========================

|ddsrecorder| depends on *eProsima Fast DDS* library and certain Debian packages.
This section describes the instructions for installing |ddsrecorder| dependencies and requirements in a Linux
environment from sources.
The following packages will be installed:

- ``foonathan_memory_vendor``, an STL compatible C++ memory allocation library.
- ``fastcdr``, a C++ library that serializes according to the standard CDR serialization mechanism.
- ``fastrtps``, the core library of eProsima Fast DDS library.
- ``cmake_utils``, an eProsima utils library for CMake.
- ``cpp_utils``, an eProsima utils library for C++.
- ``ddsrouter``, an eProsima end-user application that enables the connection of distributed DDS networks.

First of all, the :ref:`Requirements <requirements>` and :ref:`Dependencies <dependencies>` detailed below need to be
met.
Afterwards, the user can choose whether to follow either the :ref:`colcon <colcon_installation>` or the
:ref:`CMake <cmake_installation>` installation instructions.

.. _requirements:

Requirements
------------

The installation of |ddsrecorder| in a Linux environment from sources requires the following tools to be
installed in the system:

* :ref:`cmake_gcc_pip_wget_git_sl`
* :ref:`colcon_install` [optional]
* :ref:`gtest_sl` [for test only]
* :ref:`py_yaml` [for YAML Validator only]
* :ref:`json_schema` [for YAML Validator only]


.. _cmake_gcc_pip_wget_git_sl:

CMake, g++, pip, wget and git
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

These packages provide the tools required to install |ddsrecorder| and its dependencies from command line.
Install CMake_, `g++ <https://gcc.gnu.org/>`_, pip_, wget_ and git_ using the package manager of the appropriate
Linux distribution. For example, on Ubuntu use the command:

.. code-block:: bash

    sudo apt install cmake g++ pip wget git


.. _colcon_install:

Colcon
^^^^^^

colcon_ is a command line tool based on CMake_ aimed at building sets of software packages.
Install the ROS 2 development tools (colcon_ and vcstool_) by executing the following command:

.. code-block:: bash

    pip3 install -U colcon-common-extensions vcstool

.. note::

    If this fails due to an Environment Error, add the :code:`--user` flag to the :code:`pip3` installation command.


.. _gtest_sl:

Gtest
^^^^^

Gtest_ is a unit testing library for C++.
By default, |ddsrecorder| does not compile tests.
It is possible to activate them with the opportune
`CMake options <https://colcon.readthedocs.io/en/released/reference/verb/build.html#cmake-options>`_
when calling colcon_ or CMake_.
For more details, please refer to the :ref:`cmake_options` section.
For a detailed description of the Gtest_ installation process, please refer to the
`Gtest Installation Guide <https://github.com/google/googletest>`_.

It is also possible to clone the Gtest_ Github repository into the |ddsrecorder| workspace and compile it with colcon_
as a dependency package.
Use the following command to download the code:

.. code-block:: bash

    git clone --branch release-1.11.0 https://github.com/google/googletest src/googletest-distribution


.. _py_yaml:

PyYAML
^^^^^^

`PyYAML <https://pyyaml.org/>`_ is a YAML parser and emitter for Python.

Install ``pyyaml`` by executing the following command:

.. code-block:: bash

    pip3 install -U pyyaml


.. _json_schema:

jsonschema
^^^^^^^^^^

`jsonschema <https://python-jsonschema.readthedocs.io/>`_ is an implementation of the JSON Schema specification for
Python.

Install ``jsonschema`` by executing the following command:

.. code-block:: bash

    pip3 install -U jsonschema

.. _dependencies:

Dependencies
------------

|ddsrecorder| has the following dependencies, when installed from sources in a Linux environment:

* :ref:`asiotinyxml2_sl`
* :ref:`openssl_sl`
* :ref:`yaml_cpp`
* :ref:`eprosima_dependencies`

.. _asiotinyxml2_sl:

Asio and TinyXML2 libraries
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Asio is a cross-platform C++ library for network and low-level I/O programming, which provides a consistent
asynchronous model.
TinyXML2 is a simple, small and efficient C++ XML parser.
Install these libraries using the package manager of the appropriate Linux distribution.
For example, on Ubuntu use the command:

.. code-block:: bash

    sudo apt install libasio-dev libtinyxml2-dev

.. _openssl_sl:

OpenSSL
^^^^^^^

OpenSSL is a robust toolkit for the TLS and SSL protocols and a general-purpose cryptography library.
Install OpenSSL_ using the package manager of the appropriate Linux distribution.
For example, on Ubuntu use the command:

.. code-block:: bash

   sudo apt install libssl-dev

.. _yaml_cpp:

yaml-cpp
^^^^^^^^

yaml-cpp is a YAML parser and emitter in C++ matching the YAML 1.2 spec, and is used by *DDS Recorder* application to
parse the provided configuration files.
Install yaml-cpp using the package manager of the appropriate Linux distribution.
For example, on Ubuntu use the command:

.. code-block:: bash

   sudo apt install libyaml-cpp-dev

.. _eprosima_dependencies:

eProsima dependencies
^^^^^^^^^^^^^^^^^^^^^

If it already exists in the system an installation of *Fast DDS* and *DDS Router* libraries, just source
this libraries when building the |ddsrecorder| application by using the command:

.. code-block:: bash

    source <fastdds-installation-path>/install/setup.bash
    source <ddsrouter-installation-path>/install/setup.bash

In other case, just download *Fast DDS* and *DDS Router* projects from sources and build it together with |ddsrecorder| using colcon
as it is explained in section :ref:`colcon_installation`.


.. _colcon_installation:

Colcon installation
===================

#.  Create a :code:`DDS-Recorder` directory and download the :code:`.repos` file that will be used to install
    |ddsrecorder| and its dependencies:

    .. code-block:: bash

        mkdir -p ~/DDS-Recorder/src
        cd ~/DDS-Recorder
        wget https://raw.githubusercontent.com/eProsima/DDS-Recorder/main/ddsrecorder.repos
        vcs import src < ddsrecorder.repos

    .. note::

        In case there is already a *Fast DDS* installation in the system it is not required to download and build
        every dependency in the :code:`.repos` file.
        It is just needed to download and build the |ddsrecorder| project having sourced its dependencies.
        Refer to section :ref:`eprosima_dependencies` in order to check how to source *Fast DDS* library.

#.  Build the packages:

    .. code-block:: bash

        colcon build

.. note::

    Being based on CMake_, it is possible to pass the CMake configuration options to the :code:`colcon build`
    command. For more information on the specific syntax, please refer to the
    `CMake specific arguments <https://colcon.readthedocs.io/en/released/reference/verb/build.html#cmake-specific-arguments>`_
    page of the colcon_ manual.


.. _cmake_installation:

CMake installation
==================

This section explains how to compile |ddsrecorder| with CMake_, either
:ref:`locally <local_installation_sl>` or :ref:`globally <global_installation_sl>`.

.. _local_installation_sl:

Local installation
------------------

#.  Create a :code:`DDS-Recorder` directory where to download and build |ddsrecorder| and its dependencies:

    .. code-block:: bash

        mkdir -p ~/DDS-Recorder/src
        mkdir -p ~/DDS-Recorder/build
        cd ~/DDS-Recorder
        wget https://raw.githubusercontent.com/eProsima/DDS-Recorder/main/ddsrecorder.repos
        vcs import src < ddsrecorder.repos

#.  Compile all dependencies using CMake_.

    * `Foonathan memory <https://github.com/foonathan/memory>`_

        .. code-block:: bash

            cd ~/DDS-Recorder
            mkdir build/foonathan_memory_vendor
            cd build/foonathan_memory_vendor
            cmake ~/DDS-Recorder/src/foonathan_memory_vendor -DCMAKE_INSTALL_PREFIX=~/DDS-Recorder/install -DBUILD_SHARED_LIBS=ON
            cmake --build . --target install

    * `Fast CDR <https://github.com/eProsima/Fast-CDR>`_

        .. code-block:: bash

            cd ~/DDS-Recorder
            mkdir build/fastcdr
            cd build/fastcdr
            cmake ~/DDS-Recorder/src/fastcdr -DCMAKE_INSTALL_PREFIX=~/DDS-Recorder/install
            cmake --build . --target install

    * `Fast DDS <https://github.com/eProsima/Fast-DDS>`_

        .. code-block:: bash

            cd ~/DDS-Recorder
            mkdir build/fastdds
            cd build/fastdds
            cmake ~/DDS-Recorder/src/fastdds -DCMAKE_INSTALL_PREFIX=~/DDS-Recorder/install -DCMAKE_PREFIX_PATH=~/DDS-Recorder/install
            cmake --build . --target install

    * `Dev Utils <https://github.com/eProsima/dev-utils>`_

        .. code-block:: bash

            # CMake Utils
            cd ~/DDS-Recorder
            mkdir build/cmake_utils
            cd build/cmake_utils
            cmake ~/DDS-Recorder/src/dev-utils/cmake_utils -DCMAKE_INSTALL_PREFIX=~/DDS-Recorder/install -DCMAKE_PREFIX_PATH=~/DDS-Recorder/install
            cmake --build . --target install

            # C++ Utils
            cd ~/DDS-Recorder
            mkdir build/cpp_utils
            cd build/cpp_utils
            cmake ~/DDS-Recorder/src/dev-utils/cpp_utils -DCMAKE_INSTALL_PREFIX=~/DDS-Recorder/install -DCMAKE_PREFIX_PATH=~/DDS-Recorder/install
            cmake --build . --target install

    * `DDS Router <https://github.com/eProsima/DDS-Router>`_

        .. code-block:: bash

            # ddsrouter_core
            cd ~/DDS-Router
            mkdir build/ddsrouter_core
            cd build/ddsrouter_core
            cmake ~/DDS-Router/src/ddsrouter/ddsrouter_core -DCMAKE_INSTALL_PREFIX=~/DDS-Router/install -DCMAKE_PREFIX_PATH=~/DDS-Router/install
            cmake --build . --target install

            # ddsrouter_yaml
            cd ~/DDS-Router
            mkdir build/ddsrouter_yaml
            cd build/ddsrouter_yaml
            cmake ~/DDS-Router/src/ddsrouter/ddsrouter_yaml -DCMAKE_INSTALL_PREFIX=~/DDS-Router/install -DCMAKE_PREFIX_PATH=~/DDS-Router/install
            cmake --build . --target install

            # ddsrouter_tool
            cd ~/DDS-Router
            mkdir build/ddsrouter_tool
            cd build/ddsrouter_tool
            cmake ~/DDS-Router/src/ddsrouter/tools/ddsrouter_tool -DCMAKE_INSTALL_PREFIX=~/DDS-Router/install -DCMAKE_PREFIX_PATH=~/DDS-Router/install
            cmake --build . --target install

#.  Once all dependencies are installed, install |ddsrecorder|:

    .. code-block:: bash

        # ddsrecorder
        cd ~/DDS-Recorder
        mkdir build/ddsrecorder
        cd build/ddsrecorder
        cmake ~/DDS-Recorder/src/ddsrecorder/ddsrecorder -DCMAKE_INSTALL_PREFIX=~/DDS-Recorder/install -DCMAKE_PREFIX_PATH=~/DDS-Recorder/install
        cmake --build . --target install

        # ddsrecorder_yaml
        cd ~/DDS-Recorder
        mkdir build/ddsrecorder_yaml
        cd build/ddsrecorder_yaml
        cmake ~/DDS-Recorder/src/ddsrecorder/ddsrecorder_yaml -DCMAKE_INSTALL_PREFIX=~/DDS-Recorder/install -DCMAKE_PREFIX_PATH=~/DDS-Recorder/install
        cmake --build . --target install

        # ddsrecorder_tool
        cd ~/DDS-Recorder
        mkdir build/ddsrecorder_tool
        cd build/ddsrecorder_tool
        cmake ~/DDS-Recorder/src/ddsrecorder/tools/ddsrecorder_tool -DCMAKE_INSTALL_PREFIX=~/DDS-Recorder/install -DCMAKE_PREFIX_PATH=~/DDS-Recorder/install
        cmake --build . --target install

.. note::

    By default, |ddsrecorder| does not compile tests.
    However, they can be activated by downloading and installing `Gtest <https://github.com/google/googletest>`_
    and building with CMake option ``-DBUILD_TESTS=ON``.


.. _global_installation_sl:

Global installation
-------------------

To install |ddsrecorder| system-wide instead of locally, remove all the flags that
appear in the configuration steps of :code:`Fast-CDR`, :code:`Fast-DDS`, :code:`Dev-Utils`, :code:`DDS-Router`, and
:code:`DDS-Recorder`, and change the first in the configuration step of :code:`foonathan_memory_vendor` to the
following:

.. code-block:: bash

    -DCMAKE_INSTALL_PREFIX=/usr/local/ -DBUILD_SHARED_LIBS=ON

.. _run_app_colcon_sl:

Run an application
==================

To run the |ddsrecorder| application, source the *Fast DDS* library
and execute the executable file that has been installed in :code:`<install-path>/ddsrecorder_tool/bin/ddsrecorder`:

.. code-block:: bash

    # If built has been done using colcon, all projects could be sourced as follows
    source install/setup.bash
    ./<install-path>/ddsrecorder_tool/bin/ddsrecorder

Be sure that this executable has execute permissions.

.. External links

.. _colcon: https://colcon.readthedocs.io/en/released/
.. _CMake: https://cmake.org
.. _pip: https://pypi.org/project/pip/
.. _wget: https://www.gnu.org/software/wget/
.. _git: https://git-scm.com/
.. _OpenSSL: https://www.openssl.org/
.. _Gtest: https://github.com/google/googletest
.. _vcstool: https://pypi.org/project/vcstool/
