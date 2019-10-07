# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

cmake_minimum_required(VERSION 3.10)
cmake_policy(VERSION 3.10)
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

option(BUILD_SHARED_LIBS "Build shared libraries?" ON)
option(DMITIGR_CEFEIKA_HEADER_ONLY "Header-only?" OFF)
option(DMITIGR_CEFEIKA_BUILD_TESTS "Build tests?" OFF)
option(DMITIGR_LIBRARIAN_DEBUG "Print librarian.cmake debug output?" OFF)

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
# Languages
# ------------------------------------------------------------------------------

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------
# Include directories
# ------------------------------------------------------------------------------

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/lib)
include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR}/lib)

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
  # Preprocessing
  # ------------------------------------

  dmitigr_set_library_info_lib_variables(${lib})

  foreach(file dll.hpp implementation_footer.hpp
      implementation_header.hpp version.hpp version.rc)
    configure_file(lib/dmitigr/${file}.in
      ${CMAKE_CURRENT_SOURCE_DIR}/lib/dmitigr/${lib}/${file} @ONLY
      NEWLINE_STYLE UNIX)
  endforeach()

  configure_file(doc/Doxyfile.in
    ${CMAKE_CURRENT_SOURCE_DIR}/doc/${lib}/Doxyfile @ONLY
    NEWLINE_STYLE UNIX)

  if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doc/${lib}/doxyfile.specific)
    file(TOUCH ${CMAKE_CURRENT_SOURCE_DIR}/doc/${lib}/doxyfile.specific)
  endif()

  # ------------------------------------
  # Sources
  # ------------------------------------

  list(APPEND dmitigr_${lib}_preprocessed_headers
    dll.hpp implementation_footer.hpp implementation_header.hpp version.hpp)
  if (WIN32)
    list(APPEND dmitigr_${lib}_build_only_sources version.rc)
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

  if(NOT DMITIGR_CEFEIKA_HEADER_ONLY)
    add_library(${lib} ${dmitigr_${lib}_sources})
  else()
    add_library(${lib} INTERFACE)
  endif()
  add_library(dmitigr::${lib} ALIAS ${lib})

  if(NOT DMITIGR_CEFEIKA_HEADER_ONLY)
    set(output_name "dmitigr_${lib}")
    if(BUILD_SHARED_LIBS)
      target_compile_definitions(${lib}
        PRIVATE -DDMITIGR_CEFEIKA_DLL_BUILDING
        INTERFACE -DDMITIGR_${LIB}_DLL)
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

  if(NOT DMITIGR_CEFEIKA_HEADER_ONLY)
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

  if(NOT DMITIGR_CEFEIKA_HEADER_ONLY)
    target_link_libraries(${lib}
      PRIVATE ${dmitigr_${lib}_target_link_libraries_private}
      PUBLIC  ${dmitigr_${lib}_target_link_libraries_public})
  else() # header-only
    target_link_libraries(${lib} INTERFACE ${dmitigr_${lib}_target_link_libraries_interface})
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

  if(NOT DMITIGR_CEFEIKA_HEADER_ONLY)
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

  if(NOT DMITIGR_CEFEIKA_HEADER_ONLY)
    if(BUILD_SHARED_LIBS)
      set(export_file_suffix "shared")
    else()
      set(export_file_suffix "static")
    endif()
  else() # header-only
    set(export_file_suffix "interface")
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
# Testing
# ------------------------------------------------------------------------------

if(DMITIGR_CEFEIKA_BUILD_TESTS)
  enable_testing()
  message("The mode of building Dmitigr Cefeika tests is set.")

  function(dmitigr_configure_test lib test)
    set(full_name "${lib}-${test}")
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/lib/dmitigr/${lib}/test/${full_name}.cpp")
      set(is_unit_test TRUE)
      set(full_name "${lib}-unit-${test}")
    else()
      set(is_unit_test FALSE)
    endif()
    set(exe "${full_name}")
    set(src "${CMAKE_SOURCE_DIR}/lib/dmitigr/${lib}/test/${full_name}.cpp")
    add_executable(${exe} ${src})
    target_link_libraries(${exe} PRIVATE dmitigr::util dmitigr::${lib} ${dmitigr_${lib}_tests_target_link_libraries})
    dmitigr_target_compile_options(${exe})
    if(is_unit_test)
      add_test(NAME ${exe} COMMAND ${exe})
    endif()
  endfunction()

  foreach(lib ${dmitigr_cefeika_libraries})
    foreach(test ${dmitigr_${lib}_tests})
      dmitigr_configure_test(${lib} ${test})
    endforeach()
  endforeach()
endif()
