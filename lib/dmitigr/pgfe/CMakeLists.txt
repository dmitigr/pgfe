# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(dmitigr_pgfe_sources
  array_conversions.hpp
  array_conversions.hxx
  array_conversions.tcc
  basic_conversions.hpp

  basics.cmake
  basics.cpp
  basics.hpp.in

  completion.hpp
  completion.hxx

  compositional.hpp
  compositional.hxx

  composite.cpp
  composite.hpp
  composite.hxx

  connection.cpp
  connection.hpp
  connection.hxx

  connection_options.cpp
  connection_options.cxx.in
  connection_options.hpp
  connection_options.hxx

  conversions_api.hpp
  conversions.hpp
  conversions.hxx

  data.cpp
  data.hpp
  data.hxx

  errc.cmake
  errc.cxx
  errc.hpp.in
  errc.hxx

  error.hpp
  error.hxx

  exceptions.hpp
  exceptions.hxx

  message.hpp

  misc.cpp
  misc.hpp

  notice.hpp
  notice.hxx

  notification.hpp
  notification.hxx

  parameterizable.hpp
  parameterizable.hxx

  prepared_statement.cxx
  prepared_statement.hpp
  prepared_statement.hxx

  problem.cpp
  problem.hpp
  problem.hxx

  response.hpp

  row.hpp
  row.hxx

  row_info.hpp
  row_info.hxx

  server_message.hpp

  signal.hpp

  sql_string.cpp
  sql_string.cxx
  sql_string.hpp
  sql_string.hxx

  sql_vector.cpp
  sql_vector.hpp
  sql_vector.hxx

  std_system_error.cpp
  std_system_error.hpp

  types_fwd.hpp

  net.cxx
  net.hxx

  pq.hxx

  sql.cxx
  sql.hxx

  internal/dll.hpp.in
  )

if (WIN32)
  list(APPEND dmitigr_pgfe_sources version.rc)
endif()

set_source_files_properties(
  basics.cmake
  basics.hpp.in
  connection_options.cxx.in
  errc.cmake
  errc.hpp.in

  internal/dll.hpp.in

  PROPERTIES
  HEADER_FILE_ONLY ON)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

set(CMAKE_MODULE_PATH ${dmitigr_pgfe_SOURCE_DIR}/cmake)

#
# dmitigr_internal
#

find_package(dmitigr_internal REQUIRED)

#
# libpq
#

# FindPq.cmake module will use these settings to find libpq.
set(LIBPQ_PREFIX "" CACHE PATH
  "Installation prefix of the libpq library (where headers and binary resides)")
set(LIBPQ_LIB_PREFIX "${LIBPQ_PREFIX}" CACHE PATH
  "Installation prefix of the libpq binary (where the binary reside)")
set(LIBPQ_INCLUDE_PREFIX "${LIBPQ_PREFIX}" CACHE PATH
  "Installation prefix of the libpq includes (where the headers resides)")

find_package(Pq REQUIRED)
message("libpq includes to use: ${Pq_INCLUDE_DIRS}")
message("Suggested libpq binary to use: ${Suggested_Pq_LIBRARIES}")
include_directories(${Pq_INCLUDE_DIRS})

# ------------------------------------------------------------------------------
# Target tuning
# ------------------------------------------------------------------------------

# Note: Multi-configuration generators (VS, Xcode) appends a per-configuration
# subdirectory to CMAKE_RUNTIME_OUTPUT_DIRECTORY unless a generator expression
# is used.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

add_library(dmitigr_pgfe ${dmitigr_pgfe_sources})
target_link_libraries(dmitigr_pgfe dmitigr_internal ${Suggested_Pq_LIBRARIES})
if (UNIX)
  target_link_libraries(dmitigr_pgfe stdc++fs)
elseif (WIN32)
  target_link_libraries(dmitigr_pgfe Ws2_32.lib)
endif()

set_target_properties(dmitigr_pgfe
  PROPERTIES
  LINKER_LANGUAGE "CXX"
  VERSION ${PGFE_VERSION_PART1}.${PGFE_VERSION_PART2}
  DEBUG_POSTFIX "d")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(dmitigr_pgfe PRIVATE
      -pedantic
      -Wall
      -Wextra
      -Winline
      -Winit-self
      -Wuninitialized
      -Wmaybe-uninitialized
      -Woverloaded-virtual
      -Wsuggest-override
      -Wlogical-op
      -Wswitch)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(dmitigr_pgfe PRIVATE
      -pedantic
      -Weverything)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(dmitigr_pgfe PRIVATE
      /Zc:throwingNew,referenceBinding,noexceptTypes
      /errorReport:none
      /nologo
      /utf-8)
  endif()
endif()

# ------------------------------------------------------------------------------
# Preprocessing
# ------------------------------------------------------------------------------

# Generated headers (.hpp) should go to the CMAKE_CURRENT_SOURCE_DIR, but
# generated implementations (.cxx) should go to the CMAKE_CURRENT_BINARY_DIR.

include("basics.cmake")
configure_file(basics.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/basics.hpp @ONLY
  NEWLINE_STYLE UNIX)

include("errc.cmake")
configure_file(errc.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/errc.hpp @ONLY
  NEWLINE_STYLE UNIX)

configure_file(internal/dll.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/internal/dll.hpp @ONLY
  NEWLINE_STYLE UNIX)

configure_file(connection_options.cxx.in ${CMAKE_CURRENT_BINARY_DIR}/connection_options.cxx @ONLY
  NEWLINE_STYLE UNIX)

# ------------------------------------------------------------------------------
# Installations
# ------------------------------------------------------------------------------

install(FILES
  array_conversions.hpp
  array_conversions.hxx
  array_conversions.tcc
  basic_conversions.hpp
  basics.hpp
  completion.hpp
  completion.hxx
  compositional.hpp
  compositional.hxx
  connection.hpp
  connection.hxx
  connection_options.hpp
  connection_options.hxx
  conversions_api.hpp
  conversions.hpp
  conversions.hxx
  data.hpp
  data.hxx
  errc.hpp
  errc.hxx
  error.hpp
  error.hxx
  exceptions.hpp
  exceptions.hxx
  message.hpp
  misc.hpp
  net.hxx
  notice.hpp
  notice.hxx
  notification.hpp
  notification.hxx
  parameterizable.hpp
  parameterizable.hxx
  pq.hxx
  prepared_statement.hpp
  prepared_statement.hxx
  problem.hpp
  problem.hxx
  response.hpp
  row.hpp
  row.hxx
  row_info.hpp
  row_info.hxx
  server_message.hpp
  signal.hpp
  sql.hxx
  sql_string.hpp
  sql_string.hxx
  sql_vector.hpp
  sql_vector.hxx
  std_system_error.hpp
  types_fwd.hpp

  DESTINATION ${PGFE_INCLUDES_INSTALL_DIR}/dmitigr/pgfe)

install(FILES
  internal/dll.hpp

  DESTINATION ${PGFE_INCLUDES_INSTALL_DIR}/dmitigr/pgfe/internal)

install(TARGETS dmitigr_pgfe
  EXPORT dmitigr_pgfe_export
  ARCHIVE  DESTINATION ${PGFE_LIBRARY_INSTALL_DIR}
  LIBRARY  DESTINATION ${PGFE_LIBRARY_INSTALL_DIR}
  RUNTIME  DESTINATION ${PGFE_LIBRARY_INSTALL_DIR}
  INCLUDES DESTINATION ${PGFE_INCLUDES_INSTALL_DIR})

install(EXPORT dmitigr_pgfe_export
  DESTINATION ${PGFE_CMAKE_INSTALL_DIR}
  FILE dmitigr_pgfe-config.cmake)

# ------------------------------------------------------------------------------

add_subdirectory(tests)
