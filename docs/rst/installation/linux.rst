.. include:: ../exports/alias.include

.. _installation_manual_linux:

##################
Install on Linux
##################

The verified repository installation path is a source build with colcon. It
installs Recorder, Replayer, and ``mcap-convert`` together with the dependency
versions selected by ``ddsrecordreplay.repos``.

Prerequisites
=============

Install a C++ compiler, CMake, Git, Python 3, pip, and the colcon/vcstool Python
packages. On Ubuntu:

.. code-block:: console

   $ sudo apt update
   $ sudo apt install build-essential cmake git python3-pip python3-venv wget
   $ python3 -m pip install --user colcon-common-extensions vcstool

Build release |release|
=======================

Create a clean workspace and import the dependency manifest:

.. parsed-literal::

   $ mkdir -p ~/DDS-Record-Replay/src
   $ cd ~/DDS-Record-Replay
   $ wget -O ddsrecordreplay.repos |release-url|
   $ vcs import src < ddsrecordreplay.repos
   $ git -C src/ddsrecordreplay checkout |release-tag|
   $ colcon build --cmake-args -DCMAKE_BUILD_TYPE=Release

Source the environment in each new terminal:

.. code-block:: console

   $ source ~/DDS-Record-Replay/install/setup.bash

Verify the installation:

.. code-block:: console

   $ ddsrecorder --version
   $ ddsreplayer --version
   $ mcap-convert --version

Optional graphical controller
=============================

Install PyQt6 and the Fast DDS Python requirements, then rebuild with:

.. code-block:: console

   $ colcon build --cmake-args -DCMAKE_BUILD_TYPE=Release -DBUILD_DDSRECORDER_CONTROLLER=ON

The controller is not required for DDS command-topic integration. See
:ref:`recorder_remote_control` and the :ref:`cmake_options` reference.

Updating or rebuilding
======================

Use a new workspace for another release, or explicitly update the imported
repositories before rebuilding. Mixing install trees from different Fast DDS,
DDS Pipe, and DDS Record & Replay versions can cause build or runtime conflicts.

For tests and documentation builds, continue with
:ref:`developer_manual_installation_sources_linux`.
