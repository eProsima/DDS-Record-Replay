# eProsima Developers Utils

<a href="http://www.eprosima.com"><img src="https://encrypted-tbn3.gstatic.com/images?q=tbn:ANd9GcSd0PDlVz1U_7MgdTe0FRIWD0Jc9_YH-gGi0ZpLkr-qgCI6ZEoJZ5GBqQ" align="left" hspace="8" vspace="2" width="100" height="100" ></a>

[![License](https://img.shields.io/github/license/eProsima/dev-utils.svg)](https://opensource.org/licenses/Apache-2.0)
[![Releases](https://img.shields.io/github/v/release/eProsima/dev-utils?sort=semver)](https://github.com/eProsima/dev-utils/releases)
[![Issues](https://img.shields.io/github/issues/eProsima/dev-utils.svg)](https://github.com/eProsima/dev-utils/issues)
[![Forks](https://img.shields.io/github/forks/eProsima/dev-utils.svg)](https://github.com/eProsima/dev-utils/network/members)
[![Stars](https://img.shields.io/github/stars/eProsima/dev-utils.svg)](https://github.com/eProsima/dev-utils/stargazers)
[![test](https://github.com/eProsima/dev-utils/actions/workflows/test.yml/badge.svg)](https://github.com/eProsima/dev-utils/actions/workflows/test.yml)
[![codecov](https://codecov.io/gh/eProsima/dev-utils/branch/v0.6.0/graph/badge.svg?token=6NA5PVA9QL)](https://codecov.io/gh/eProsima/dev-utils)

The packages that conform this repository are:

* **CMake utils**: `cmake_utils` CMake utilities to build packages.
* **C++ utils**: `cpp_utils` C++ classes and functions for common use.
* **Dev utils**: `dev_utils` Tools and applications to help in code development.

## Documentation

There is no public documentation for this repository.
Please, refer the the README.md files of each subpackage for information regarding the API and functionality.

## Installation Guide

The instructions for installing the *Dev Utils* application from sources and its required dependencies on a Linux
environment are provided below. These installation instructions are a summarized version of the complete
[installation guide](https://eprosima-dds-router.readthedocs.io/en/latest/rst/developer_manual/installation/sources/linux.html) available online.
<!-- TODO windows or cmake instructions -->

### Requirements

*eProsima Dev Utils* requires the following tools to be installed in the system:
* [CMake](https://cmake.org/), [g++](https://gcc.gnu.org/), [pip](https://pypi.org/project/pip/), [wget](https://www.gnu.org/software/wget/) and [git](https://git-scm.com/)
* [Colcon](https://colcon.readthedocs.io/en/released/) [optional, not required for CMake-only installation]
* [Gtest](https://github.com/google/googletest) [for test only]

#### CMake, g++, pip, wget and git

These packages provide the tools required to install Dev Utils and its dependencies from command line. Install
[CMake](https://cmake.org/), [g++](https://gcc.gnu.org/), [pip](https://pypi.org/project/pip/), [wget](https://www.gnu.org/software/wget/) and [git](https://git-scm.com/) using the package manager of the appropriate Linux distribution. For
example, on Ubuntu use the command:

```bash
sudo apt install cmake g++ pip wget git
```

#### Colcon

[colcon](https://colcon.readthedocs.io/en/released/) is a command line tool based on [CMake](https://cmake.org/) aimed at building sets of software packages. Install the ROS 2 development tools ([colcon](https://colcon.readthedocs.io/en/released/) and [vcstool](https://pypi.org/project/vcstool/)) by executing the following command:

```bash
pip3 install -U colcon-common-extensions vcstool
```

If this fails due to an Environment Error, add the `--user` flag to the `pip3` installation command.

#### Gtest

[Gtest](https://github.com/google/googletest) is a unit testing library for C++. By default, *Dev Utils* does not
compile tests. It is possible to activate them with the opportune [CMake options](https://colcon.readthedocs.io/en/released/reference/verb/build.html#cmake-options) when calling [colcon](https://colcon.readthedocs.io/en/released/) or
[CMake](https://cmake.org/). For a detailed description of the Gtest installation process, please refer to the
[Gtest Installation Guide](https://github.com/google/googletest).

### Dependencies

#### Asio and TinyXML2 libraries

Asio is a cross-platform C++ library for network and low-level I/O programming, which provides a consistent asynchronous
model. TinyXML2 is a simple, small and efficient C++ XML parser. Install these libraries using the package manager of
the appropriate Linux distribution. For example, on Ubuntu use the command:

```bash
sudo apt install libasio-dev libtinyxml2-dev
```

#### OpenSSL

[OpenSSL](https://www.openssl.org/) is a robust toolkit for the TLS and SSL protocols and a general-purpose cryptography
library. Install OpenSSL using the package manager of the appropriate Linux distribution. For example, on Ubuntu use the
command:

```bash
sudo apt install libssl-dev
```

#### eProsima dependencies

If it already exists in the system an installation of *Fast DDS* library with version greater than *2.4.0*, just source
this library when building the *Dev Utils* application by using the command:

```bash
source <fastdds-installation-path>/install/setup.bash
```

In other case, just download *Fast DDS* project from sources and build it together with *Dev Utils* using colcon as it
is explained in the following section.

### Colcon installation

1. Create a `dev-utils` directory and download the `.repos` file that will be used to install *Dev Utils* and its dependencies:

```bash
mkdir -p ~/DDS-Router/src
cd ~/DDS-Router
wget https://raw.githubusercontent.com/eProsima/dev-utils/v0.6.0/dev_utils.repos
vcs import src < dev_utils.repos
```

2. Build the packages:

```bash
colcon build
```

This repository holds several colcon packages.
These packages are:

* `cmake_utils`
* `cpp_utils`

> *NOTE:* Those packages could be installed and use independently (according with each package dependency).
  In order to compile only a package and its dependencies, use the colcon argument `--packages-up-to <package>`.
  In order to explicitly skip some of these packages, use the colcon argument
  `--packages-skip <package1> [<package2> ...]`.

### Testing

By default, *Dev Utils* does not compile tests. However, they can be activated by downloading and installing
[Gtest](https://github.com/google/googletest) and building with CMake option `-DBUILD_TESTS=ON`. Once done, tests
can be run with the following command:

```bash
colcon test --event-handler=console_direct+
```

## Getting Help

If you need support you can reach us by mail at `support@eProsima.com` or by phone at `+34 91 804 34 48`.
