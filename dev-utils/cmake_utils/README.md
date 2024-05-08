# eProsima Developer Cmake Utils module

This package generates CMake functions, macros and templates.
The main idea is to not duplicate common cmake functionalities along different packages.

## Use

This is a very useful package for a developer as it allows to compile a library, documentation or tool abstracting
from cmake functions.

### CMakeLists

First, every module must have its own `CMakeLists.txt`.
This CMake must:

1. Set its version `cmake_minimum_required(VERSION 3.5)`
1. Find this package `find_package(cmake_utils REQUIRES)`
1. Configure project `configure_project()`
1. Call project explicitly `project(${MODULE_NAME} VERSION ${MODULE_VERSION} DESCRIPTION ${MODULE_DESCRIPTION})`

From this point, the CMakeLists has everything loaded and can keep calling cmake_utils macros
without needed to include or load anything.

### Library

In order to compile a C++ library, use the following macros:

1. To configure every C++ required flag and variable: `configure_project_cpp()`
1. To compile library: `compile_library(${<src path>} ${<include path>})`
1. To compile tests for the library: `compile_test(${<test path>})`
1. To install the package following eprosima defaults `eprosima_packaging()`

---

## Module Requirements

In order create a package (called Module from now on) using this library,
three main files are required in the parent directory:

### Project settings

The file `project_settings.cmake` should be a CMake file with definitions of important variables.
Check the table below to see which variables are required and which are useful to use.

### Version

The file `VERSION` must contain 3 lines with the version of this module.
The version will be loaded from this file if it is not set in `project_settings.cmake`.

### LICENSE

Every module requires to have a `LICENSE` file in order to install it with the result of the module.

---

## Project configuration using cmake_utils variables

These are the variables that could/must be set in the `project_settings.cmake` file.
Those variables which default is `x` must be set, and those with `-` are not required.

| Variable | Default | Description |
|------------------------------|-------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------|
| MODULE_NAME | x | Name of the module (must be project name) |
| MODULE_TARGET_NAME | ${MODULE_NAME} | Output name of the target |
| MODULE_NAME_LARGE | ${MODULE_NAME} | Large name |
| MODULE_SUMMARY | ${MODULE_NAME_LARGE} | Summary (short description) |
| MODULE_DESCRIPTION | ${MODULE_SUMMARY} | Description |
| MODULE_MACRO | TOUPPER ${MODULE_NAME} | Macro to use in CMake and C++ definitions (it is recommended to leave it as Uppercase of name) |
| MODULE_HEADERS_PATH | ${MODULE_NAME} | Path (relative to include) where headers to be installed are located |
| MODULE_HEADERS_INSTALL_PATH | ${MODULE_HEADERS_PATH} | Path (relative to install dir) where headers are installed |
|------------------------------|-------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------|
| MODULE_FIND_PACKAGES | - | Modules that require to be found by find_package |
| MODULE_THIRDPARTY_HEADERONLY | - | Headeronly thirdparties that require to be included (must be inside ${MODULE_THIRDPARTY_PATH} dir) |
| MODULE_THIRDPARTY_PATH | ../thirdparty | Thirdparties parent dir path |
| MODULE_DEPENDENCIES | ${MODULE_FIND_PACKAGES} | Libraries that require to be linked by the target |
| MODULE_PUBLIC_EXTRA_HEADERS | - | Specifies public scope include directories to use when compiling a given target. |
| MODULE_PRIVATE_EXTRA_HEADERS | - | Specifies private scope include directories to use when compiling a given target. |
|------------------------------|-------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------|
| MODULE_VERSION_FILE_PATH | ../VERSION | Path to the file containing version information |
| MODULE_VERSION_MAJOR | x (if not set, are taken from VERSION file) | Major version |
| MODULE_VERSION_MINOR | x (if not set, are taken from VERSION file) | Minor version |
| MODULE_VERSION_PATCH | x (if not set, are taken from VERSION file) | Patch version |
| MODULE_VERSION | ${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}.${MODULE_VERSION_PATCH} | Module version |
| MODULE_VERSION_STRING | v${MODULE_VERSION} | Module version |
|------------------------------|-------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------|
| MODULE_LICENSE_FILE_PATH | ../LICENSE | Path to the license file |
| MODULE_RESOURCES_PATH | (if not given, no resources will be installed) | Path of the resources to install |
|------------------------------|-------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------|
| MODULE_BIN_INSTALL_DIR | bin/ | Binary installation path |
| MODULE_INCLUDE_INSTALL_DIR | include/ | Include installation path |
| MODULE_LIB_INSTALL_DIR | lib/ | Library installation path |
| MODULE_DATA_INSTALL_DIR | share/ | Data installation path |
| MODULE_LICENSE_INSTALL_DIR | share/ | License installation path |
|------------------------------|-------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------|
| MODULE_CPP_VERSION | '' | C++ version |

### Minimum Package Version

Setting the CMake variable `<library>_MINIMUM_VERSION` will force the `find_package` call to look for library
`<library>` with minimum version `${<library>_MINIMUM_VERSION}`.

e.g.

```cmake
set(MODULE_FIND_PACKAGES fastrtps)
set(fastrtps_MINIMUM_VERSION "2.8") # This will force to use a version of fastrtps higher or equal 2.8
```

---

## Project using cmake_utils CMake options set

There are default CMake options used within cmake_utils package that will always be set.
In case the user sets these variables, they will have that value. Otherwise,
they will be initialized to their corresponding default value.

| Option Name         | Default value     | Description                                 |
|---------------------|-------------------|---------------------------------------------|
| BUILD_TESTS         | OFF               | Build tests                                 |
| BUILD_DOCS_TESTS    | $BUILD_TESTS      | Build tests only for Documentation packages |
| BUILD_TOOL_TESTS    | $BUILD_TESTS      | Build tests only for Application packages   |
| BUILD_LIBRARY_TESTS | $BUILD_TESTS      | Build tests only for Library packages       |
| BUILD_ALL           | $BUILD_TESTS      | Build package                               |
| BUILD_DOCS          | $BUILD_DOCS_TESTS | Build only Documentation packages           |
| BUILD_TOOL          | ON                | Build only Application packages             |
| BUILD_LIBRARY       | ON                | Build only Library packages                 |
| CODE_COVERAGE       | OFF               | Activate Code Coverage flags                |
| TSAN_BUILD          | OFF               | Activate thread sanitizer flags             |
| ASAN_BUILD          | OFF               | Activate address sanitizer flags            |
| CMAKE_BUILD_TYPE    | Release           | CMake Build Type                            |
| LOG_INFO            | OFF (ON if Debug) | Activate log info verbosity level           |
