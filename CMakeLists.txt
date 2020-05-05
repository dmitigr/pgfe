# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

cmake_minimum_required(VERSION 3.17)
cmake_policy(VERSION 3.17)
project(dmitigr_cefeika)

if (NOT (UNIX OR WIN32))
  message(FATAL_ERROR "Dmitigr Cefeika only supports Unix-like or Windows")
endif()

list(APPEND CMAKE_MODULE_PATH ${dmitigr_cefeika_SOURCE_DIR}/cmake)
include(dmitigr)
include(dmitigr_cefeika)

# ------------------------------------------------------------------------------
# Build options
# ------------------------------------------------------------------------------

set(CMAKE_VERBOSE_MAKEFILE FALSE CACHE BOOL
  "Verbose output upon build?")
set(BUILD_SHARED_LIBS TRUE CACHE BOOL
  "Build shared libraries?")
set(DMITIGR_CEFEIKA_HEADER_ONLY FALSE CACHE BOOL
  "Whole header-only?")
set(DMITIGR_CEFEIKA_BUILD_TESTS FALSE CACHE BOOL
  "Build tests?")
set(DMITIGR_CEFEIKA_WITH_OPENSSL False CACHE BOOL
  "Link to OpenSSL where possible?")
set(DMITIGR_CEFEIKA_WITH_ZLIB False CACHE BOOL
  "Link to Zlib where possible?")
set(DMITIGR_LIBRARIAN_DEBUG FALSE CACHE BOOL
  "Print librarian.cmake debug output?")

if(NOT DMITIGR_CEFEIKA_HEADER_ONLY)
  if(BUILD_SHARED_LIBS)
    message("The mode of building shared Dmitigr Cefeika libraries is set.")
  else()
    message("The mode of building static Dmitigr Cefeika libraries is set.")
  endif()

  if(NOT DEFINED CMAKE_BUILD_TYPE OR NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
      "Build type: Debug Release RelWithDebInfo MinSizeRel." FORCE)
  endif()
  message("Dmitigr Cefeika build type it set to ${CMAKE_BUILD_TYPE}")
else()
  message("Dmitigr Cefeika header-only mode is set.")
endif()

# ------------------------------------------------------------------------------
# Installation options
# ------------------------------------------------------------------------------

if(UNIX)
  set(DMITIGR_CEFEIKA_SHARE_INSTALL_DIR "share/dmitigr_cefeika" CACHE
    STRING "Name of the installation directory for the shared stuff relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_CEFEIKA_CMAKE_INSTALL_DIR "${DMITIGR_CEFEIKA_SHARE_INSTALL_DIR}/cmake" CACHE
    STRING "Name of the installation directory for the CMake stuff relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_CEFEIKA_DOC_INSTALL_DIR "${DMITIGR_CEFEIKA_SHARE_INSTALL_DIR}/doc" CACHE
    STRING "Name of the installation directory for the documentation relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_CEFEIKA_LIB_INSTALL_DIR "lib" CACHE
    STRING "Name of the installation directory for the libraries relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_CEFEIKA_INCLUDE_INSTALL_DIR "include" CACHE
    STRING "Name of the installation directory for the includes relative to ${CMAKE_INSTALL_PREFIX}")
elseif(WIN32)
  # On Windows, CMAKE_INSTALL_PREFIX is $ENV{ProgramFiles}\\${CMAKE_PROJECT_NAME} by default. In turn:
  #   - on AMD64: ProgramFiles=%ProgramFiles%
  #   - on   x86: ProgramFiles=%ProgramFiles(x86)%
  # See: https://msdn.microsoft.com/en-us/library/aa384274.aspx
  set(DMITIGR_CEFEIKA_SHARE_INSTALL_DIR "." CACHE
    STRING "Name of the installation directory for the shared stuff relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_CEFEIKA_CMAKE_INSTALL_DIR "cmake" CACHE
    STRING "Name of the installation directory for the CMake stuff relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_CEFEIKA_DOC_INSTALL_DIR "doc" CACHE
    STRING "Name of the installation directory for the documentation relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_CEFEIKA_LIB_INSTALL_DIR "lib" CACHE
    STRING "Name of the installation directory for the libraries relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_CEFEIKA_INCLUDE_INSTALL_DIR "include" CACHE
    STRING "Name of the installation directory for the includes relative to ${CMAKE_INSTALL_PREFIX}")
endif()

# ------------------------------------------------------------------------------
# Global variables (constants)
# ------------------------------------------------------------------------------

if(NOT DMITIGR_CEFEIKA_HEADER_ONLY)
  if(BUILD_SHARED_LIBS)
    set(export_file_suffix "shared")
  else()
    set(export_file_suffix "static")
  endif()
else() # header-only
  set(export_file_suffix "interface")
endif()

# ------------------------------------------------------------------------------
# Languages
# ------------------------------------------------------------------------------

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------
# Include directories
# ------------------------------------------------------------------------------

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}/lib
  ${CMAKE_CURRENT_BINARY_DIR}/lib
  )

# ------------------------------------------------------------------------------
# Third-party dependencies
# ------------------------------------------------------------------------------

if ("rajson" IN_LIST dmitigr_cefeika_libraries)
  add_subdirectory(thirdparty/rapidjson)
endif()

if ("ws" IN_LIST dmitigr_cefeika_libraries)
  add_subdirectory(thirdparty/uv)
  add_subdirectory(thirdparty/usockets)
  add_subdirectory(thirdparty/uwebsockets)
endif()

# ------------------------------------------------------------------------------
# Output settings
# ------------------------------------------------------------------------------

# Note: Multi-configuration generators (VS, Xcode) appends a per-configuration
# subdirectory to CMAKE_RUNTIME_OUTPUT_DIRECTORY unless a generator expression
# is used.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

if (WIN32)
  set(dmitigr_cefeika_resource_destination_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>")
elseif (UNIX)
  set(dmitigr_cefeika_resource_destination_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

# ------------------------------------------------------------------------------
# Custom targets
# ------------------------------------------------------------------------------

add_custom_target(dmitigr_cefeika_uninstall)

add_custom_target(dmitigr_cefeika_create_resource_destination_dir ALL
  COMMAND cmake -E make_directory "${dmitigr_cefeika_resource_destination_dir}"
  )

# --------------------------------------
# Installing
# --------------------------------------

install(FILES LICENSE.txt
  DESTINATION ${DMITIGR_CEFEIKA_SHARE_INSTALL_DIR})

install(FILES
  cmake/dmitigr_cefeika.cmake
  cmake/dmitigr_cefeika-config.cmake
  DESTINATION ${DMITIGR_CEFEIKA_CMAKE_INSTALL_DIR})

# ------------------------------------------------------------------------------
# Libraries
# ------------------------------------------------------------------------------

foreach(lib ${dmitigr_cefeika_libraries})
  string(TOUPPER "${lib}" LIB)

  add_subdirectory(lib/dmitigr/${lib})

  # ------------------------------------
  # Unconditional preprocessing
  # ------------------------------------

  dmitigr_set_library_info_lib_variables(${lib})

  configure_file(doc/Doxyfile.in
    ${CMAKE_CURRENT_SOURCE_DIR}/doc/${lib}/Doxyfile @ONLY
    NEWLINE_STYLE UNIX)

  if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doc/${lib}/doxyfile.specific)
    file(TOUCH ${CMAKE_CURRENT_SOURCE_DIR}/doc/${lib}/doxyfile.specific)
  endif()

  # -------------------------------------
  # Conditional preprocessing and sources
  # -------------------------------------

  # Length of 0 means that the library is header-only anyway.
  list(LENGTH dmitigr_${lib}_implementations dmitigr_${lib}_implementations_length)
  if(${dmitigr_${lib}_implementations_length} EQUAL 0)
    set(dmitigr_${lib}_header_only TRUE)
  else()
    set(dmitigr_${lib}_header_only ${DMITIGR_CEFEIKA_HEADER_ONLY})
  endif()

  # Root headers
  configure_file(lib/dmitigr/lib.hpp.in
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/dmitigr/${lib}.hpp @ONLY
    NEWLINE_STYLE UNIX)
  list(APPEND dmitigr_${lib}_root_headers "../${lib}.hpp")

  # Headers
  list(APPEND dmitigr_${lib}_headers ${lib}.hpp)

  if(NOT ${dmitigr_${lib}_header_only})
    # Preprocessed headers
    foreach(file dll.hpp)
      configure_file(lib/dmitigr/${file}.in
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dmitigr/${lib}/${file} @ONLY
        NEWLINE_STYLE UNIX)
      list(APPEND dmitigr_${lib}_preprocessed_headers ${file})
    endforeach()

    # Transunits
    configure_file(lib/dmitigr/lib.cpp.in
      ${CMAKE_CURRENT_SOURCE_DIR}/lib/dmitigr/${lib}.cpp @ONLY
      NEWLINE_STYLE UNIX)
    list(APPEND dmitigr_${lib}_transunits "../${lib}.cpp")
  endif()

  # Both major and minor versions of 0 means that the library handles
  # versioning by itself, so no version.* files needs to be configured.
  if((${dmitigr_${lib}_version_major} GREATER 0) OR (${dmitigr_${lib}_version_minor} GREATER 0))
    # Preprocessing
    configure_file(lib/dmitigr/version.hpp.in
      ${CMAKE_CURRENT_SOURCE_DIR}/lib/dmitigr/${lib}/version.hpp @ONLY
      NEWLINE_STYLE UNIX)
    list(APPEND dmitigr_${lib}_preprocessed_headers version.hpp)

    if(WIN32 AND NOT ${dmitigr_${lib}_header_only})
      configure_file(lib/dmitigr/version.rc.in
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dmitigr/${lib}/version.rc @ONLY
        NEWLINE_STYLE UNIX)
      list(APPEND dmitigr_${lib}_build_only_sources version.rc)
    endif()
  endif()

  foreach(st ${dmitigr_source_types})
    list(TRANSFORM dmitigr_${lib}_${st} PREPEND "lib/dmitigr/${lib}/")
    list(APPEND dmitigr_${lib}_sources ${dmitigr_${lib}_${st}})
  endforeach()

  set_source_files_properties(
    ${dmitigr_${lib}_implementations}
    ${dmitigr_${lib}_cmake_sources}
    ${dmitigr_${lib}_cmake_unpreprocessed}

    PROPERTIES
    HEADER_FILE_ONLY ON)

  # ------------------------------------
  # Targets
  # ------------------------------------

  if(NOT ${dmitigr_${lib}_header_only})
    add_library(${lib} ${dmitigr_${lib}_sources})
  else()
    add_library(${lib} INTERFACE)
  endif()
  add_library(dmitigr::${lib} ALIAS ${lib})

  if(NOT ${dmitigr_${lib}_header_only})
    set(output_name "dmitigr_${lib}")
    if(BUILD_SHARED_LIBS)
      target_compile_definitions(${lib}
        PRIVATE -DDMITIGR_CEFEIKA_DLL_BUILDING
        PUBLIC -DDMITIGR_${LIB}_DLL)
    elseif(WIN32)
      set(output_name "dmitigr_${lib}_static")
    endif()

    set_target_properties(${lib}
      PROPERTIES
      OUTPUT_NAME "${output_name}"
      LINKER_LANGUAGE "CXX"
      POSITION_INDEPENDENT_CODE True
      VERSION ${dmitigr_${lib}_version_major}.${dmitigr_${lib}_version_minor}
      DEBUG_POSTFIX "d")

    dmitigr_target_compile_options(${lib})
  else() # header-only
    target_compile_definitions(${lib} INTERFACE -DDMITIGR_${LIB}_HEADER_ONLY)
  endif()

  if(NOT ${dmitigr_${lib}_header_only})
    target_compile_definitions(${lib}
      PRIVATE ${dmitigr_${lib}_target_compile_definitions_private}
      PUBLIC  ${dmitigr_${lib}_target_compile_definitions_public})

    target_include_directories(${lib}
      PRIVATE ${dmitigr_${lib}_target_include_directories_private}
      PUBLIC  ${dmitigr_${lib}_target_include_directories_public})
  else() # header-only
    target_compile_definitions(${lib}
      INTERFACE ${dmitigr_${lib}_target_compile_definitions_interface})

    target_include_directories(${lib}
      INTERFACE ${dmitigr_${lib}_target_include_directories_interface})
  endif()

  # ------------------------------------
  # Dependencies
  # ------------------------------------

  if(NOT ${dmitigr_${lib}_header_only})
    foreach(dep ${dmitigr_cefeika_${lib}_deps})
      target_link_libraries(${lib} PUBLIC ${dep})
    endforeach()

    # Link with manually specified dependencies in ${lib}/CMakeLists.txt (if any)
    target_link_libraries(${lib}
      PRIVATE ${dmitigr_${lib}_target_link_libraries_private}
      PUBLIC  ${dmitigr_${lib}_target_link_libraries_public})
  else() # header-only
    foreach(dep ${dmitigr_cefeika_${lib}_deps})
      target_link_libraries(${lib} INTERFACE ${dep})
    endforeach()

    # Link with manually specified dependencies in ${lib}/CMakeLists.txt (if any)
    target_link_libraries(${lib}
      INTERFACE ${dmitigr_${lib}_target_link_libraries_interface})
  endif()

  # ------------------------------------
  # Installing
  # ------------------------------------

  install(FILES ${dmitigr_${lib}_root_headers}
    DESTINATION ${DMITIGR_CEFEIKA_INCLUDE_INSTALL_DIR}/dmitigr)

  install(FILES ${dmitigr_${lib}_preprocessed_headers}
    DESTINATION ${DMITIGR_CEFEIKA_INCLUDE_INSTALL_DIR}/dmitigr/${lib})

  install(FILES ${dmitigr_${lib}_headers}
    DESTINATION ${DMITIGR_CEFEIKA_INCLUDE_INSTALL_DIR}/dmitigr/${lib})

  if(NOT ${dmitigr_${lib}_header_only})
    install(TARGETS ${lib}
      EXPORT dmitigr_${lib}_export
      ARCHIVE  DESTINATION ${DMITIGR_CEFEIKA_LIB_INSTALL_DIR}
      LIBRARY  DESTINATION ${DMITIGR_CEFEIKA_LIB_INSTALL_DIR}
      RUNTIME  DESTINATION ${DMITIGR_CEFEIKA_LIB_INSTALL_DIR}
      INCLUDES DESTINATION ${DMITIGR_CEFEIKA_INCLUDE_INSTALL_DIR})
  else()
    install(TARGETS ${lib}
      EXPORT dmitigr_${lib}_export
      INCLUDES DESTINATION ${DMITIGR_CEFEIKA_INCLUDE_INSTALL_DIR})

    install(FILES ${dmitigr_${lib}_implementations}
      DESTINATION ${DMITIGR_CEFEIKA_INCLUDE_INSTALL_DIR}/dmitigr/${lib})

    install(FILES lib/dmitigr/cefeika_header_only.hpp
      DESTINATION ${DMITIGR_CEFEIKA_INCLUDE_INSTALL_DIR}/dmitigr)
  endif()

  install(EXPORT dmitigr_${lib}_export
    NAMESPACE dmitigr::
    DESTINATION ${DMITIGR_CEFEIKA_CMAKE_INSTALL_DIR}
    FILE dmitigr_${lib}_${export_file_suffix}-config.cmake)

  # ------------------------------------
  # Uninstalling
  # ------------------------------------

  add_custom_command(TARGET dmitigr_cefeika_uninstall PRE_BUILD
    COMMAND cmake -E remove -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_CEFEIKA_CMAKE_INSTALL_DIR}/dmitigr_${lib}*
    COMMAND cmake -E remove -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_CEFEIKA_DOC_INSTALL_DIR}/dmitigr_${lib}*
    COMMAND cmake -E remove -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_CEFEIKA_LIB_INSTALL_DIR}/dmitigr_${lib}*
    COMMAND cmake -E remove -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_CEFEIKA_LIB_INSTALL_DIR}/libdmitigr_${lib}*
    COMMAND cmake -E remove -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_CEFEIKA_INCLUDE_INSTALL_DIR}/dmitigr/${lib}*
    COMMAND cmake -E remove_directory ${CMAKE_INSTALL_PREFIX}/${DMITIGR_CEFEIKA_INCLUDE_INSTALL_DIR}/dmitigr/${lib})
endforeach()

# ------------------------------------------------------------------------------
# Tests
# ------------------------------------------------------------------------------

if(DMITIGR_CEFEIKA_BUILD_TESTS)
  enable_testing()
  message("The mode of building Dmitigr Cefeika tests is set.")

  function(dmitigr_configure_test lib test)
    set(full_name "${lib}-${test}")
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/${lib}/${full_name}.cpp")
      set(is_unit_test TRUE)
      set(full_name "${lib}-unit-${test}")
    else()
      set(is_unit_test FALSE)
    endif()
    set(exe "${full_name}")
    set(src "${CMAKE_CURRENT_SOURCE_DIR}/test/${lib}/${full_name}.cpp")
    add_executable(${exe} ${src})
    target_link_libraries(${exe} PRIVATE dmitigr::testo dmitigr::${lib} ${dmitigr_${lib}_tests_target_link_libraries})
    dmitigr_target_compile_options(${exe})
    if(is_unit_test)
      add_test(NAME ${exe} COMMAND ${exe})
    endif()
  endfunction()

  foreach(lib ${dmitigr_cefeika_libraries})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/${lib}")
      add_subdirectory(test/${lib})
      foreach(test ${dmitigr_${lib}_tests})
        dmitigr_configure_test(${lib} ${test})
      endforeach()
    endif()
  endforeach()
endif()
