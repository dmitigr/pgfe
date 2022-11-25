# -*- cmake -*-
#
# Copyright 2022 Dmitry Igrishin
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

cmake_minimum_required(VERSION 3.16)
cmake_policy(VERSION 3.16)
project(dmitigr_libs)

if (NOT (UNIX OR WIN32))
  message(FATAL_ERROR "Unsupported platform")
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(dmitigr)
include(dmitigr_libs)

# ------------------------------------------------------------------------------
# Build options
# ------------------------------------------------------------------------------

set(DMITIGR_LIBS_CLANG_USE_LIBCPP On CACHE BOOL
  "Use libc++ with Clang?")
set(DMITIGR_LIBS_HEADER_ONLY Off CACHE BOOL
  "Whole header-only?")
set(DMITIGR_LIBS_DOXYGEN Off CACHE BOOL
  "Build configurations for Doxygen?")
set(DMITIGR_LIBS_TESTS Off CACHE BOOL
  "Build tests?")
set(DMITIGR_LIBS_OPENSSL Off CACHE BOOL
  "Link to OpenSSL where possible?")
set(DMITIGR_LIBS_ZLIB Off CACHE BOOL
  "Link to Zlib where possible?")
set(DMITIGR_LIBS_AIO "uv" CACHE STRING
  "What AIO to use? (\"uv\" - is the only option now.)")
set(BUILD_SHARED_LIBS Off CACHE BOOL
  "Build shared libraries?")
set(CMAKE_VERBOSE_MAKEFILE On CACHE BOOL
  "Verbose output upon build?")

# Enable linking with libc++ if requested.
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if (DMITIGR_LIBS_CLANG_USE_LIBCPP)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif()
endif()

# Set default CMAKE_BUILD_TYPE.
if(NOT DEFINED CMAKE_BUILD_TYPE OR NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
    "Build type: Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif()

if (NOT DMITIGR_LIBS_HEADER_ONLY)
  if(BUILD_SHARED_LIBS)
    message("Shared libraries are requested (${CMAKE_BUILD_TYPE})")
  else()
    message("Static libraries are requested (${CMAKE_BUILD_TYPE})")
  endif()
elseif(BUILD_SHARED_LIBS)
  message(FATAL_ERROR "DMITIGR_LIBS_HEADER_ONLY and BUILD_SHARED_LIBS are mutually exclusive")
else()
  message("Only 3rdparties without header-only option will be built")
endif()

if(DMITIGR_LIBS_DOXYGEN)
  message("Doxygen configurations are requested")
endif()

if(DMITIGR_LIBS_TESTS)
  message("Tests are requested (${CMAKE_BUILD_TYPE})")
endif()

string(REGEX MATCH "(uv)$" aio "${DMITIGR_LIBS_AIO}")
if(NOT aio)
  message(FATAL_ERROR "Invalid AIO -- \"${DMITIGR_LIBS_AIO}\"")
endif()

# ------------------------------------------------------------------------------

set(DMITIGR_LIBS_INSTALL On CACHE BOOL
  "Install the stuff?")

if(UNIX)
  set(DMITIGR_LIBS_SHARE_INSTALL_DIR "share/dmitigr_libs" CACHE
    STRING "Name of the installation directory for the shared stuff relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_LIBS_CMAKE_INSTALL_DIR "${DMITIGR_LIBS_SHARE_INSTALL_DIR}/cmake" CACHE
    STRING "Name of the installation directory for the CMake stuff relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_LIBS_LIB_INSTALL_DIR "lib" CACHE
    STRING "Name of the installation directory for the libraries relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_LIBS_INCLUDE_INSTALL_DIR "include" CACHE
    STRING "Name of the installation directory for the includes relative to ${CMAKE_INSTALL_PREFIX}")
elseif(WIN32)
  # On Windows, CMAKE_INSTALL_PREFIX is $ENV{ProgramFiles}\\${CMAKE_PROJECT_NAME} by default. In turn:
  #   - on AMD64: ProgramFiles=%ProgramFiles%
  #   - on   x86: ProgramFiles=%ProgramFiles(x86)%
  # See: https://msdn.microsoft.com/en-us/library/aa384274.aspx
  set(DMITIGR_LIBS_SHARE_INSTALL_DIR "." CACHE
    STRING "Name of the installation directory for the shared stuff relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_LIBS_CMAKE_INSTALL_DIR "cmake" CACHE
    STRING "Name of the installation directory for the CMake stuff relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_LIBS_LIB_INSTALL_DIR "lib" CACHE
    STRING "Name of the installation directory for the libraries relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_LIBS_INCLUDE_INSTALL_DIR "include" CACHE
    STRING "Name of the installation directory for the includes relative to ${CMAKE_INSTALL_PREFIX}")
endif()

# ------------------------------------------------------------------------------
# Languages
# ------------------------------------------------------------------------------

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------
# Get the actual list of libraries
# ------------------------------------------------------------------------------

# The libraries sources root directory.
set(lib_root "src")

# Ensure that dmitigr_libs_package is empty!
set(dmitigr_libs_package)
foreach(lib ${dmitigr_libs})
  set(pref "${CMAKE_CURRENT_SOURCE_DIR}/${lib_root}/${lib}")
  if(EXISTS "${pref}" OR EXISTS "${pref}.hpp")
    list(APPEND dmitigr_libs_package ${lib})
  endif()
endforeach()
unset(pref)

configure_file(cmake/dmitigr_libs_package.cmake.in
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/dmitigr_libs_package.cmake @ONLY
  NEWLINE_STYLE UNIX)

# ------------------------------------------------------------------------------
# Check the dependencies of the actual libraries
# ------------------------------------------------------------------------------

set(content "#!/bin/bash\n# Copyright (C) Dmitry Igrishin\n\n")
set(content "${content}# This file is automatically generated by CMake!\n\n")

# Dump library list
set(content "${content}igrilibs=\"")
foreach(lib ${dmitigr_libs_package})
  set(content "${content}${lib} ")
endforeach()
string(STRIP "${content}" content)
set(content "${content}\"\n")

# Dump dependency lists
foreach(lib ${dmitigr_libs_package})
  set(content "${content}${lib}_deps=\"")
  dmitigr_libs_get_deps(lib_deps "${lib}")
  foreach(dep ${lib_deps})
    # Check that the dependency is present
    string(FIND "${dep}" "3rdparty_" pos)
    if(pos EQUAL 0)
      string(REGEX REPLACE "^3rdparty_(.*)" "3rdparty/\\1" dep_path "${dep}")
    else()
      set(dep_path "${dep}")
    endif()
    set(pref "${CMAKE_CURRENT_SOURCE_DIR}/${lib_root}/${dep_path}")
    if((NOT EXISTS "${pref}") AND (NOT EXISTS "${pref}.hpp"))
      message(FATAL_ERROR "${lib} is depends on ${dep} which is not present")
    endif()

    # Append the content for igrilibs_deps.sh
    set(content "${content}${dep} ")
  endforeach()
  string(STRIP "${content}" content)
  set(content "${content}\"\n")
endforeach()
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tool)
  file(WRITE tool/igrilibs_deps.sh "${content}")
endif()
unset(content)
message("This package contains: ${dmitigr_libs_package}")

# ------------------------------------------------------------------------------
# Third-party dependencies
# ------------------------------------------------------------------------------

if ("rajson" IN_LIST dmitigr_libs_package)
  include(dmitigr_3rdparty_rapidjson)
endif()

if ("ws" IN_LIST dmitigr_libs_package)
  include(dmitigr_3rdparty_usockets)
  include(dmitigr_3rdparty_uwebsockets)
endif()

if ("wscl" IN_LIST dmitigr_libs_package)
  include(dmitigr_3rdparty_uwsc)
endif()

# ------------------------------------------------------------------------------
# Output settings
# ------------------------------------------------------------------------------

# Note: Multi-configuration generators (VS, Xcode) appends a per-configuration
# subdirectory to CMAKE_RUNTIME_OUTPUT_DIRECTORY unless a generator expression
# is used.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

if (WIN32)
  set(dmitigr_libs_resource_destination_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>")
elseif (UNIX)
  set(dmitigr_libs_resource_destination_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

# ------------------------------------------------------------------------------
# Custom targets
# ------------------------------------------------------------------------------

add_custom_target(dmitigr_libs_uninstall)

add_custom_target(dmitigr_libs_create_resource_destination_dir ALL
  COMMAND cmake -E make_directory "${dmitigr_libs_resource_destination_dir}"
  )

# --------------------------------------
# Installing
# --------------------------------------

if(DMITIGR_LIBS_INSTALL)
  install(FILES LICENSE.txt
    DESTINATION ${DMITIGR_LIBS_SHARE_INSTALL_DIR})
  install(FILES
    cmake/dmitigr_libs.cmake
    cmake/dmitigr_libs_package.cmake
    cmake/dmitigr_libs-config.cmake
    DESTINATION ${DMITIGR_LIBS_CMAKE_INSTALL_DIR})
endif()

# ------------------------------------------------------------------------------
# Libraries
# ------------------------------------------------------------------------------

# @param file_list A file list name, e.g. "dmitigr_base_headers".
function(dmitigr_install_lib_includes file_list)
  foreach(file ${${file_list}})
    string(REGEX REPLACE "^${lib_root}/" "" file "${file}")
    get_filename_component(dir ${file} DIRECTORY)
    install(FILES ${lib_root}/${file}
      DESTINATION ${DMITIGR_LIBS_INCLUDE_INSTALL_DIR}/dmitigr/${dir})
  endforeach()
endfunction()

foreach(lib ${dmitigr_libs_package})
  string(TOUPPER "${lib}" LIB)

  set(dmlib dmitigr_${lib})
  set(DMLIB DMITIGR_${LIB})

  include(${dmlib})

  dmitigr_libs_set_library_info_lib_variables(${lib})

  # -------------
  # Documentation
  # -------------

  if(DMITIGR_LIBS_DOXYGEN)
    configure_file(Doxyfile.in
      ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/${lib}/Doxyfile @ONLY
      NEWLINE_STYLE UNIX)

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doc/${lib}/doxyfile.specific)
      file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/doc/${lib}/doxyfile.specific
        DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/${lib})
    else()
      file(TOUCH ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/${lib}/doxyfile.specific)
    endif()
  endif()

  # -------------------------------------
  # Conditional preprocessing and sources
  # -------------------------------------

  # Length of 0 means that the library is header-only anyway.
  list(LENGTH ${dmlib}_implementations ${dmlib}_implementations_length)
  if(NOT ${${dmlib}_implementations_length} EQUAL 0)
    # Some preprocessed headers (like dll.hpp) must be included for every
    # library with implementation files.
    foreach(file dll.hpp)
      configure_file(${lib_root}/${file}.in
        ${CMAKE_CURRENT_SOURCE_DIR}/${lib_root}/${lib}/${file} @ONLY
        NEWLINE_STYLE UNIX)
      list(APPEND ${dmlib}_headers ${file})
    endforeach()

    set(${dmlib}_header_only ${DMITIGR_LIBS_HEADER_ONLY})
  else()
    set(${dmlib}_header_only TRUE)
  endif()

  if(NOT ${dmlib}_header_only)
    if(BUILD_SHARED_LIBS)
      set(export_file_suffix "shared")
    else()
      set(export_file_suffix "static")
    endif()
  else() # header-only
    set(export_file_suffix "interface")
  endif()

  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${lib_root}/${lib}/${lib}.hpp)
    # Headers
    list(APPEND ${dmlib}_headers ${lib}.hpp)
  endif()

  if(NOT ${${dmlib}_header_only})
    # Transunits
    configure_file(${lib_root}/lib.cpp.in
      ${CMAKE_CURRENT_SOURCE_DIR}/${lib_root}/${lib}/${lib}.cpp @ONLY
      NEWLINE_STYLE UNIX)
    list(APPEND ${dmlib}_transunits "${lib}.cpp")
  endif()

  # Both major and minor versions of 0 means that the library handles
  # versioning by itself, so no version.* files needs to be configured.
  if(DEFINED ${dmlib}_version_major)
    if((${${dmlib}_version_major} GREATER 0) OR (${${dmlib}_version_minor} GREATER 0))
      # Preprocessing
      configure_file(${lib_root}/version.hpp.in
        ${CMAKE_CURRENT_SOURCE_DIR}/${lib_root}/${lib}/version.hpp @ONLY
        NEWLINE_STYLE UNIX)
      list(APPEND ${dmlib}_headers version.hpp)

      if(NOT ${${dmlib}_header_only})
        configure_file(${lib_root}/lib_version.cpp.in
          ${CMAKE_CURRENT_SOURCE_DIR}/${lib_root}/${lib}/lib_version.cpp @ONLY
          NEWLINE_STYLE UNIX)
        list(APPEND ${dmlib}_implementations lib_version.cpp)
        configure_file(${lib_root}/lib_version.hpp.in
          ${CMAKE_CURRENT_SOURCE_DIR}/${lib_root}/${lib}/lib_version.hpp @ONLY
          NEWLINE_STYLE UNIX)
        list(APPEND ${dmlib}_headers lib_version.hpp)

        if(WIN32)
          configure_file(${lib_root}/version.rc.in
            ${CMAKE_CURRENT_SOURCE_DIR}/${lib_root}/${lib}/version.rc @ONLY
            NEWLINE_STYLE UNIX)
          list(APPEND ${dmlib}_build_only_sources version.rc)
        endif()
      endif()
    endif()
  else()
    message(FATAL_ERROR "Variable ${dmlib}_version_major is not defined")
  endif()

  foreach(st ${dmitigr_libs_source_types})
    list(TRANSFORM ${dmlib}_${st} PREPEND "${lib_root}/${lib}/")
    list(APPEND ${dmlib}_sources ${${dmlib}_${st}})
  endforeach()

  set_source_files_properties(
    ${${dmlib}_implementations}
    ${${dmlib}_cmake_sources}
    ${${dmlib}_cmake_unpreprocessed}

    PROPERTIES
    HEADER_FILE_ONLY ON)

  # ------------------------------------
  # Targets
  # ------------------------------------

  if(NOT ${${dmlib}_header_only})
    add_library(${dmlib} ${${dmlib}_sources})
  else()
    add_library(${dmlib} INTERFACE)
  endif()
  add_library(dmitigr::${lib} ALIAS ${dmlib})

  if(NOT ${${dmlib}_header_only})
    set(output_name "${dmlib}")
    target_compile_definitions(${dmlib} PUBLIC ${DMLIB}_NOT_HEADER_ONLY)
    if(BUILD_SHARED_LIBS)
      target_compile_definitions(${dmlib}
        PRIVATE ${DMLIB}_DLL_BUILDING
        PUBLIC  ${DMLIB}_DLL)
    elseif(WIN32)
      set(output_name "${dmlib}_static")
    endif()

    set_target_properties(${dmlib}
      PROPERTIES
      OUTPUT_NAME "${output_name}"
      LINKER_LANGUAGE "CXX"
      POSITION_INDEPENDENT_CODE True
      VERSION ${${dmlib}_version_major}.${${dmlib}_version_minor}
      # DEBUG_POSTFIX "d"
      )

    dmitigr_libs_target_compile_options(${dmlib})
  endif()

  if(NOT ${${dmlib}_header_only})
    target_compile_definitions(${dmlib}
      PRIVATE ${${dmlib}_target_compile_definitions_private}
      PUBLIC  ${${dmlib}_target_compile_definitions_public})

    target_include_directories(${dmlib}
      PRIVATE ${${dmlib}_target_include_directories_private}
      PUBLIC  ${${dmlib}_target_include_directories_public})
  else() # header-only
    target_compile_definitions(${dmlib}
      INTERFACE ${${dmlib}_target_compile_definitions_interface})

    target_include_directories(${dmlib}
      INTERFACE ${${dmlib}_target_include_directories_interface})
  endif()

  # ------------------------------------
  # Dependencies
  # ------------------------------------

  if(NOT ${${dmlib}_header_only})
    foreach(dep ${dmitigr_libs_${lib}_deps})
      target_link_libraries(${dmlib} PUBLIC dmitigr_${dep})
    endforeach()

    # Link with manually specified dependencies in ${lib}/CMakeLists.txt (if any)
    target_link_libraries(${dmlib}
      PRIVATE ${${dmlib}_target_link_libraries_private}
      PUBLIC  ${${dmlib}_target_link_libraries_public})
  else() # header-only
    foreach(dep ${dmitigr_libs_${lib}_deps})
      target_link_libraries(${dmlib} INTERFACE dmitigr_${dep})
    endforeach()

    # Link with manually specified dependencies in ${lib}/CMakeLists.txt (if any)
    target_link_libraries(${dmlib}
      INTERFACE ${${dmlib}_target_link_libraries_interface})
  endif()

  # ------------------------------------
  # Installing
  # ------------------------------------

  if(DMITIGR_LIBS_INSTALL)
    dmitigr_install_lib_includes(${dmlib}_headers)
    if(${${dmlib}_header_only})
      dmitigr_install_lib_includes(${dmlib}_implementations)
    endif()

    if(NOT ${${dmlib}_header_only})
      install(TARGETS ${dmlib}
        EXPORT ${dmlib}_export
        ARCHIVE  DESTINATION ${DMITIGR_LIBS_LIB_INSTALL_DIR}
        LIBRARY  DESTINATION ${DMITIGR_LIBS_LIB_INSTALL_DIR}
        RUNTIME  DESTINATION ${DMITIGR_LIBS_LIB_INSTALL_DIR})
    else()
      install(TARGETS ${dmlib}
        EXPORT ${dmlib}_export
        INCLUDES DESTINATION ${DMITIGR_LIBS_INCLUDE_INSTALL_DIR})
    endif()

    # Don't use NAMESPACE, since ${dmlib} contains "dmitigr_" prefix already
    # and it seems there is no way to omit it in order to use NAMESPACE...
    install(EXPORT ${dmlib}_export
      # NAMESPACE dmitigr::
      DESTINATION ${DMITIGR_LIBS_CMAKE_INSTALL_DIR}
      FILE ${dmlib}_${export_file_suffix}-config.cmake)
  endif()

  # ------------------------------------
  # Uninstalling
  # ------------------------------------

  add_custom_command(TARGET dmitigr_libs_uninstall PRE_BUILD
    COMMAND cmake -E rm -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_LIBS_CMAKE_INSTALL_DIR}/${dmlib}*
    COMMAND cmake -E rm -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_LIBS_LIB_INSTALL_DIR}/${dmlib}*
    COMMAND cmake -E rm -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_LIBS_LIB_INSTALL_DIR}/lib${dmlib}*
    COMMAND cmake -E rm -rf ${CMAKE_INSTALL_PREFIX}/${DMITIGR_LIBS_INCLUDE_INSTALL_DIR}/dmitigr/${lib}*
    COMMAND cmake -E rm -rf ${CMAKE_INSTALL_PREFIX}/${DMITIGR_LIBS_INCLUDE_INSTALL_DIR}/dmitigr/${lib})

  dmitigr_libs_get_deps(res ${lib})
  foreach(dep ${res})
    string(FIND "${dep}" "3rdparty_" pos)
    if (pos EQUAL 0)
      string(SUBSTRING "${dep}" 11 -1 dep)
      add_custom_command(TARGET dmitigr_libs_uninstall PRE_BUILD
        COMMAND cmake -E rm -rf ${CMAKE_INSTALL_PREFIX}/${DMITIGR_LIBS_INCLUDE_INSTALL_DIR}/dmitigr/3rdparty/*${dep}*)
    endif()
  endforeach()
endforeach()

# ------------------------------------------------------------------------------
# Tests
# ------------------------------------------------------------------------------

if(DMITIGR_LIBS_TESTS)
  enable_testing()

  function(dmitigr_configure_test lib test)
    set(dmlib dmitigr_${lib})
    set(full_name "${lib}-${test}")
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/${lib}/${full_name}.cpp")
      set(is_unit_test TRUE)
      set(full_name "${lib}-unit-${test}")
    else()
      set(is_unit_test FALSE)
    endif()
    set(exe "dmitigr_${full_name}")
    set(src "${CMAKE_CURRENT_SOURCE_DIR}/test/${lib}/${full_name}.cpp")
    add_executable(${exe} ${src})
    target_link_libraries(${exe} PRIVATE ${dmlib}
      ${${dmlib}_tests_target_link_libraries}
      ${${dmlib}_test_${test}_target_link_libraries})
    target_compile_definitions(${exe} PRIVATE
      ${${dmlib}_tests_target_compile_definitions}
      ${${dmlib}_test_${test}_target_compile_definitions})
    target_compile_options(${exe} PRIVATE
      ${${dmlib}_tests_target_compile_options}
      ${${dmlib}_test_${test}_target_compile_options})
    dmitigr_libs_target_compile_options(${exe})
    if(is_unit_test)
      add_test(NAME ${exe} COMMAND ${exe})
    endif()
  endfunction()

  # Get the actual list of libraries with tests and configure these tests
  foreach(lib ${dmitigr_libs_package})
    set(dmlib dmitigr_${lib})
    if(${dmlib}_tests)
      list(APPEND libraries_with_tests ${lib})
      foreach(test ${${dmlib}_tests})
        dmitigr_configure_test(${lib} ${test})
      endforeach()
    endif()
  endforeach()

  # Set dependency lists of library tests
  macro(dmitigr_append_test_deps deps)
    foreach(dep ${${deps}})
      string(FIND "${dep}" "dmitigr_" pos)
      if(pos EQUAL 0)
        string(REGEX REPLACE "^dmitigr_(.*)" "\\1" dep "${dep}")
      endif()
      if(NOT "${dep}" IN_LIST ${lib}_test_deps)
        list(APPEND ${lib}_test_deps ${dep})
      endif()
    endforeach()
  endmacro()
  foreach(lib ${libraries_with_tests})
    set(dmlib dmitigr_${lib})
    dmitigr_append_test_deps(${dmlib}_tests_target_link_libraries)
    foreach(test ${${dmlib}_tests})
      dmitigr_append_test_deps(${dmlib}_test_${test}_target_link_libraries)
    endforeach()
  endforeach()

  # Dump list of libraries with tests
  set(content "#!/bin/bash\n# Copyright (C) Dmitry Igrishin\n\n")
  set(content "${content}# This file is automatically generated by CMake!\n\n")
  set(content "${content}igrilibs_with_tests=\"")
  foreach(lib ${libraries_with_tests})
    set(content "${content}${lib} ")
  endforeach()
  string(STRIP "${content}" content)
  set(content "${content}\"\n")

  # Dump dependency lists of library tests
  foreach(lib ${libraries_with_tests})
    set(content "${content}${lib}_test_deps=\"")
    foreach(dep ${${lib}_test_deps})
      set(content "${content}${dep} ")
    endforeach()
    string(STRIP "${content}" content)
    set(content "${content}\"\n")
  endforeach()

  # Write content
  if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tool)
    file(WRITE tool/igrilibs_test_deps.sh "${content}")
  endif()
  unset(content)
endif()
