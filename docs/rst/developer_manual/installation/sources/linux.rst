.. include:: ../../../exports/alias.include
.. include:: ../../../exports/roles.include

.. _developer_manual_installation_sources_linux:

###############################
Linux installation from sources
###############################

The instructions for installing the |eddsrecord| from sources and its required dependencies are provided in this page.
It is organized as follows:

.. contents::
    :local:
    :backlinks: none
    :depth: 2


Dependencies installation
=========================

|ddsrecord| depends on *eProsima Fast DDS* library and certain Debian packages.
This section describes the instructions for installing |ddsrecord| dependencies and requirements in a Linux environment from sources.
The following packages will be installed:

- ``foonathan_memory_vendor``, an STL compatible C++ memory allocation library.
- ``fastcdr``, a C++ library that serializes according to the standard CDR serialization mechanism.
- ``fastrtps``, the core library of eProsima Fast DDS library.
- ``cmake_utils``, an eProsima utils library for CMake.
- ``cpp_utils``, an eProsima utils library for C++.
- ``ddspipe``, an eProsima internal library that enables the communication of DDS interfaces.

First of all, the :ref:`Requirements <requirements>` and :ref:`Dependencies <dependencies>` detailed below need to be met.
Afterwards, the user can choose whether to follow either the :ref:`colcon <colcon_installation>` or the
:ref:`CMake <cmake_installation>` installation instructions.

.. _requirements:

Requirements
------------

The installation of |eddsrecord| in a Linux environment from sources requires the following tools to be installed in the system:

* :ref:`cmake_gcc_pip_wget_git_sl`
* :ref:`colcon_install` [optional]
* :ref:`fastdds_python` [for remote controller only]
* :ref:`gtest_sl` [for test only]


.. _cmake_gcc_pip_wget_git_sl:

CMake, g++, pip, wget and git
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

These packages provide the tools required to install |eddsrecord| and its dependencies from command line.
Install CMake_, `g++ <https://gcc.gnu.org/>`_, pip_, wget_ and git_ using the package manager of the appropriate Linux distribution.
For example, on Ubuntu use the command:

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


.. _fastdds_python:

Fast DDS Python
^^^^^^^^^^^^^^^

`eProsima Fast DDS Python <https://github.com/eProsima/Fast-DDS-python/>`_ is a Python binding for the eProsima Fast DDS C++ library.
It is only required for the :ref:`remote controller application <recorder_remote_controller>`.

Clone the Github repository into the |eddsrecord| workspace and compile it with colcon_ as a dependency package.
Use the following command to download the code:

.. code-block:: bash

    git clone https://github.com/eProsima/Fast-DDS-python.git src/Fast-DDS-python


.. _gtest_sl:

Gtest
^^^^^

Gtest_ is a unit testing library for C++.
By default, |eddsrecord| does not compile tests.
It is possible to activate them with the opportune `CMake options <https://colcon.readthedocs.io/en/released/reference/verb/build.html#cmake-options>`_ when calling colcon_ or CMake_.
For more details, please refer to the :ref:`cmake_options` section.
For a detailed description of the Gtest_ installation process, please refer to the `Gtest Installation Guide <https://github.com/google/googletest>`_.

It is also possible to clone the Gtest_ Github repository into the |eddsrecord| workspace and compile it with colcon_ as a dependency package.
Use the following command to download the code:

.. code-block:: bash

    git clone --branch release-1.11.0 https://github.com/google/googletest src/googletest-distribution


.. _dependencies:

Dependencies
------------

|eddsrecord| has the following dependencies, when installed from sources in a Linux environment:

* :ref:`asiotinyxml2_sl`
* :ref:`openssl_sl`
* :ref:`yaml_cpp`
* :ref:`swig_sl` [for remote controller only]
* :ref:`pyqt6_linux_dependencies` [for remote controller only]
* :ref:`mcap_dependencies`
* :ref:`eprosima_dependencies`

.. _asiotinyxml2_sl:

Asio and TinyXML2 libraries
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Asio is a cross-platform C++ library for network and low-level I/O programming, which provides a consistent asynchronous model.
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

yaml-cpp is a YAML parser and emitter in C++ matching the YAML 1.2 spec, and is used by |ddsrecord| application to parse the provided configuration files.
Install yaml-cpp using the package manager of the appropriate Linux distribution.
For example, on Ubuntu use the command:

.. code-block:: bash

   sudo apt install libyaml-cpp-dev

.. _swig_sl:

SWIG
^^^^

`SWIG <https://www.swig.org>`_ is a software development tool that connects programs written in C and C++ with a variety of high-level programming languages.
It is leveraged by :ref:`Fast DDS Python <fastdds_python>` to generate a Python wrapper over Fast DDS library.
SWIG is only a requirement for the :ref:`remote controller application <recorder_remote_controller>`.
It can be installed using the package manager of the appropriate Linux distribution.
For example, on Ubuntu use the command:

.. code-block:: bash

   sudo apt install swig libpython3-dev

.. _pyqt6_linux_dependencies:

PyQt6
^^^^^

The |eddsrecord| remote controller is a graphical user interface application implemented in Python using `PyQt6 <https://pypi.org/project/PyQt6/>`__.
To install PyQt6 simply run:

.. code-block:: bash

    pip3 install PyQt6

.. note::

    To install PyQt6 on Ubuntu 20.04, update ``pip`` and ``setuptools`` packages first.

    .. code-block:: bash

        python3 -m pip install pip setuptools --upgrade

.. _mcap_dependencies:

MCAP dependencies
^^^^^^^^^^^^^^^^^

`MCAP <https://github.com/foxglove/mcap>`_ is a modular container format and logging library for pub/sub messages with arbitrary message serialization.
It is primarily intended for use in robotics applications, and works well under various workloads, resource constraints, and durability requirements.
MCAP C++ library is packed within |ddsrecord| as a header-only, but its dependencies need to be installed using the package manager of the appropriate Linux distribution.
For example, on Ubuntu use the command:

.. code-block:: bash

   sudo apt install liblz4-dev libzstd-dev

.. note::

    To work with MCAP files via command line interface, you can use `MCAP CLI <https://mcap.dev/guides/cli>` _ to manage the data in MCAP files.

.. _eprosima_dependencies:

eProsima dependencies
^^^^^^^^^^^^^^^^^^^^^

If it already exists in the system an installation of *Fast DDS* and *DDS Pipe* libraries, just source this libraries when building |eddsrecord| by running the following commands.
In other case, just skip this step.

.. code-block:: bash

    source <fastdds-installation-path>/install/setup.bash
    source <ddspipe-installation-path>/install/setup.bash



.. _colcon_installation:

Colcon installation (recommended)
=================================

#.  Create a :code:`DDS-Record-Replay` directory and download the :code:`.repos` file that will be used to install |eddsrecord| and its dependencies:

    .. code-block:: bash

        mkdir -p ~/DDS-Record-Replay/src
        cd ~/DDS-Record-Replay
        wget https://raw.githubusercontent.com/eProsima/DDS-Record-Replay/main/ddsrecordreplay.repos
        vcs import src < ddsrecordreplay.repos

    .. note::

        In case there is already a *Fast DDS* installation in the system it is not required to download and build every dependency in the :code:`.repos` file.
        It is just needed to download and build the |eddsrecord| project having sourced its dependencies.
        Refer to section :ref:`eprosima_dependencies` in order to check how to source *Fast DDS* library.

#.  Build the packages:

    .. code-block:: bash

        colcon build

.. note::

    To install |ddsrecorder| :ref:`remote controller application <recorder_remote_controller>`, compilation flag ``-DBUILD_DDSRECORDER_CONTROLLER=ON`` is required.

.. note::

    Being based on CMake_, it is possible to pass the CMake configuration options to the :code:`colcon build` command.
    For more information on the specific syntax, please refer to the `CMake specific arguments <https://colcon.readthedocs.io/en/released/reference/verb/build.html#cmake-specific-arguments>`_ page of the colcon_ manual.


.. _cmake_installation:

CMake installation
==================

This section explains how to compile |eddsrecord| with CMake_, either :ref:`locally <local_installation_sl>` or :ref:`globally <global_installation_sl>`.

.. _local_installation_sl:

Local installation
------------------

#.  Create a :code:`DDS-Record-Replay` directory where to download and build |ddsrecord| and its dependencies:

    .. code-block:: bash

        mkdir -p ~/DDS-Record-Replay/src
        mkdir -p ~/DDS-Record-Replay/build
        cd ~/DDS-Record-Replay
        wget https://raw.githubusercontent.com/eProsima/DDS-Record-Replay/main/ddsrecordreplay.repos
        vcs import src < ddsrecordreplay.repos

#.  Compile all dependencies using CMake_.

    * `Foonathan memory <https://github.com/foonathan/memory>`_

        .. code-block:: bash

            cd ~/DDS-Record-Replay
            mkdir build/foonathan_memory_vendor
            cd build/foonathan_memory_vendor
            cmake ~/DDS-Record-Replay/src/foonathan_memory_vendor -DCMAKE_INSTALL_PREFIX=~/DDS-Record-Replay/install -DBUILD_SHARED_LIBS=ON
            cmake --build . --target install

    * `Fast CDR <https://github.com/eProsima/Fast-CDR>`_

        .. code-block:: bash

            cd ~/DDS-Record-Replay
            mkdir build/fastcdr
            cd build/fastcdr
            cmake ~/DDS-Record-Replay/src/fastcdr -DCMAKE_INSTALL_PREFIX=~/DDS-Record-Replay/install
            cmake --build . --target install

    * `Fast DDS <https://github.com/eProsima/Fast-DDS>`_

        .. code-block:: bash

            cd ~/DDS-Record-Replay
            mkdir build/fastdds
            cd build/fastdds
            cmake ~/DDS-Record-Replay/src/fastdds -DCMAKE_INSTALL_PREFIX=~/DDS-Record-Replay/install -DCMAKE_PREFIX_PATH=~/DDS-Record-Replay/install
            cmake --build . --target install

    * `Dev Utils <https://github.com/eProsima/dev-utils>`_

        .. code-block:: bash

            # CMake Utils
            cd ~/DDS-Record-Replay
            mkdir build/cmake_utils
            cd build/cmake_utils
            cmake ~/DDS-Record-Replay/src/dev-utils/cmake_utils -DCMAKE_INSTALL_PREFIX=~/DDS-Record-Replay/install -DCMAKE_PREFIX_PATH=~/DDS-Record-Replay/install
            cmake --build . --target install

            # C++ Utils
            cd ~/DDS-Record-Replay
            mkdir build/cpp_utils
            cd build/cpp_utils
            cmake ~/DDS-Record-Replay/src/dev-utils/cpp_utils -DCMAKE_INSTALL_PREFIX=~/DDS-Record-Replay/install -DCMAKE_PREFIX_PATH=~/DDS-Record-Replay/install
            cmake --build . --target install

    * `DDS Pipe <https://github.com/eProsima/DDS-Pipe>`_

        .. code-block:: bash

            # ddspipe_core
            cd ~/DDS-Record-Replay
            mkdir build/ddspipe_core
            cd build/ddspipe_core
            cmake ~/DDS-Record-Replay/src/ddspipe/ddspipe_core -DCMAKE_INSTALL_PREFIX=~/DDS-Record-Replay/install -DCMAKE_PREFIX_PATH=~/DDS-Record-Replay/install
            cmake --build . --target install

            # ddspipe_participants
            cd ~/DDS-Record-Replay
            mkdir build/ddspipe_participants
            cd build/ddspipe_participants
            cmake ~/DDS-Record-Replay/src/ddspipe/ddspipe_participants -DCMAKE_INSTALL_PREFIX=~/DDS-Record-Replay/install -DCMAKE_PREFIX_PATH=~/DDS-Record-Replay/install
            cmake --build . --target install

            # ddspipe_yaml
            cd ~/DDS-Record-Replay
            mkdir build/ddspipe_yaml
            cd build/ddspipe_yaml
            cmake ~/DDS-Record-Replay/src/ddspipe/ddspipe_yaml -DCMAKE_INSTALL_PREFIX=~/DDS-Record-Replay/install -DCMAKE_PREFIX_PATH=~/DDS-Record-Replay/install
            cmake --build . --target install

#.  Once all dependencies are installed, install |eddsrecord|:

    .. code-block:: bash

        # ddsrecorder_participants
        cd ~/DDS-Record-Replay
        mkdir build/ddsrecorder_participants
        cd build/ddsrecorder_participants
        cmake ~/DDS-Record-Replay/src/ddsrecorder/ddsrecorder_participants -DCMAKE_INSTALL_PREFIX=~/DDS-Record-Replay/install -DCMAKE_PREFIX_PATH=~/DDS-Record-Replay/install
        cmake --build . --target install

        # ddsrecorder_yaml
        cd ~/DDS-Record-Replay
        mkdir build/ddsrecorder_yaml
        cd build/ddsrecorder_yaml
        cmake ~/DDS-Record-Replay/src/ddsrecorder/ddsrecorder_yaml -DCMAKE_INSTALL_PREFIX=~/DDS-Record-Replay/install -DCMAKE_PREFIX_PATH=~/DDS-Record-Replay/install
        cmake --build . --target install

        # ddsrecorder
        cd ~/DDS-Record-Replay
        mkdir build/ddsrecorder_tool
        cd build/ddsrecorder_tool
        cmake ~/DDS-Record-Replay/src/ddsrecorder/ddsrecorder -DCMAKE_INSTALL_PREFIX=~/DDS-Record-Replay/install -DCMAKE_PREFIX_PATH=~/DDS-Record-Replay/install
        cmake --build . --target install

        # ddsreplayer
        cd ~/DDS-Record-Replay
        mkdir build/ddsreplayer_tool
        cd build/ddsreplayer_tool
        cmake ~/DDS-Record-Replay/src/ddsrecorder/ddsreplayer -DCMAKE_INSTALL_PREFIX=~/DDS-Record-Replay/install -DCMAKE_PREFIX_PATH=~/DDS-Record-Replay/install
        cmake --build . --target install

    .. note::

        By default, |eddsrecord| does not compile tests.
        However, they can be activated by downloading and installing `Gtest <https://github.com/google/googletest>`_
        and building with CMake option ``-DBUILD_TESTS=ON``.


#.  Optionally, install the :ref:`remote controller application <recorder_remote_controller>` along with its dependency :ref:`Fast DDS Python <fastdds_python>`:

    .. code-block:: bash

        # Fast DDS Python
        cd ~/DDS-Record-Replay
        mkdir build/fastdds_python
        cd build/fastdds_python
        cmake ~/DDS-Record-Replay/src/Fast-DDS-python/fastdds_python -DCMAKE_INSTALL_PREFIX=~/DDS-Record-Replay/install -DCMAKE_PREFIX_PATH=~/DDS-Record-Replay/install
        cmake --build . --target install

        # Remote Controller Application
        cd ~/DDS-Record-Replay
        mkdir build/controller_tool
        cd build/controller_tool
        cmake ~/DDS-Record-Replay/src/ddsrecorder/controller/controller_tool -DCMAKE_INSTALL_PREFIX=~/DDS-Record-Replay/install -DCMAKE_PREFIX_PATH=~/DDS-Record-Replay/install -DBUILD_DDSRECORDER_CONTROLLER=ON
        cmake --build . --target install


.. _global_installation_sl:

Global installation
-------------------

To install |eddsrecord| system-wide instead of locally, remove all the flags that appear in the configuration steps of :code:`Fast-CDR`, :code:`Fast-DDS`, :code:`Dev-Utils`, :code:`DDS-Pipe`, and :code:`DDS-Record-Replay`, and change the first in the configuration step of :code:`foonathan_memory_vendor` to the following:

.. code-block:: bash

    -DCMAKE_INSTALL_PREFIX=/usr/local/ -DBUILD_SHARED_LIBS=ON

.. _run_app_colcon_sl:

Run an application
==================

To run the |ddsrecorder| tool, source the installation path and execute the executable file that has been installed in :code:`<install-path>/ddsrecorder_tool/bin/ddsrecorder`:

.. code-block:: bash

    # If built has been done using colcon, all projects could be sourced as follows
    source install/setup.bash
    ./<install-path>/ddsrecorder_tool/bin/ddsrecorder

Likewise, to run the |ddsreplay|, source the installation path and execute the executable file that has been installed in :code:`<install-path>/ddsreplayer_tool/bin/ddsreplayer`:

.. code-block:: bash

    # If built has been done using colcon, all projects could be sourced as follows
    source install/setup.bash
    ./<install-path>/ddsreplayer_tool/bin/ddsreplayer

Be sure that these executables have execution permissions.

.. External links

.. _colcon: https://colcon.readthedocs.io/en/released/
.. _CMake: https://cmake.org
.. _pip: https://pypi.org/project/pip/
.. _wget: https://www.gnu.org/software/wget/
.. _git: https://git-scm.com/
.. _OpenSSL: https://www.openssl.org/
.. _Gtest: https://github.com/google/googletest
.. _vcstool: https://pypi.org/project/vcstool/
