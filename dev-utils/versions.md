# dev-utils Versions

This file includes the released versions of **dev-utils** project along with their contributions to the project.
The *Forthcoming* section includes those features added in `main` branch that are not yet in a stable release.

## Forthcoming

## Version 0.6.0

This release includes the following features in `cmake-utils` project:
* Extend `find_packages` function to load module dependency versions.

This release includes the following features in `cpp-utils` project:
* New **Log Configuration** class to configure the **Log** class.
* Split the **CustomStdLogConsumer** class into a **BaseLogConsumer** class that filters logs based on their kind and content and a **StdLogConsumer** class that prints the logs through std.
* Add function to convert a string with a floating number followed by either the letters B, KB, MB, GB, TB, PB or KiB, MiB, GiB, TiB, PiB to bytes.

This release includes the following **dependencies update**:

|  | Repository | Old Version | New Version |
|---|---|---|---|
| Foonathan Memory Vendor | [eProsima/foonathan_memory_vendor](https://github.com/eProsima/foonathan_memory_vendor) | [v1.3.1](https://github.com/eProsima/foonathan_memory_vendor/releases/tag/v1.3.1) | [v1.3.1](https://github.com/eProsima/foonathan_memory_vendor/releases/tag/v1.3.1) |
| Fast CDR | [eProsima/Fast-CDR](https://github.com/eProsima/Fast-CDR) | [v2.1.3](https://github.com/eProsima/Fast-CDR/releases/tag/v2.1.3) | [v2.2.0](https://github.com/eProsima/Fast-CDR/releases/tag/v2.2.0) |
| Fast DDS | [eProsima/Fast-DDS](https://github.com/eProsima/Fast-DDS) | [v2.13.1](https://github.com/eProsima/Fast-DDS/releases/tag/v2.13.1) | [v2.14.0](https://github.com/eProsima/Fast-DDS/releases/tag/v2.14.0) |

## Version 0.5.0

This release includes the following features in `cmake-utils` project:
* Add `MODULE_PUBLIC_EXTRA_HEADERS` and `MODULE_PRIVATE_EXTRA_HEADERS` as new options to add extra includes path.
* Introduce optional arguments in `compile_library` function to explicitly set source files.
* Add optional arguments to `all_library_sources` and 'add_unittest_executable` functions to explicitly set source files.

This release includes the following features in `cpp-utils` project:
* Add base64 string encoder and decoder.
* Append tree leaf nodes (dependencies) before the trunk.
* Add ROS 2 mangling methods.
* Fix `ros2_msgs_format` function.
* Add missing thread library.
* Add function to convert FuzzyLevel to string.

This release includes the following **dependencies update**:

|  | Repository | Old Version | New Version |
|---|---|---|---|
| Foonathan Memory Vendor | [eProsima/foonathan_memory_vendor](https://github.com/eProsima/foonathan_memory_vendor) | [v1.3.1](https://github.com/eProsima/foonathan_memory_vendor/releases/tag/v1.3.1) | [v1.3.1](https://github.com/eProsima/foonathan_memory_vendor/releases/tag/v1.3.1) |
| Fast CDR | [eProsima/Fast-CDR](https://github.com/eProsima/Fast-CDR) | [v1.1.0](https://github.com/eProsima/Fast-CDR/releases/tag/v1.1.0) | [v2.1.3](https://github.com/eProsima/Fast-CDR/releases/tag/v2.1.3) |
| Fast DDS | [eProsima/Fast-DDS](https://github.com/eProsima/Fast-DDS) | [v2.11.0](https://github.com/eProsima/Fast-DDS/releases/tag/v2.11.0) | [v2.13.1](https://github.com/eProsima/Fast-DDS/releases/tag/v2.13.1) |

## Version 0.4.0

* CI new features:
  * Update CI to use `eProsima CI <https://github.com/eProsima/eProsima-CI>`__.
* New package **py_utils** with Python utils:
  * logging utils
  * debugging utils
  * WaitHandlers implementations
  * Timer implementations
* Sub-packages updated:
  * **cmake_utils**
    * Refactor the way to set labels to tests.
  * **cpp_utils**
    * Add some utils functions.
* Dependencies update:
    |  | Repository | Old Version | New Version |
    |---|---|---|---|
    | Foonathan Memory Vendor | [eProsima/foonathan_memory_vendor](https://github.com/eProsima/foonathan_memory_vendor) | [v1.3.0](https://github.com/eProsima/foonathan_memory_vendor/releases/tag/v1.3.0) | [v1.3.1](https://github.com/eProsima/foonathan_memory_vendor/releases/tag/v1.3.1) |
    | Fast CDR | [eProsima/Fast-CDR](https://github.com/eProsima/Fast-CDR) | [v1.0.27](https://github.com/eProsima/Fast-CDR/releases/tag/v1.0.27) | [v1.1.0](https://github.com/eProsima/Fast-CDR/releases/tag/v1.1.0) |
    | Fast DDS | [eProsima/Fast-DDS](https://github.com/eProsima/Fast-DDS) | [v2.10.1](https://github.com/eProsima/Fast-DDS/releases/tag/v2.10.1) | [v2.11.0](https://github.com/eProsima/Fast-DDS/releases/tag/v2.11.0) |

## Version 0.3.0

* New event class **StdinEventHandler** to easily read from std::cin.
* New event class **CommandReader** to easily read from std::cin commands synchronously.
* New contariner class **SafeDatabase** to implement a thread safe map.
* New contariner class **Tree** to implement a thread safe map.
* Extend **enumeration builder**.
* Extend **iterative macros** max range.
* Add some utils functions.

## Version 0.2.0

* New sub-packages included:
  * **dev_utils** package with tools for programmers to help in implementation process
    * Code auto-generator for Custom Enumerations.

* Sub-packages updated:
  * **cmake_utils**
    * Support minimum version requirement when finding a package.
  * **cpp_utils**
    * Logging module
      * Simplify and fixed TSAN issue.
    * Math module
      * New Random Manager class
    * Types module
      * New Singleton Auxiliary class

* CI new features:
  * Address Sanitizer check for all tests.
  * Thread Sanitizer check for all tests.
  * `-Wall` warning level for CI.
  * Add Python linter check in CI.

## Version 0.1.0

* First version of **dev-utils**.
* Packages and functionality included:
  * **cmake_utils**: tools for CMake builds and common macros and functions.
  * **cpp_utils**: C++ library for utilities and implementations.
    * Event module
    * Exception module
    * Logging module
    * Macros module
    * Math module
    * Memory module
    * Pool module
    * Testing module
    * ThreadPool module
    * Time module
    * Types module
    * Wait module

* CI features:
  * CI based on Github actions.
  * Test for all the main modules.
