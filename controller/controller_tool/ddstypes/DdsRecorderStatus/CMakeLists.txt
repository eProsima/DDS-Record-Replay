
cmake_minimum_required(VERSION 3.16.3)

# SWIG: use standard target name.
if(POLICY CMP0078)
  cmake_policy(SET CMP0078 NEW)
endif()

# SWIG: use SWIG_MODULE_NAME property.
if(POLICY CMP0086)
  cmake_policy(SET CMP0086 NEW)
endif()

###############################################################################
# Library for types defined in DdsRecorderStatus IDL
###############################################################################

message(STATUS "Configuring python wrapper for types in DdsRecorderStatus...")

###############################################################################
# Type library on C++

project(DdsRecorderStatus)

find_package(fastcdr REQUIRED)
find_package(fastdds REQUIRED)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#Create library for C++ types
add_library(${PROJECT_NAME} SHARED
        DdsRecorderStatusPubSubTypes.cxx
        DdsRecorderStatusTypeObjectSupport.cxx
        )
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE EPROSIMA_USER_DLL_EXPORT)
endif(WIN32)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}
    )
target_link_libraries(${PROJECT_NAME}
    PUBLIC
    fastcdr
    fastdds
    )

###############################################################################
# Python bindings for type

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})
set(CMAKE_SWIG_FLAGS "")

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
set(PYTHON_INCLUDE_PATH ${Python3_INCLUDE_DIRS})
set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
set(PYTHON_LIBRARIES ${Python3_LIBRARIES})

if (WIN32)
    # Detect stale CMake cache cases where interpreter and development library
    # point to different Python ABIs (e.g. python312.lib, python311.lib, cp312 ...)
    set(_python3_library_to_check "${Python3_LIBRARY_RELEASE}")
    if (NOT _python3_library_to_check)
        list(GET Python3_LIBRARIES 0 _python3_library_to_check)
    endif()

    if (_python3_library_to_check)
        get_filename_component(_python3_library_name "${_python3_library_to_check}" NAME)
        string(REGEX MATCH "python([0-9]+)(_d)?\\.lib" _python3_library_match "${_python3_library_name}")
        if (_python3_library_match)
            set(_python3_abi_expected "${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}")
            set(_python3_abi_from_library "${CMAKE_MATCH_1}")
            if (NOT _python3_abi_expected STREQUAL _python3_abi_from_library)
                message(FATAL_ERROR
                    "Python interpreter/development mismatch detected.\n"
                    "Interpreter: ${Python3_EXECUTABLE} (cp${_python3_abi_expected})\n"
                    "Development library: ${_python3_library_to_check} (cp${_python3_abi_from_library})\n"
                    "Clean the build directory (or use --cmake-clean-cache) and reconfigure with "
                    "-DPython3_EXECUTABLE=<path-to-python.exe>."
                )
            endif()
        endif()
    endif()
endif()

include_directories(${PYTHON_INCLUDE_PATH})

set(${PROJECT_NAME}_MODULE
    DdsRecorderStatusWrapper
    )

set(${PROJECT_NAME}_MODULE_FILES
    ${PROJECT_NAME}.i
    )

SET_SOURCE_FILES_PROPERTIES(
    ${${PROJECT_NAME}_MODULE_FILES}
    PROPERTIES CPLUSPLUS ON
    USE_TARGET_INCLUDE_DIRECTORIES TRUE
    )

set_property(SOURCE ${PROJECT_NAME}.i PROPERTY OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/")

SWIG_ADD_LIBRARY(${${PROJECT_NAME}_MODULE}
    TYPE SHARED
    LANGUAGE python
    SOURCES ${${PROJECT_NAME}_MODULE_FILES})

set_property(TARGET ${${PROJECT_NAME}_MODULE} PROPERTY CXX_STANDARD 11)
if(UNIX AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    set_property(TARGET ${${PROJECT_NAME}_MODULE} PROPERTY SWIG_COMPILE_DEFINITIONS SWIGWORDSIZE64)
endif()

target_link_libraries(${${PROJECT_NAME}_MODULE}
    Python3::Module
    fastdds
    ${PROJECT_NAME}
    )

set_target_properties(${${PROJECT_NAME}_MODULE}
    PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/"
    )


# Find the installation path
execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "from distutils import sysconfig; print(sysconfig.get_python_lib(plat_specific=True, prefix='${CMAKE_INSTALL_PREFIX}'))"
    OUTPUT_VARIABLE _ABS_PYTHON_MODULE_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

get_filename_component (_ABS_PYTHON_MODULE_PATH ${_ABS_PYTHON_MODULE_PATH} ABSOLUTE)
file (RELATIVE_PATH _REL_PYTHON_MODULE_PATH ${CMAKE_INSTALL_PREFIX} ${_ABS_PYTHON_MODULE_PATH})
SET (PYTHON_MODULE_PATH
    ${_REL_PYTHON_MODULE_PATH}
    )

# Install
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin/
    LIBRARY DESTINATION lib/
    ARCHIVE DESTINATION lib/
    )
install(TARGETS ${${PROJECT_NAME}_MODULE} DESTINATION ${PYTHON_MODULE_PATH}/)
get_property(support_files TARGET ${${PROJECT_NAME}_MODULE} PROPERTY SWIG_SUPPORT_FILES)
install(FILES ${support_files} DESTINATION ${PYTHON_MODULE_PATH}/)
