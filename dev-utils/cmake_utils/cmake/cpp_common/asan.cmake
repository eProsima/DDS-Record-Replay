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
# Activate Address sanitizer
###############################################################################

# Activate Address sanitizer setting compilation flags
macro(activate_address_sanitizer)

    # Warning/Error messages
    if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        message(WARNING "Address sanitizer results with an optimized (non-Debug) build may be misleading")
    endif()

    if("${CMAKE_C_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
        message(STATUS "Building with llvm Address sanitizer Tools")

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

    elseif(CMAKE_COMPILER_IS_GNUCXX)
        message(STATUS "Building with Address sanitizer Tools")

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    else()
        message(FATAL_ERROR "Address sanitizer requires Clang or GCC. Aborting.")
    endif()

endmacro()
