# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

project(dmitigr_pgfe)

cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

if (NOT (UNIX OR WIN32))
  message(FATAL_ERROR "unsupported target platform")
endif()

list(APPEND CMAKE_MODULE_PATH ${dmitigr_pgfe_SOURCE_DIR}/cmake)
include(dmitigr)

set(dmitigr_pgfe_version_part1 1)
set(dmitigr_pgfe_version_part2 1)
add_definitions(-DDMITIGR_PGFE_VERSION_PART1=${dmitigr_pgfe_version_part1})
add_definitions(-DDMITIGR_PGFE_VERSION_PART2=${dmitigr_pgfe_version_part2})
if(WIN32)
  add_definitions(-DNOMINMAX)
endif()

# ------------------------------------------------------------------------------
# Build options
# ------------------------------------------------------------------------------

option(BUILD_SHARED_LIBS "Build shared library?" ON)
set(DMITIGR_PGFE_HEADER_ONLY OFF) # TODO
option(DMITIGR_PGFE_BUILD_TESTS "Build tests?" ON)

if(NOT DMITIGR_PGFE_HEADER_ONLY)
  if(BUILD_SHARED_LIBS)
    message("Building of shared library is enabled.")
    add_definitions(-DDMITIGR_PGFE_DLL_BUILDING)
  else()
    message("Building of static library is enabled.")
  endif()

  if(NOT DEFINED CMAKE_BUILD_TYPE OR NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
      "Build type: Debug Release RelWithDebInfo MinSizeRel." FORCE)
  endif()
  message("Build type is ${CMAKE_BUILD_TYPE}")
else()
  message("Header-only mode is enabled")
endif()

if(DMITIGR_PGFE_BUILD_TESTS)
  enable_testing()
  message("Building of tests is enabled")
endif()

# ------------------------------------------------------------------------------
# Installation options
# ------------------------------------------------------------------------------

if(UNIX)
  set(DMITIGR_PGFE_CMAKE_INSTALL_DIR "share/dmitigr_pgfe/cmake" CACHE
    STRING "Name of the installation directory for the CMake stuff relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_PGFE_DOC_INSTALL_DIR "share/dmitigr_pgfe/doc" CACHE
    STRING "Name of the installation directory for the documentation relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_PGFE_LIB_INSTALL_DIR "lib" CACHE
    STRING "Name of the installation directory for the libraries relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_PGFE_INCLUDE_INSTALL_DIR "include" CACHE
    STRING "Name of the installation directory for the includes relative to ${CMAKE_INSTALL_PREFIX}")
elseif(WIN32)
  # On Windows, CMAKE_INSTALL_PREFIX is $ENV{ProgramFiles}\\${CMAKE_PROJECT_NAME} by default. In turn:
  #   - on AMD64: ProgramFiles=%ProgramFiles%
  #   - on   x86: ProgramFiles=%ProgramFiles(x86)%
  # See: https://msdn.microsoft.com/en-us/library/aa384274.aspx
  set(DMITIGR_PGFE_CMAKE_INSTALL_DIR "cmake" CACHE
    STRING "Name of share directory relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_PGFE_DOC_INSTALL_DIR "doc" CACHE
    STRING "Name of the documentation directory relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_PGFE_LIB_INSTALL_DIR "lib" CACHE
    STRING "Name of the library directory relative to ${CMAKE_INSTALL_PREFIX}")
  set(DMITIGR_PGFE_INCLUDE_INSTALL_DIR "include" CACHE
    STRING "Name of includes directory relative to ${CMAKE_INSTALL_PREFIX}")
endif()

# ------------------------------------------------------------------------------
# Default connection options
# ------------------------------------------------------------------------------

if (UNIX)
  if(NOT DEFINED DMITIGR_PGFE_CONNECTION_COMMUNICATION_MODE)
    set(DMITIGR_PGFE_CONNECTION_COMMUNICATION_MODE "uds" CACHE
      STRING "The connection communication mode: \"uds\" or \"tcp\"")
  endif()
  if(NOT "${DMITIGR_PGFE_CONNECTION_COMMUNICATION_MODE}" STREQUAL "uds" AND
      NOT "${DMITIGR_PGFE_CONNECTION_COMMUNICATION_MODE}" STREQUAL "tcp")
    message(FATAL_ERROR "Invalid value of DMITIGR_PGFE_CONNECTION_COMMUNICATION_MODE")
  endif()

  if(NOT DEFINED DMITIGR_PGFE_CONNECTION_UDS_DIRECTORY)
    set(DMITIGR_PGFE_CONNECTION_UDS_DIRECTORY "/tmp" CACHE
      PATH "Absolute name of the directory where the Unix-domain socket file is located")
  endif()
  if ("${DMITIGR_PGFE_CONNECTION_UDS_DIRECTORY}" STREQUAL "")
    message(FATAL_ERROR "Invalid value of DMITIGR_PGFE_CONNECTION_UDS_DIRECTORY")
  endif()

  if(NOT DEFINED DMITIGR_PGFE_CONNECTION_UDS_REQUIRE_SERVER_PROCESS_USERNAME)
    set(DMITIGR_PGFE_CONNECTION_UDS_REQUIRE_SERVER_PROCESS_USERNAME "" CACHE
      STRING "The authentication requirement to the effective UID of the server process")
  endif()
  if (NOT "${DMITIGR_PGFE_CONNECTION_UDS_REQUIRE_SERVER_PROCESS_USERNAME}" STREQUAL "")
    set(DMITIGR_PGFE_CONNECTION_UDS_REQUIRE_SERVER_PROCESS_USERNAME_CPP
      "\"${DMITIGR_PGFE_CONNECTION_UDS_REQUIRE_SERVER_PROCESS_USERNAME}\"")
  endif()
else()
  # The PostgreSQL server doesn't support Unix Domain Sockets on Windows.
  set(DMITIGR_PGFE_CONNECTION_COMMUNICATION_MODE "tcp")
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_ENABLED)
  set(DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_ENABLED OFF CACHE
    BOOL "Keepalives mode")
endif()
if(${DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_ENABLED})
  set(DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_ENABLED_CPP "true")
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_IDLE)
  set(DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_IDLE "" CACHE
    STRING "Integer (in seconds). See TCP_KEEPIDLE (or its equivalent)")
endif()
if (NOT "${DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_IDLE}" STREQUAL "")
  if(NOT "${DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_IDLE}" MATCHES ^[0-9]+$)
    message(FATAL_ERROR "Invalid value of DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_IDLE")
  endif()
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_INTERVAL)
  set(DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_INTERVAL "" CACHE
    STRING "Integer (in seconds). See TCP_KEEPINTVL (or its equivalent)")
endif()
if (NOT "${DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_INTERVAL}" STREQUAL "")
  if(NOT "${DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_INTERVAL}" MATCHES ^[0-9]+$)
    message(FATAL_ERROR "Invalid value of DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_INTERVAL")
  endif()
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_COUNT)
  set(DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_COUNT "" CACHE
    STRING "Integer. See TCP_KEEPCNT (or its equivalent)")
endif()
if (NOT "${DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_COUNT}" STREQUAL "")
  if(NOT "${DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_COUNT}" MATCHES ^[0-9]+$)
    message(FATAL_ERROR "Invalid value of DMITIGR_PGFE_CONNECTION_TCP_KEEPALIVES_COUNT")
  endif()
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_TCP_ADDRESS)
  set(DMITIGR_PGFE_CONNECTION_TCP_ADDRESS "127.0.0.1" CACHE
    STRING "Numeric IP address of the host to connect to")
endif()
if (NOT "${DMITIGR_PGFE_CONNECTION_TCP_ADDRESS}" STREQUAL "")
  if (NOT "${DMITIGR_PGFE_CONNECTION_TCP_ADDRESS}" MATCHES ^[a-zA-Z0-9:]+[a-zA-Z0-9.:]+[a-zA-Z0-9]+$)
    message(FATAL_ERROR "Invalid value of DMITIGR_PGFE_CONNECTION_TCP_ADDRESS")
  else()
    set(DMITIGR_PGFE_CONNECTION_TCP_ADDRESS_CPP "\"${DMITIGR_PGFE_CONNECTION_TCP_ADDRESS}\"")
  endif()
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_TCP_HOSTNAME)
  set(DMITIGR_PGFE_CONNECTION_TCP_HOSTNAME "localhost" CACHE
    STRING "Name of the host to connect to")
endif()
if(NOT "${DMITIGR_PGFE_CONNECTION_TCP_HOSTNAME}" STREQUAL "")
  if (NOT "${DMITIGR_PGFE_CONNECTION_TCP_HOSTNAME}" MATCHES ^[a-zA-Z0-9._-]+$)
    message(FATAL_ERROR "Invalid value of DMITIGR_PGFE_CONNECTION_TCP_HOSTNAME")
  else()
    set(DMITIGR_PGFE_CONNECTION_TCP_HOSTNAME_CPP "\"${DMITIGR_PGFE_CONNECTION_TCP_HOSTNAME}\"")
  endif()
endif()

if ("${DMITIGR_PGFE_CONNECTION_TCP_HOSTNAME}" STREQUAL "" AND "${DMITIGR_PGFE_CONNECTION_TCP_ADDRESS}" STREQUAL "")
  message(FATAL_ERROR "Neither DMITIGR_PGFE_CONNECTION_TCP_HOSTNAME nor DMITIGR_PGFE_CONNECTION_TCP_ADDRESS specified")
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_PORT)
  set(DMITIGR_PGFE_CONNECTION_PORT "5432" CACHE
    STRING "Server port number")
endif()
if(NOT "${DMITIGR_PGFE_CONNECTION_PORT}" MATCHES ^[0-9]+$)
  message(FATAL_ERROR "Invalid value of DMITIGR_PGFE_CONNECTION_PORT")
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_USERNAME)
  set(DMITIGR_PGFE_CONNECTION_USERNAME "postgres" CACHE
    STRING "Name of the role registered on the server")
endif()
if(NOT "${DMITIGR_PGFE_CONNECTION_USERNAME}" STREQUAL "")
  if(NOT "${DMITIGR_PGFE_CONNECTION_USERNAME}" MATCHES ^[a-zA-Z_]+[a-zA-Z0-9$_]*$)
    message(WARNING "Probably problematic value of DMITIGR_PGFE_CONNECTION_USERNAME")
  endif()
  set(DMITIGR_PGFE_CONNECTION_USERNAME_CPP "\"${DMITIGR_PGFE_CONNECTION_USERNAME}\"")
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_DATABASE)
  set(DMITIGR_PGFE_CONNECTION_DATABASE "postgres" CACHE
    STRING "Name of the database on the server to connect to")
endif()
if(NOT "${DMITIGR_PGFE_CONNECTION_DATABASE}" STREQUAL "")
  if(NOT "${DMITIGR_PGFE_CONNECTION_DATABASE}" MATCHES ^[a-zA-Z_]+[a-zA-Z0-9$_]*$)
    message(WARNING "Probably problematic value of DMITIGR_PGFE_CONNECTION_DATABASE")
  endif()
  set(DMITIGR_PGFE_CONNECTION_DATABASE_CPP "\"${DMITIGR_PGFE_CONNECTION_DATABASE}\"")
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_PASSWORD)
  set(DMITIGR_PGFE_CONNECTION_PASSWORD "" CACHE
    STRING "The password for Password/LDAP authentication methods")
endif()
if (NOT "${DMITIGR_PGFE_CONNECTION_PASSWORD}" STREQUAL "")
  set(DMITIGR_PGFE_CONNECTION_PASSWORD_CPP "\"${DMITIGR_PGFE_CONNECTION_PASSWORD}\"")
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_KERBEROS_SERVICE_NAME)
  set(DMITIGR_PGFE_CONNECTION_KERBEROS_SERVICE_NAME "" CACHE
    STRING "Kerberos service name")
endif()
if (NOT "${DMITIGR_PGFE_CONNECTION_KERBEROS_SERVICE_NAME}" STREQUAL "")
  set(DMITIGR_PGFE_CONNECTION_KERBEROS_SERVICE_NAME_CPP "\"${DMITIGR_PGFE_CONNECTION_KERBEROS_SERVICE_NAME}\"")
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_SSL_ENABLED)
  set(DMITIGR_PGFE_CONNECTION_SSL_ENABLED OFF CACHE
    BOOL "The SSL mode")
endif()
if(${DMITIGR_PGFE_CONNECTION_SSL_ENABLED})
  set(DMITIGR_PGFE_CONNECTION_SSL_ENABLED_CPP "true")
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_SSL_SERVER_HOSTNAME_VERIFICATION_ENABLED)
  set(DMITIGR_PGFE_CONNECTION_SSL_SERVER_HOSTNAME_VERIFICATION_ENABLED OFF CACHE
    BOOL "The SSL server host name verification enabled")
endif()
if(${DMITIGR_PGFE_CONNECTION_SSL_SERVER_HOSTNAME_VERIFICATION_ENABLED})
  set(DMITIGR_PGFE_CONNECTION_SSL_SERVER_HOSTNAME_VERIFICATION_ENABLED_CPP "true")
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_SSL_COMPRESSION_ENABLED)
  set(DMITIGR_PGFE_CONNECTION_SSL_COMPRESSION_ENABLED OFF CACHE
    BOOL "The SSL compression enabled")
endif()
if(${DMITIGR_PGFE_CONNECTION_SSL_COMPRESSION_ENABLED})
  set(DMITIGR_PGFE_CONNECTION_SSL_COMPRESSION_ENABLED_CPP "true")
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_SSL_CERTIFICATE_FILE)
  set(DMITIGR_PGFE_CONNECTION_SSL_CERTIFICATE_FILE "" CACHE
    FILEPATH "The SSL certificate file")
endif()
if (NOT "${DMITIGR_PGFE_CONNECTION_SSL_CERTIFICATE_FILE}" STREQUAL "")
  set(DMITIGR_PGFE_CONNECTION_SSL_CERTIFICATE_FILE_CPP "\"${DMITIGR_PGFE_CONNECTION_SSL_CERTIFICATE_FILE}\"")
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_SSL_PRIVATE_KEY_FILE)
  set(DMITIGR_PGFE_CONNECTION_SSL_PRIVATE_KEY_FILE "" CACHE
    FILEPATH "The SSL private key file")
endif()
if (NOT "${DMITIGR_PGFE_CONNECTION_SSL_PRIVATE_KEY_FILE}" STREQUAL "")
  set(DMITIGR_PGFE_CONNECTION_SSL_PRIVATE_KEY_FILE_CPP "\"${DMITIGR_PGFE_CONNECTION_SSL_PRIVATE_KEY_FILE}\"")
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_SSL_CERTIFICATE_AUTHORITY_FILE)
  set(DMITIGR_PGFE_CONNECTION_SSL_CERTIFICATE_AUTHORITY_FILE "" CACHE
    FILEPATH "The SSL certificate authority (CA) file")
endif()
if (NOT "${DMITIGR_PGFE_CONNECTION_SSL_CERTIFICATE_AUTHORITY_FILE}" STREQUAL "")
  set(DMITIGR_PGFE_CONNECTION_SSL_CERTIFICATE_AUTHORITY_FILE_CPP "\"${DMITIGR_PGFE_CONNECTION_SSL_CERTIFICATE_AUTHORITY_FILE}\"")
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_SSL_CERTIFICATE_REVOCATION_LIST_FILE)
  set(DMITIGR_PGFE_CONNECTION_SSL_CERTIFICATE_REVOCATION_LIST_FILE "" CACHE
    FILEPATH "The SSL certificate revocation list (CRL) file")
endif()
if (NOT "${DMITIGR_PGFE_CONNECTION_SSL_CERTIFICATE_REVOCATION_LIST_FILE}" STREQUAL "")
  set(DMITIGR_PGFE_CONNECTION_SSL_CERTIFICATE_REVOCATION_LIST_FILE_CPP
    "\"${DMITIGR_PGFE_CONNECTION_SSL_CERTIFICATE_REVOCATION_LIST_FILE}\"")
endif()

# ------------------------------------------------------------------------------
# Languages
# ------------------------------------------------------------------------------

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/lib)
include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR}/lib)

# ------------------------------------------------------------------------------
# Preprocessing of the sources
# ------------------------------------------------------------------------------

# Generated headers (.hpp) should go to the CMAKE_CURRENT_SOURCE_DIR, but
# generated implementations (.cxx) should go to the CMAKE_CURRENT_BINARY_DIR.

include("lib/dmitigr/pgfe/basics.cmake")
configure_file(lib/dmitigr/pgfe/basics.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/lib/dmitigr/pgfe/basics.hpp @ONLY
  NEWLINE_STYLE UNIX)

include("lib/dmitigr/pgfe/errc.cmake")
configure_file(lib/dmitigr/pgfe/errc.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/lib/dmitigr/pgfe/errc.hpp @ONLY
  NEWLINE_STYLE UNIX)

configure_file(lib/dmitigr/pgfe/connection_options.cxx.in ${CMAKE_CURRENT_BINARY_DIR}/lib/dmitigr/pgfe/connection_options.cxx @ONLY
  NEWLINE_STYLE UNIX)

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(dmitigr_pgfe_root_headers
  lib/dmitigr/pgfe.hpp
  )

set(dmitigr_pgfe_preprocessed_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/dmitigr/pgfe/basics.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/dmitigr/pgfe/errc.hpp
  ${CMAKE_CURRENT_BINARY_DIR}/lib/dmitigr/pgfe/connection_options.cxx
  )

# set(dmitigr_pgfe_header_only_headers
#   lib/dmitigr/pgfe/header_only.hpp
#   )

set(dmitigr_pgfe_headers
  lib/dmitigr/pgfe/array_conversions.hpp
  lib/dmitigr/pgfe/array_conversions.hxx
  lib/dmitigr/pgfe/array_conversions.tcc
  lib/dmitigr/pgfe/basic_conversions.hpp

  lib/dmitigr/pgfe/completion.hpp
  lib/dmitigr/pgfe/completion.hxx

  lib/dmitigr/pgfe/compositional.hpp
  lib/dmitigr/pgfe/compositional.hxx

  lib/dmitigr/pgfe/composite.hpp
  lib/dmitigr/pgfe/composite.hxx

  lib/dmitigr/pgfe/connection.hpp
  lib/dmitigr/pgfe/connection.hxx

  lib/dmitigr/pgfe/connection_options.hpp
  lib/dmitigr/pgfe/connection_options.hxx

  lib/dmitigr/pgfe/conversions_api.hpp
  lib/dmitigr/pgfe/conversions.hpp
  lib/dmitigr/pgfe/conversions.hxx

  lib/dmitigr/pgfe/data.hpp
  lib/dmitigr/pgfe/data.hxx

  lib/dmitigr/pgfe/dll.hpp

  lib/dmitigr/pgfe/errc.hxx

  lib/dmitigr/pgfe/error.hpp
  lib/dmitigr/pgfe/error.hxx

  lib/dmitigr/pgfe/exceptions.hpp
  lib/dmitigr/pgfe/exceptions.hxx

  lib/dmitigr/pgfe/message.hpp

  lib/dmitigr/pgfe/misc.hpp

  lib/dmitigr/pgfe/net.hxx

  lib/dmitigr/pgfe/notice.hpp
  lib/dmitigr/pgfe/notice.hxx

  lib/dmitigr/pgfe/notification.hpp
  lib/dmitigr/pgfe/notification.hxx

  lib/dmitigr/pgfe/parameterizable.hpp
  lib/dmitigr/pgfe/parameterizable.hxx

  lib/dmitigr/pgfe/pq.hxx

  lib/dmitigr/pgfe/prepared_statement.hpp
  lib/dmitigr/pgfe/prepared_statement.hxx

  lib/dmitigr/pgfe/problem.hpp
  lib/dmitigr/pgfe/problem.hxx

  lib/dmitigr/pgfe/response.hpp

  lib/dmitigr/pgfe/row.hpp
  lib/dmitigr/pgfe/row.hxx

  lib/dmitigr/pgfe/row_info.hpp
  lib/dmitigr/pgfe/row_info.hxx

  lib/dmitigr/pgfe/server_message.hpp

  lib/dmitigr/pgfe/signal.hpp

  lib/dmitigr/pgfe/sql.hxx

  lib/dmitigr/pgfe/sql_string.hpp
  lib/dmitigr/pgfe/sql_string.hxx

  lib/dmitigr/pgfe/sql_vector.hpp
  lib/dmitigr/pgfe/sql_vector.hxx

  lib/dmitigr/pgfe/std_system_error.hpp

  lib/dmitigr/pgfe/types_fwd.hpp
  )

set(dmitigr_pgfe_cmake_sources
  lib/dmitigr/pgfe/basics.cmake
  lib/dmitigr/pgfe/errc.cmake
  )

set(dmitigr_pgfe_cmake_unpreprocessed
  lib/dmitigr/pgfe/basics.hpp.in
  lib/dmitigr/pgfe/connection_options.cxx.in
  lib/dmitigr/pgfe/errc.hpp.in
  )

set(dmitigr_pgfe_transunits
  lib/dmitigr/pgfe/basics.cpp
  lib/dmitigr/pgfe/composite.cpp
  lib/dmitigr/pgfe/connection.cpp
  lib/dmitigr/pgfe/connection_options.cpp
  lib/dmitigr/pgfe/data.cpp
  lib/dmitigr/pgfe/errc.cxx
  lib/dmitigr/pgfe/misc.cpp
  lib/dmitigr/pgfe/net.cxx
  lib/dmitigr/pgfe/prepared_statement.cxx
  lib/dmitigr/pgfe/problem.cpp
  lib/dmitigr/pgfe/sql.cxx
  lib/dmitigr/pgfe/sql_string.cpp
  lib/dmitigr/pgfe/sql_string.cxx
  lib/dmitigr/pgfe/sql_vector.cpp
  lib/dmitigr/pgfe/std_system_error.cpp
  )

set(dmitigr_pgfe_sources
  ${dmitigr_pgfe_root_headers}
  #${dmitigr_pgfe_header_only_headers}
  ${dmitigr_pgfe_headers}
  ${dmitigr_pgfe_cmake_sources}
  ${dmitigr_pgfe_cmake_unpreprocessed}
  ${dmitigr_pgfe_transunits}
  )

if (WIN32)
  list(APPEND dmitigr_pgfe_sources lib/dmitigr/pgfe/version.rc)
endif()

set_source_files_properties(
  ${dmitigr_pgfe_cmake_sources}
  ${dmitigr_pgfe_cmake_unpreprocessed}

  PROPERTIES
  HEADER_FILE_ONLY ON)

# ------------------------------------------------------------------------------
# Targets
# ------------------------------------------------------------------------------

# Note: Multi-configuration generators (VS, Xcode) appends a per-configuration
# subdirectory to CMAKE_RUNTIME_OUTPUT_DIRECTORY unless a generator expression
# is used.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

if(NOT DMITIGR_PGFE_HEADER_ONLY)
  if(BUILD_SHARED_LIBS)
    set(dmitigr_pgfe_target "dmitigr_pgfe")
  else()
    set(dmitigr_pgfe_target "dmitigr_pgfe_static")
  endif()

  add_library(${dmitigr_pgfe_target} ${dmitigr_pgfe_sources})

  if(BUILD_SHARED_LIBS)
    target_compile_definitions(dmitigr_pgfe PUBLIC -DDMITIGR_PGFE_DLL)
  endif()

  set_target_properties(${dmitigr_pgfe_target}
    PROPERTIES
    LINKER_LANGUAGE "CXX"
    POSITION_INDEPENDENT_CODE True
    VERSION ${dmitigr_pgfe_version_part1}.${dmitigr_pgfe_version_part2}
    DEBUG_POSTFIX "d")

  dmitigr_target_compile_options(${dmitigr_pgfe_target})
else() # for header-only
  set(dmitigr_pgfe_target "dmitigr_pgfe_interface")

  add_library(dmitigr_pgfe_interface INTERFACE)

  target_compile_definitions(dmitigr_pgfe_interface INTERFACE -DDMITIGR_PGFE_HEADER_ONLY)
endif()

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

#
# dmitigr_common
#

find_package(dmitigr_common REQUIRED)
message("Using ${dmitigr_common_library} library")

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

# --------------------------------------

if(NOT DMITIGR_PGFE_HEADER_ONLY)
  target_link_libraries(${dmitigr_pgfe_target}
    PRIVATE ${Suggested_Pq_LIBRARIES} ${dmitigr_common_library})

  if (UNIX)
    target_link_libraries(${dmitigr_pgfe_target} PRIVATE stdc++fs)
  elseif (WIN32)
    target_link_libraries(${dmitigr_pgfe_target} PRIVATE Ws2_32.lib)
  endif()
else()
  target_link_libraries(dmitigr_pgfe_interface
    INTERFACE ${Suggested_Pq_LIBRARIES} ${dmitigr_common_library})

  if (UNIX)
    target_link_libraries(dmitigr_pgfe_interface INTERFACE stdc++fs)
  elseif (WIN32)
    target_link_libraries(dmitigr_pgfe_interface INTERFACE Ws2_32.lib)
  endif()
endif()

# ------------------------------------------------------------------------------
# Installing
# ------------------------------------------------------------------------------

install(FILES ${dmitigr_pgfe_root_headers}
  DESTINATION ${DMITIGR_PGFE_INCLUDE_INSTALL_DIR}/dmitigr)

install(FILES ${dmitigr_pgfe_preprocessed_headers}
  DESTINATION ${DMITIGR_PGFE_INCLUDE_INSTALL_DIR}/dmitigr/pgfe)

if(DMITIGR_PGFE_HEADER_ONLY)
  install(FILES ${dmitigr_pgfe_header_only_headers}
    DESTINATION ${DMITIGR_PGFE_INCLUDE_INSTALL_DIR}/dmitigr/pgfe)
endif()

install(FILES ${dmitigr_pgfe_headers}
  DESTINATION ${DMITIGR_PGFE_INCLUDE_INSTALL_DIR}/dmitigr/pgfe)

if(NOT DMITIGR_PGFE_HEADER_ONLY)
  install(TARGETS ${dmitigr_pgfe_target}
    EXPORT ${dmitigr_pgfe_target}_export
    ARCHIVE  DESTINATION ${DMITIGR_PGFE_LIB_INSTALL_DIR}
    LIBRARY  DESTINATION ${DMITIGR_PGFE_LIB_INSTALL_DIR}
    RUNTIME  DESTINATION ${DMITIGR_PGFE_LIB_INSTALL_DIR}
    INCLUDES DESTINATION ${DMITIGR_PGFE_INCLUDE_INSTALL_DIR})
else()
  install(TARGETS dmitigr_pgfe_interface
    EXPORT dmitigr_pgfe_interface_export
    INCLUDES DESTINATION ${DMITIGR_PGFE_INCLUDE_INSTALL_DIR})

  install(FILES ${dmitigr_pgfe_transunits}
    DESTINATION ${DMITIGR_PGFE_INCLUDE_INSTALL_DIR}/dmitigr/pgfe)
endif()

install(EXPORT ${dmitigr_pgfe_target}_export
  DESTINATION ${DMITIGR_PGFE_CMAKE_INSTALL_DIR}
  FILE ${dmitigr_pgfe_target}-config.cmake)

# --------------------------------------

install(FILES
  doc/dmitigr_pgfe.class.violet.html

  DESTINATION ${DMITIGR_PGFE_DOC_INSTALL_DIR})

# ------------------------------------------------------------------------------
# Uninstalling
# ------------------------------------------------------------------------------

add_custom_target(uninstall)

add_custom_command(TARGET uninstall PRE_BUILD
  COMMAND cmake -E remove -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_PGFE_CMAKE_INSTALL_DIR}/dmitigr_pgfe*
  COMMAND cmake -E remove -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_PGFE_DOC_INSTALL_DIR}/dmitigr_pgfe*
  COMMAND cmake -E remove -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_PGFE_LIB_INSTALL_DIR}/dmitigr_pgfe*
  COMMAND cmake -E remove -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_PGFE_LIB_INSTALL_DIR}/libdmitigr_pgfe*
  COMMAND cmake -E remove -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_PGFE_INCLUDE_INSTALL_DIR}/dmitigr/pgfe*
  COMMAND cmake -E remove_directory ${CMAKE_INSTALL_PREFIX}/${DMITIGR_PGFE_INCLUDE_INSTALL_DIR}/dmitigr/pgfe
  )

# ------------------------------------------------------------------------------

if(DMITIGR_PGFE_BUILD_TESTS)
  add_subdirectory(tests)
endif()
