# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

project(dmitigr_pgfe)

cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

if (NOT (UNIX OR WIN32))
  message(FATAL_ERROR "unsupported target platform")
endif()

set(dmitigr_pgfe_version_part1 1)
set(dmitigr_pgfe_version_part2 1)
add_definitions(-DDMITIGR_PGFE_VERSION_PART1=${dmitigr_pgfe_version_part1})
add_definitions(-DDMITIGR_PGFE_VERSION_PART2=${dmitigr_pgfe_version_part2})
if(WIN32)
  add_definitions(-DNOMINMAX)
endif()

# ------------------------------------------------------------------------------

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE
    STRING "Build type: Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
endif()
message("Build type is ${CMAKE_BUILD_TYPE}")

# ------------------------------------------------------------------------------

set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared library")

if(BUILD_SHARED_LIBS)
  message("Building shared library enabled.")
  add_definitions(-DDMITIGR_PGFE_DLL_BUILDING)
  set(DMITIGR_PGFE_DLL 1)
else()
  message("Building shared library disabled.")
  set(DMITIGR_PGFE_DLL 0)
endif()

# ------------------------------------------------------------------------------

option(DMITIGR_PGFE_BUILD_TESTS "Build tests?" ON)

if(DMITIGR_PGFE_BUILD_TESTS)
  enable_testing()
  message("Building tests enabled.")
else()
  message("Building tests disabled.")
endif()

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

  if(NOT DEFINED DMITIGR_PGFE_CONNECTION_UDS_FILE_EXTENSION)
    set(DMITIGR_PGFE_CONNECTION_UDS_FILE_EXTENSION "5432" CACHE
      STRING "Unix-domain socket file extension")
  endif()
  if ("${DMITIGR_PGFE_CONNECTION_UDS_FILE_EXTENSION}" STREQUAL "")
    message(FATAL_ERROR "Invalid value of DMITIGR_PGFE_CONNECTION_UDS_FILE_EXTENSION")
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

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_TCP_HOST_ADDRESS)
  set(DMITIGR_PGFE_CONNECTION_TCP_HOST_ADDRESS "127.0.0.1" CACHE
    STRING "Numeric IP address of the host to connect to")
endif()
if (NOT "${DMITIGR_PGFE_CONNECTION_TCP_HOST_ADDRESS}" STREQUAL "")
  if (NOT "${DMITIGR_PGFE_CONNECTION_TCP_HOST_ADDRESS}" MATCHES ^[a-zA-Z0-9:]+[a-zA-Z0-9.:]+[a-zA-Z0-9]+$)
    message(FATAL_ERROR "Invalid value of DMITIGR_PGFE_CONNECTION_TCP_HOST_ADDRESS")
  else()
    set(DMITIGR_PGFE_CONNECTION_TCP_HOST_ADDRESS_CPP "\"${DMITIGR_PGFE_CONNECTION_TCP_HOST_ADDRESS}\"")
  endif()
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_TCP_HOST_NAME)
  set(DMITIGR_PGFE_CONNECTION_TCP_HOST_NAME "localhost" CACHE
    STRING "Name of the host to connect to")
endif()
if(NOT "${DMITIGR_PGFE_CONNECTION_TCP_HOST_NAME}" STREQUAL "")
  if (NOT "${DMITIGR_PGFE_CONNECTION_TCP_HOST_NAME}" MATCHES ^[a-zA-Z0-9._-]+$)
    message(FATAL_ERROR "Invalid value of DMITIGR_PGFE_CONNECTION_TCP_HOST_NAME")
  else()
    set(DMITIGR_PGFE_CONNECTION_TCP_HOST_NAME_CPP "\"${DMITIGR_PGFE_CONNECTION_TCP_HOST_NAME}\"")
  endif()
endif()

if ("${DMITIGR_PGFE_CONNECTION_TCP_HOST_NAME}" STREQUAL "" AND "${DMITIGR_PGFE_CONNECTION_TCP_HOST_ADDRESS}" STREQUAL "")
  message(FATAL_ERROR "Neither DMITIGR_PGFE_CONNECTION_TCP_HOST_NAME nor DMITIGR_PGFE_CONNECTION_TCP_HOST_ADDRESS specified")
endif()

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_TCP_HOST_PORT)
  set(DMITIGR_PGFE_CONNECTION_TCP_HOST_PORT "5432" CACHE
    STRING "Server port number")
endif()
if(NOT "${DMITIGR_PGFE_CONNECTION_TCP_HOST_PORT}" MATCHES ^[0-9]+$)
  message(FATAL_ERROR "Invalid value of DMITIGR_PGFE_CONNECTION_TCP_HOST_PORT")
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

if(NOT DEFINED DMITIGR_PGFE_CONNECTION_SSL_SERVER_HOST_NAME_VERIFICATION_ENABLED)
  set(DMITIGR_PGFE_CONNECTION_SSL_SERVER_HOST_NAME_VERIFICATION_ENABLED OFF CACHE
    BOOL "The SSL server host name verification enabled")
endif()
if(${DMITIGR_PGFE_CONNECTION_SSL_SERVER_HOST_NAME_VERIFICATION_ENABLED})
  set(DMITIGR_PGFE_CONNECTION_SSL_SERVER_HOST_NAME_VERIFICATION_ENABLED_CPP "true")
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

add_custom_target(uninstall)

add_custom_command(TARGET uninstall PRE_BUILD
  COMMAND cmake -E remove -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_PGFE_LIB_INSTALL_DIR}/libdmitigr_pgfe*
  COMMAND cmake -E remove -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_PGFE_LIB_INSTALL_DIR}/dmitigr_pgfe*.dll
  COMMAND cmake -E remove -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_PGFE_LIB_INSTALL_DIR}/dmitigr_pgfe*.lib

  COMMAND cmake -E remove -f ${CMAKE_INSTALL_PREFIX}/${DMITIGR_PGFE_INCLUDE_INSTALL_DIR}/dmitigr/pgfe.hpp
  COMMAND cmake -E remove_directory ${CMAKE_INSTALL_PREFIX}/${DMITIGR_PGFE_INCLUDE_INSTALL_DIR}/dmitigr/pgfe
  COMMAND cmake -E remove_directory ${CMAKE_INSTALL_PREFIX}/${DMITIGR_PGFE_CMAKE_INSTALL_DIR}
  COMMAND cmake -E remove_directory ${CMAKE_INSTALL_PREFIX}/${DMITIGR_PGFE_DOC_INSTALL_DIR}
  )

# -------------------------------------------------------------------------------

add_subdirectory(doc)
add_subdirectory(lib)
