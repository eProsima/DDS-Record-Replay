.. include:: ../exports/alias.include

.. _installation_manual_windows:

####################
Install on Windows
####################

Build from source in a **Developer PowerShell for Visual Studio 2022** so the
MSVC compiler environment is active.

Prerequisites
=============

Install Visual Studio 2022 with **Desktop development with C++**, CMake, Git,
and Python 3. Then install colcon and vcstool:

.. code-block:: powershell

   py -m pip install --user colcon-common-extensions vcstool

Ensure the Python user scripts directory is on ``PATH`` before continuing.

Build release |release|
=======================

Run these commands in Developer PowerShell:

.. parsed-literal::

   PS> New-Item -ItemType Directory -Force "$HOME\DDS-Record-Replay\src"
   PS> Set-Location "$HOME\DDS-Record-Replay"
   PS> Invoke-WebRequest -Uri "|release-url|" -OutFile "ddsrecordreplay.repos"
   PS> vcs import src --input ddsrecordreplay.repos
   PS> git -C src\ddsrecordreplay checkout |release-tag|
   PS> colcon build --merge-install --cmake-args -DCMAKE_BUILD_TYPE=Release

Load the environment:

.. code-block:: powershell

   PS> .\install\setup.ps1

If PowerShell policy prevents local scripts, use a Developer Command Prompt and
run ``call install\setup.bat`` instead.

Verify the installation:

.. code-block:: powershell

   PS> ddsrecorder --version
   PS> ddsreplayer --version
   PS> mcap-convert --version

Optional graphical controller
=============================

Install PyQt6 and the Fast DDS Python requirements, then add
``-DBUILD_DDSRECORDER_CONTROLLER=ON`` to the colcon CMake arguments. See
:ref:`recorder_remote_control`.

For test and documentation builds, continue with
:ref:`developer_manual_installation_sources_windows`.
