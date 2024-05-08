# Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

###############################################################################
# C++ Version
###############################################################################

# Set C++ project version and check it is available and force it
#
# ARGUMENTS:
# - CPP_VERSION: C++ version to use [int format]
macro(force_cpp CPP_VERSION)

    if("${CPP_VERSION}" STREQUAL "")
        message(STATUS "No C++ version forced")
    elseif("${CPP_VERSION}" STREQUAL "11" OR "${CPP_VERSION}" STREQUAL "C++11")
        force_cpp_11()
    elseif("${CPP_VERSION}" STREQUAL "14" OR "${CPP_VERSION}" STREQUAL "C++14")
        force_cpp_14()
    elseif("${CPP_VERSION}" STREQUAL "17" OR "${CPP_VERSION}" STREQUAL "C++17")
        force_cpp_17()
    elseif("${CPP_VERSION}" STREQUAL "20" OR "${CPP_VERSION}" STREQUAL "C++20")
        force_cpp_20()
    else()
        message(FATAL_ERROR "C++ version ${CPP_VERSION} is not supported yet by this cmake macro")
    endif()

endmacro()

macro(force_cpp_11)

    include(CheckCXXCompilerFlag)
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR
            CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
        if(NOT SUPPORTS_CXX11)
            message(FATAL_ERROR "Compiler doesn't support C++11")
        endif()
    endif()

    # Force C++ version
    set(CMAKE_CXX_STANDARD 11)

    # Finish macro
    message(STATUS "C++11 is supported and forced")

endmacro()

macro(force_cpp_14)

    include(CheckCXXCompilerFlag)
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR
            CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        check_cxx_compiler_flag(-std=c++14 SUPPORTS_CXX14)
        if(NOT SUPPORTS_CXX14)
            message(FATAL_ERROR "Compiler doesn't support C++14")
        endif()
    endif()

    # Force C++ version
    set(CMAKE_CXX_STANDARD 14)

    # Finish macro
    message(STATUS "C++14 is supported and forced")

endmacro()

macro(force_cpp_17)

    include(CheckCXXCompilerFlag)
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR
            CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        check_cxx_compiler_flag(-std=c++17 SUPPORTS_CXX17)
        if(NOT SUPPORTS_CXX17)
            message(FATAL_ERROR "Compiler doesn't support C++17")
        endif()
    endif()

    # Force C++ version
    set(CMAKE_CXX_STANDARD 17)

    # Finish macro
    message(STATUS "C++17 is supported and forced")

endmacro()

macro(force_cpp_20)

    include(CheckCXXCompilerFlag)
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR
            CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        check_cxx_compiler_flag(-std=c++20 SUPPORTS_CXX20)
        if(NOT SUPPORTS_CXX20)
            message(FATAL_ERROR "Compiler doesn't support C++20")
        endif()
    endif()

    # Force C++ version
    set(CMAKE_CXX_STANDARD 20)

    # Finish macro
    message(STATUS "C++20 is supported and forced")

endmacro()
