# -*- cmake -*-
# Copyright (C) 2022 Dmitry Igrishin
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#
# Dmitry Igrishin
# dmitigr@gmail.com

cmake_minimum_required(VERSION 3.16)
cmake_policy(VERSION 3.16)
project(dmitigr_cpplipa)

if (NOT (UNIX OR WIN32))
  message(FATAL_ERROR "Unsupported platform")
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(dmitigr)
include(dmitigr_cpplipa)

# ------------------------------------------------------------------------------
# Build options
# ------------------------------------------------------------------------------

set(DMITIGR_LIBRARIAN_DEBUG Off CACHE BOOL
  "Print librarian.cmake debug output?")
set(DMITIGR_CPPLIPA_CLANG_USE_LIBCPP On CACHE BOOL
  "Use libc++ with Clang?")
set(DMITIGR_CPPLIPA_HEADER_ONLY Off CACHE BOOL
  "Whole header-only?")
set(DMITIGR_CPPLIPA_DOXYGEN Off CACHE BOOL
  "Build configurations for Doxygen?")
set(DMITIGR_CPPLIPA_TESTS Off CACHE BOOL
  "Build tests?")
set(DMITIGR_CPPLIPA_OPENSSL Off CACHE BOOL
  "Link to OpenSSL where possible?")
set(DMITIGR_CPPLIPA_ZLIB Off CACHE BOOL
  "Link to Zlib where possible?")
set(BUILD_SHARED_LIBS Off CACHE BOOL
  "Build shared libraries?")
set(CMAKE_VERBOSE_MAKEFILE On CACHE BOOL
  "Verbose output upon build?")

# FindLib.cmake modules will use these settings to find Lib.
foreach(lib ev pq uv)
  string(TOUPPER "${lib}" LIB)
  set(LIB${LIB}_PREFIX "" CACHE PATH
    "Installation prefix of the lib${lib} library (where both headers and binary resides)")
  set(LIB${LIB}_LIB_PREFIX "${LIB${LIB}_PREFIX}" CACHE PATH
    "Installation prefix of the lib${lib} binary (where the binary reside)")
  set(LIB${LIB}_INCLUDE_PREFIX "${LIB${LIB}_PREFIX}" CACHE PATH
    "Installation prefix of the lib${lib} includes (where the headers resides)")
endforeach()

# Enable linking with libc++ if requested.
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if (DMITIGR_CPPLIPA_CLANG_USE_LIBCPP)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif()
endif()

# Set default CMAKE_BUILD_TYPE.
if(NOT DEFINED CMAKE_BUILD_TYPE OR NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING
    "Build type: Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif()

if (NOT DMITIGR_CPPLIPA_HEADER_ONLY)
  if(BUILD_SHARED_LIBS)
    message("Shared libraries are requested (${CMAKE_BUILD_TYPE})")
  else()
    message("Static libraries are requested (${CMAKE_BUILD_TYPE})")
  endif()
elseif(BUILD_SHARED_LIBS)
  message(FATAL_ERROR "DMITIGR_CPPLIPA_HEADER_ONLY and BUILD_SHARED_LIBS are mutually exclusive")
else()
  message("Only 3rdparties without header-only option will be built")
endif()

if(DMITIGR_CPPLIPA_DOXYGEN)
  message("Doxygen configurations are requested")
endif()

if(DMITIGR_CPPLIPA_TESTS)
  message("Tests are requested (${CMAKE_BUILD_TYPE})")
endif()

# The libraries sources root directory.
set(lib_root "src")

# ------------------------------------------------------------------------------
# Installation options
# ------------------------------------------------------------------------------

set(DMITIGR_CPPLIPA_INSTALL_SHARE On CACHE BOOL
  "Install shared stuff?")
set(DMITIGR_CPPLIPA_INSTALL_CMAKE On CACHE BOOL
  "Install cmake stuff?")
set(DMITIGR_CPPLIPA_INSTALL_LIBS On CACHE BOOL
  "Install libraries?")
set(DMITIGR_CPPLIPA_INSTALL_INCLUDES On CACHE BOOL
  "Install includes?")

if(UNIX)
  set(DMITIGR_CPPLIPA_SHARE_INSTALL_DIR "share/dmitigr_cpplipa" CACHE
    STRING "Name of the installation directory for the shared stuff relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_CPPLIPA_CMAKE_INSTALL_DIR "${DMITIGR_CPPLIPA_SHARE_INSTALL_DIR}/cmake" CACHE
    STRING "Name of the installation directory for the CMake stuff relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_CPPLIPA_LIB_INSTALL_DIR "lib" CACHE
    STRING "Name of the installation directory for the libraries relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_CPPLIPA_INCLUDE_INSTALL_DIR "include" CACHE
    STRING "Name of the installation directory for the includes relative to ${CMAKE_INSTALL_PREFIX}")
elseif(WIN32)
  # On Windows, CMAKE_INSTALL_PREFIX is $ENV{ProgramFiles}\\${CMAKE_PROJECT_NAME} by default. In turn:
  #   - on AMD64: ProgramFiles=%ProgramFiles%
  #   - on   x86: ProgramFiles=%ProgramFiles(x86)%
  # See: https://msdn.microsoft.com/en-us/library/aa384274.aspx
  set(DMITIGR_CPPLIPA_SHARE_INSTALL_DIR "." CACHE
    STRING "Name of the installation directory for the shared stuff relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_CPPLIPA_CMAKE_INSTALL_DIR "cmake" CACHE
    STRING "Name of the installation directory for the CMake stuff relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_CPPLIPA_LIB_INSTALL_DIR "lib" CACHE
    STRING "Name of the installation directory for the libraries relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_CPPLIPA_INCLUDE_INSTALL_DIR "include" CACHE
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

# Ensure that dmitigr_cpplipa_libraries is empty!
set(dmitigr_cpplipa_libraries)
foreach(lib ${dmitigr_cpplipa_libraries_all})
  set(pref "${CMAKE_CURRENT_SOURCE_DIR}/${lib_root}/${lib}")
  if(EXISTS "${pref}" OR EXISTS "${pref}.hpp")
    list(APPEND dmitigr_cpplipa_libraries ${lib})
  endif()
endforeach()
unset(pref)

configure_file(cmake/dmitigr_cpplipa_libraries.cmake.in
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/dmitigr_cpplipa_libraries.cmake @ONLY
  NEWLINE_STYLE UNIX)

# ------------------------------------------------------------------------------
# Check the dependencies of the actual libraries
# ------------------------------------------------------------------------------

set(content "#!/bin/bash\n# Copyright (C) Dmitry Igrishin\n\n")

# Dump library list
set(content "${content}cpplipa_libraries=\"")
foreach(lib ${dmitigr_cpplipa_libraries})
  set(content "${content}${lib} ")
endforeach()
string(STRIP "${content}" content)
set(content "${content}\"\n")

# Dump dependency lists
foreach(lib ${dmitigr_cpplipa_libraries})
  set(content "${content}${lib}_deps=\"")
  dmitigr_cpplipa_get_deps(lib_deps "${lib}")
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

    # Append the content for cpplipa_deps.sh
    set(content "${content}${dep} ")
  endforeach()
  string(STRIP "${content}" content)
  set(content "${content}\"\n")
endforeach()
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tool)
  file(WRITE tool/cpplipa_deps.sh "${content}")
endif()
unset(content)
message("This library package contains: ${dmitigr_cpplipa_libraries}")

# ------------------------------------------------------------------------------
# Third-party dependencies
# ------------------------------------------------------------------------------

if ("rajson" IN_LIST dmitigr_cpplipa_libraries)
  add_subdirectory(${lib_root}/3rdparty/rapidjson)
endif()

if ("ws" IN_LIST dmitigr_cpplipa_libraries)
  add_subdirectory(${lib_root}/3rdparty/usockets)
  add_subdirectory(${lib_root}/3rdparty/uwebsockets)
endif()

if ("wscl" IN_LIST dmitigr_cpplipa_libraries)
  add_subdirectory(${lib_root}/3rdparty/uwsc)
endif()

# ------------------------------------------------------------------------------
# Output settings
# ------------------------------------------------------------------------------

# Note: Multi-configuration generators (VS, Xcode) appends a per-configuration
# subdirectory to CMAKE_RUNTIME_OUTPUT_DIRECTORY unless a generator expression
# is used.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

if (WIN32)
  set(dmitigr_cpplipa_resource_destination_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>")
elseif (UNIX)
  set(dmitigr_cpplipa_resource_destination_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

# ------------------------------------------------------------------------------
# Custom targets
# ------------------------------------------------------------------------------

add_custom_target(dmitigr_cpplipa_uninstall)

add_custom_target(dmitigr_cpplipa_create_resource_destination_dir ALL
  COMMAND cmake -E make_directory "${dmitigr_cpplipa_resource_destination_dir}"
  )

# --------------------------------------
# Installing
# --------------------------------------

if(DMITIGR_CPPLIPA_INSTALL_SHARE)
  install(FILES LICENSE.txt
    DESTINATION ${DMITIGR_CPPLIPA_SHARE_INSTALL_DIR})
endif()

if(DMITIGR_CPPLIPA_INSTALL_CMAKE)
  install(FILES
    cmake/dmitigr_cpplipa_libraries.cmake
    cmake/dmitigr_cpplipa_libraries_all.cmake
    cmake/dmitigr_cpplipa-config.cmake
    DESTINATION ${DMITIGR_CPPLIPA_CMAKE_INSTALL_DIR})
endif()

# ------------------------------------------------------------------------------
# Libraries
# ------------------------------------------------------------------------------

# @param file_list A file list name, e.g. "dmitigr_base_root_headers".
function(dmitigr_install_lib_includes file_list)
  foreach(file ${${file_list}})
    string(REGEX REPLACE "^${lib_root}/" "" file "${file}")
    get_filename_component(dir ${file} DIRECTORY)
    install(FILES ${lib_root}/${file}
      DESTINATION ${DMITIGR_CPPLIPA_INCLUDE_INSTALL_DIR}/dmitigr/${dir})
  endforeach()
endfunction()

foreach(lib ${dmitigr_cpplipa_libraries})
  string(TOUPPER "${lib}" LIB)

  set(dmlib dmitigr_${lib})
  set(DMLIB DMITIGR_${LIB})
  set(${dmlib}_rel_path_prefix "${lib_root}/${lib}")
  set(${dmlib}_abs_path_prefix "${CMAKE_CURRENT_SOURCE_DIR}/${${dmlib}_rel_path_prefix}")

  add_subdirectory(${${dmlib}_rel_path_prefix})

  dmitigr_cpplipa_set_library_info_lib_variables(${lib})

  # -------------
  # Documentation
  # -------------

  if(DMITIGR_CPPLIPA_DOXYGEN)
    configure_file(Doxyfile.in
      ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/${lib}/Doxyfile @ONLY
      NEWLINE_STYLE UNIX)

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${lib_root}/${lib}/doxyfile.specific)
      file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${lib_root}/${lib}/doxyfile.specific
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
      list(APPEND ${dmlib}_preprocessed_headers ${file})
    endforeach()

    set(${dmlib}_header_only ${DMITIGR_CPPLIPA_HEADER_ONLY})
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
    # Root headers
    configure_file(${lib_root}/lib.hpp.in
      ${CMAKE_CURRENT_SOURCE_DIR}/${lib_root}/${lib}.hpp @ONLY
      NEWLINE_STYLE UNIX)
    list(APPEND ${dmlib}_root_headers "../${lib}.hpp")

    # Headers
    list(APPEND ${dmlib}_headers ${lib}.hpp)
  endif()

  if(NOT ${${dmlib}_header_only})
    # Transunits
    configure_file(${lib_root}/lib.cpp.in
      ${CMAKE_CURRENT_SOURCE_DIR}/${lib_root}/${lib}.cpp @ONLY
      NEWLINE_STYLE UNIX)
    list(APPEND ${dmlib}_transunits "../${lib}.cpp")
  endif()

  # Both major and minor versions of 0 means that the library handles
  # versioning by itself, so no version.* files needs to be configured.
  if(DEFINED ${dmlib}_version_major)
    if((${${dmlib}_version_major} GREATER 0) OR (${${dmlib}_version_minor} GREATER 0))
      # Preprocessing
      configure_file(${lib_root}/version.hpp.in
        ${CMAKE_CURRENT_SOURCE_DIR}/${lib_root}/${lib}/version.hpp @ONLY
        NEWLINE_STYLE UNIX)
      list(APPEND ${dmlib}_preprocessed_headers version.hpp)

      if(WIN32 AND NOT ${${dmlib}_header_only})
        configure_file(${lib_root}/version.rc.in
          ${CMAKE_CURRENT_SOURCE_DIR}/${lib_root}/${lib}/version.rc @ONLY
          NEWLINE_STYLE UNIX)
        list(APPEND ${dmlib}_build_only_sources version.rc)
      endif()
    endif()
  else()
    message(FATAL_ERROR "Variable ${dmlib}_version_major is not defined")
  endif()

  foreach(st ${dmitigr_cpplipa_source_types})
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
      DEBUG_POSTFIX "d")

    dmitigr_cpplipa_target_compile_options(${dmlib})
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
    foreach(dep ${dmitigr_cpplipa_${lib}_deps})
      target_link_libraries(${dmlib} PUBLIC dmitigr_${dep})
    endforeach()

    # Link with manually specified dependencies in ${lib}/CMakeLists.txt (if any)
    target_link_libraries(${dmlib}
      PRIVATE ${${dmlib}_target_link_libraries_private}
      PUBLIC  ${${dmlib}_target_link_libraries_public})
  else() # header-only
    foreach(dep ${dmitigr_cpplipa_${lib}_deps})
      target_link_libraries(${dmlib} INTERFACE dmitigr_${dep})
    endforeach()

    # Link with manually specified dependencies in ${lib}/CMakeLists.txt (if any)
    target_link_libraries(${dmlib}
      INTERFACE ${${dmlib}_target_link_libraries_interface})
  endif()

  # ------------------------------------
  # Installing
  # ------------------------------------

  if(DMITIGR_CPPLIPA_INSTALL_INCLUDES)
    dmitigr_install_lib_includes(${dmlib}_root_headers)
    dmitigr_install_lib_includes(${dmlib}_preprocessed_headers)
    dmitigr_install_lib_includes(${dmlib}_headers)
    if(${${dmlib}_header_only})
      dmitigr_install_lib_includes(${dmlib}_implementations)
    endif()
  endif()

  if(DMITIGR_CPPLIPA_INSTALL_LIBS)
    if(NOT ${${dmlib}_header_only})
      install(TARGETS ${dmlib}
        EXPORT ${dmlib}_export
        ARCHIVE  DESTINATION ${DMITIGR_CPPLIPA_LIB_INSTALL_DIR}
        LIBRARY  DESTINATION ${DMITIGR_CPPLIPA_LIB_INSTALL_DIR}
        RUNTIME  DESTINATION ${DMITIGR_CPPLIPA_LIB_INSTALL_DIR}
        INCLUDES DESTINATION ${DMITIGR_CPPLIPA_INCLUDE_INSTALL_DIR})
    else()
      install(TARGETS ${dmlib}
        EXPORT ${dmlib}_export
        INCLUDES DESTINATION ${DMITIGR_CPPLIPA_INCLUDE_INSTALL_DIR})
    endif()
  endif()

  if(DMITIGR_CPPLIPA_INSTALL_CMAKE)
    # Don't use NAMESPACE, since ${dmlib} contains "dmitigr_" prefix already
    # and it seems there is no way to omit it in order to use NAMESPACE...
    install(EXPORT ${dmlib}_export
      # NAMESPACE dmitigr::
      DESTINATION ${DMITIGR_CPPLIPA_CMAKE_INSTALL_DIR}
      FILE ${dmlib}_${export_file_suffix}-config.cmake)
  endif()

  # ------------------------------------
  # Uninstalling
  # ------------------------------------

  add_custom_command(TARGET dmitigr_cpplipa_uninstall PRE_BUILD
    COMMAND cmake -E rm -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_CPPLIPA_CMAKE_INSTALL_DIR}/${dmlib}*
    COMMAND cmake -E rm -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_CPPLIPA_LIB_INSTALL_DIR}/${dmlib}*
    COMMAND cmake -E rm -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_CPPLIPA_LIB_INSTALL_DIR}/lib${dmlib}*
    COMMAND cmake -E rm -rf ${CMAKE_INSTALL_PREFIX}/${DMITIGR_CPPLIPA_INCLUDE_INSTALL_DIR}/dmitigr/${lib}*
    COMMAND cmake -E rm -rf ${CMAKE_INSTALL_PREFIX}/${DMITIGR_CPPLIPA_INCLUDE_INSTALL_DIR}/dmitigr/${lib})

  dmitigr_cpplipa_get_deps(res ${lib})
  foreach(dep ${res})
    string(FIND "${dep}" "3rdparty_" pos)
    if (pos EQUAL 0)
      string(SUBSTRING "${dep}" 11 -1 dep)
      add_custom_command(TARGET dmitigr_cpplipa_uninstall PRE_BUILD
        COMMAND cmake -E rm -rf ${CMAKE_INSTALL_PREFIX}/${DMITIGR_CPPLIPA_INCLUDE_INSTALL_DIR}/dmitigr/3rdparty/*${dep}*)
    endif()
  endforeach()
endforeach()

# ------------------------------------------------------------------------------
# Tests
# ------------------------------------------------------------------------------

if(DMITIGR_CPPLIPA_TESTS)
  enable_testing()

  function(dmitigr_configure_test lib test)
    set(dmlib dmitigr_${lib})
    set(full_name "${lib}-${test}")
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${lib_root}/${lib}/test/${full_name}.cpp")
      set(is_unit_test TRUE)
      set(full_name "${lib}-unit-${test}")
    else()
      set(is_unit_test FALSE)
    endif()
    set(exe "dmitigr_${full_name}")
    set(src "${CMAKE_CURRENT_SOURCE_DIR}/${lib_root}/${lib}/test/${full_name}.cpp")
    add_executable(${exe} ${src})
    target_link_libraries(${exe} PRIVATE ${dmlib}
      ${${dmlib}_tests_target_link_libraries}
      ${${dmlib}_test_${test}_target_link_libraries})
    dmitigr_cpplipa_target_compile_options(${exe})
    if(is_unit_test)
      add_test(NAME ${exe} COMMAND ${exe})
    endif()
  endfunction()

  # Get the actual list of libraries with tests and configure these tests
  foreach(lib ${dmitigr_cpplipa_libraries})
    set(dmlib dmitigr_${lib})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${lib_root}/${lib}/test")
      list(APPEND libraries_with_tests ${lib})
      add_subdirectory(${lib_root}/${lib}/test)
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
  set(content "${content}cpplipa_libraries_with_tests=\"")
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
    file(WRITE tool/cpplipa_test_deps.sh "${content}")
  endif()
  unset(content)
endif()
