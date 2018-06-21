# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

cmake_minimum_required(VERSION 3.10)
cmake_policy(VERSION 3.10)

if (NOT (UNIX OR WIN32))
  message(FATAL_ERROR "unsupported target platform")
endif()

project(dmitigr_pgfe)

set(PGFE_VERSION_PART1 1)
set(PGFE_VERSION_PART2 1)
add_definitions(-DPGFE_VERSION_PART1=${PGFE_VERSION_PART1})
add_definitions(-DPGFE_VERSION_PART2=${PGFE_VERSION_PART2})

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

set(PGFE_BUILD_TESTS ON CACHE BOOL "Build library tests")

if(PGFE_BUILD_TESTS)
  enable_testing()
  message("Building library tests enabled.")
else()
  message("Building library tests disabled.")
endif()

# ------------------------------------------------------------------------------

if (UNIX)
  set(PGFE_CMAKE_INSTALL_DIR "share/dmitigr_pgfe/cmake" CACHE
    STRING "Name of the cmake directory relative to ${CMAKE_INSTALL_PREFIX}")
  set(PGFE_DOC_INSTALL_DIR "share/dmitigr_pgfe/doc" CACHE
    STRING "Name of the documentation directory relative to ${CMAKE_INSTALL_PREFIX}")
  set(PGFE_LIBRARY_INSTALL_DIR "lib" CACHE
    STRING "Name of the library directory relative to ${CMAKE_INSTALL_PREFIX}")
  set(PGFE_INCLUDES_INSTALL_DIR "include" CACHE
    STRING "Name of includes directory relative to ${CMAKE_INSTALL_PREFIX}")
elseif(WIN32)
  # On Windows, CMAKE_INSTALL_PREFIX is $ENV{ProgramFiles}\\${CMAKE_PROJECT_NAME} by default. In turn:
  #   - on AMD64: ProgramFiles=%ProgramFiles%
  #   - on   x86: ProgramFiles=%ProgramFiles(x86)%
  # See: https://msdn.microsoft.com/en-us/library/aa384274.aspx
  set(PGFE_CMAKE_INSTALL_DIR "cmake" CACHE
    STRING "Name of share directory relative to ${CMAKE_INSTALL_PREFIX}")
  set(PGFE_DOC_INSTALL_DIR "doc" CACHE
    STRING "Name of the documentation directory relative to ${CMAKE_INSTALL_PREFIX}")
  set(PGFE_LIBRARY_INSTALL_DIR "lib" CACHE
    STRING "Name of the library directory relative to ${CMAKE_INSTALL_PREFIX}")
  set(PGFE_INCLUDES_INSTALL_DIR "include" CACHE
    STRING "Name of includes directory relative to ${CMAKE_INSTALL_PREFIX}")
endif()

# ------------------------------------------------------------------------------
# Default connection options
# ------------------------------------------------------------------------------

if (UNIX)
  set(PGFE_CONNECTION_COMMUNICATION_MODE "uds" CACHE
    STRING "The connection communication mode: \"uds\" or \"tcp\"" FORCE)
  if(NOT "${PGFE_CONNECTION_COMMUNICATION_MODE}" STREQUAL "uds" AND
     NOT "${PGFE_CONNECTION_COMMUNICATION_MODE}" STREQUAL "tcp")
    message(FATAL_ERROR "Invalid value of PGFE_CONNECTION_COMMUNICATION_MODE")
  endif()

  set(PGFE_CONNECTION_UDS_DIRECTORY "/tmp" CACHE
    PATH "Absolute name of the directory where the Unix-domain socket file is located" FORCE)
  if ("${PGFE_CONNECTION_UDS_DIRECTORY}" STREQUAL "")
    message(FATAL_ERROR "Invalid value of PGFE_CONNECTION_UDS_DIRECTORY")
  endif()

 set(PGFE_CONNECTION_UDS_FILE_EXTENSION "5432" CACHE
   STRING "Unix-domain socket file extension" FORCE)
  if ("${PGFE_CONNECTION_UDS_FILE_EXTENSION}" STREQUAL "")
    message(FATAL_ERROR "Invalid value of PGFE_CONNECTION_UDS_FILE_EXTENSION")
  endif()

  set(PGFE_CONNECTION_UDS_REQUIRE_SERVER_PROCESS_USERNAME "" CACHE
    STRING "The authentication requirement to the effective UID of the server process")
  if (NOT "${PGFE_CONNECTION_UDS_REQUIRE_SERVER_PROCESS_USERNAME}" STREQUAL "")
    set(PGFE_CONNECTION_UDS_REQUIRE_SERVER_PROCESS_USERNAME_CPP "\"${PGFE_CONNECTION_UDS_REQUIRE_SERVER_PROCESS_USERNAME}\"")
  endif()
else()
  # The PostgreSQL server doesn't support listening on Unix Domain Sockets on Windows.
  set(PGFE_CONNECTION_COMMUNICATION_MODE "tcp")
endif()

set(PGFE_CONNECTION_TCP_KEEPALIVES_ENABLED OFF CACHE
  BOOL "Keepalives mode" FORCE)
if(${PGFE_CONNECTION_TCP_KEEPALIVES_ENABLED})
  set(PGFE_CONNECTION_TCP_KEEPALIVES_ENABLED_CPP "true")
endif()

set(PGFE_CONNECTION_TCP_KEEPALIVES_IDLE "" CACHE
  STRING "Integer (in seconds). See TCP_KEEPIDLE (or its equivalent)" FORCE)
if (NOT "${PGFE_CONNECTION_TCP_KEEPALIVES_IDLE}" STREQUAL "")
  if(NOT "${PGFE_CONNECTION_TCP_KEEPALIVES_IDLE}" MATCHES ^[0-9]+$)
    message(FATAL_ERROR "Invalid value of PGFE_CONNECTION_TCP_KEEPALIVES_IDLE")
  endif()
endif()

set(PGFE_CONNECTION_TCP_KEEPALIVES_INTERVAL "" CACHE
  STRING "Integer (in seconds). See TCP_KEEPINTVL (or its equivalent)" FORCE)
if (NOT "${PGFE_CONNECTION_TCP_KEEPALIVES_INTERVAL}" STREQUAL "")
  if(NOT "${PGFE_CONNECTION_TCP_KEEPALIVES_INTERVAL}" MATCHES ^[0-9]+$)
    message(FATAL_ERROR "Invalid value of PGFE_CONNECTION_TCP_KEEPALIVES_INTERVAL")
  endif()
endif()

set(PGFE_CONNECTION_TCP_KEEPALIVES_COUNT "" CACHE
  STRING "Integer. See TCP_KEEPCNT (or its equivalent)" FORCE)
if (NOT "${PGFE_CONNECTION_TCP_KEEPALIVES_COUNT}" STREQUAL "")
  if(NOT "${PGFE_CONNECTION_TCP_KEEPALIVES_COUNT}" MATCHES ^[0-9]+$)
    message(FATAL_ERROR "Invalid value of PGFE_CONNECTION_TCP_KEEPALIVES_COUNT")
  endif()
endif()

set(PGFE_CONNECTION_TCP_HOST_ADDRESS "127.0.0.1" CACHE
  STRING "Numeric IP address of the host to connect to" FORCE)
if (NOT "${PGFE_CONNECTION_TCP_HOST_ADDRESS}" STREQUAL "")
  if (NOT "${PGFE_CONNECTION_TCP_HOST_ADDRESS}" MATCHES ^[a-zA-Z0-9:]+[a-zA-Z0-9.:]+[a-zA-Z0-9]+$)
    message(FATAL_ERROR "Invalid value of PGFE_CONNECTION_TCP_HOST_ADDRESS")
  else()
    set(PGFE_CONNECTION_TCP_HOST_ADDRESS_CPP "\"${PGFE_CONNECTION_TCP_HOST_ADDRESS}\"")
  endif()
endif()

set(PGFE_CONNECTION_TCP_HOST_NAME "localhost" CACHE
  STRING "Name of the host to connect to" FORCE)
if(NOT "${PGFE_CONNECTION_TCP_HOST_NAME}" STREQUAL "")
  if (NOT "${PGFE_CONNECTION_TCP_HOST_NAME}" MATCHES ^[a-zA-Z0-9._-]+$)
    message(FATAL_ERROR "Invalid value of PGFE_CONNECTION_TCP_HOST_NAME")
  else()
    set(PGFE_CONNECTION_TCP_HOST_NAME_CPP "\"${PGFE_CONNECTION_TCP_HOST_NAME}\"")
  endif()
endif()

if ("${PGFE_CONNECTION_TCP_HOST_NAME}" STREQUAL "" AND "${PGFE_CONNECTION_TCP_HOST_ADDRESS}" STREQUAL "")
  message(FATAL_ERROR "Neither PGFE_CONNECTION_TCP_HOST_NAME nor PGFE_CONNECTION_TCP_HOST_ADDRESS specified")
endif()

set(PGFE_CONNECTION_TCP_HOST_PORT "5432" CACHE
  STRING "Server port number" FORCE)
if(NOT "${PGFE_CONNECTION_TCP_HOST_PORT}" MATCHES ^[0-9]+$)
  message(FATAL_ERROR "Invalid value of PGFE_CONNECTION_TCP_HOST_PORT")
endif()

set(PGFE_CONNECTION_USERNAME "postgres" CACHE
  STRING "Name of the role registered on the server" FORCE)
if(NOT "${PGFE_CONNECTION_USERNAME}" MATCHES ^[a-zA-Z_]+[a-zA-Z0-9$_]*$)
  message(FATAL_ERROR "Invalid value of PGFE_CONNECTION_USERNAME")
endif()

set(PGFE_CONNECTION_DATABASE "postgres" CACHE
  STRING "Name of the database on the server to connect to" FORCE)
if(NOT "${PGFE_CONNECTION_DATABASE}" MATCHES ^[a-zA-Z_]+[a-zA-Z0-9$_]*$)
  message(FATAL_ERROR "Invalid value of PGFE_CONNECTION_DATABASE")
endif()

set(PGFE_CONNECTION_PASSWORD "" CACHE
  STRING "The password for Password/LDAP authentication methods" FORCE)
if (NOT "${PGFE_CONNECTION_PASSWORD}" STREQUAL "")
  set(PGFE_CONNECTION_PASSWORD_CPP "\"${PGFE_CONNECTION_PASSWORD}\"")
endif()

set(PGFE_CONNECTION_KERBEROS_SERVICE_NAME "" CACHE
  STRING "Kerberos service name" FORCE)
if (NOT "${PGFE_CONNECTION_KERBEROS_SERVICE_NAME}" STREQUAL "")
  set(PGFE_CONNECTION_KERBEROS_SERVICE_NAME_CPP "\"${PGFE_CONNECTION_KERBEROS_SERVICE_NAME}\"")
endif()

set(PGFE_CONNECTION_SSL_ENABLED OFF CACHE
  BOOL "The SSL mode" FORCE)
if(${PGFE_CONNECTION_SSL_ENABLED})
  set(PGFE_CONNECTION_SSL_ENABLED_CPP "true")
endif()

set(PGFE_CONNECTION_SSL_SERVER_HOST_NAME_VERIFICATION_ENABLED OFF CACHE
  BOOL "The SSL server host name verification enabled" FORCE)
if(${PGFE_CONNECTION_SSL_SERVER_HOST_NAME_VERIFICATION_ENABLED})
  set(PGFE_CONNECTION_SSL_SERVER_HOST_NAME_VERIFICATION_ENABLED_CPP "true")
endif()

set(PGFE_CONNECTION_SSL_COMPRESSION_ENABLED OFF CACHE
  BOOL "The SSL compression enabled" FORCE)
if(${PGFE_CONNECTION_SSL_COMPRESSION_ENABLED})
  set(PGFE_CONNECTION_SSL_COMPRESSION_ENABLED_CPP "true")
endif()

set(PGFE_CONNECTION_SSL_CERTIFICATE_FILE "" CACHE
  FILEPATH "The SSL certificate file" FORCE)
if (NOT "${PGFE_CONNECTION_SSL_CERTIFICATE_FILE}" STREQUAL "")
  set(PGFE_CONNECTION_SSL_CERTIFICATE_FILE_CPP "\"${PGFE_CONNECTION_SSL_CERTIFICATE_FILE}\"")
endif()

set(PGFE_CONNECTION_SSL_PRIVATE_KEY_FILE "" CACHE
  FILEPATH "The SSL private key file" FORCE)
if (NOT "${PGFE_CONNECTION_SSL_PRIVATE_KEY_FILE}" STREQUAL "")
  set(PGFE_CONNECTION_SSL_PRIVATE_KEY_FILE_CPP "\"${PGFE_CONNECTION_SSL_PRIVATE_KEY_FILE}\"")
endif()

set(PGFE_CONNECTION_SSL_CERTIFICATE_AUTHORITY_FILE "" CACHE
  FILEPATH "The SSL certificate authority (CA) file" FORCE)
if (NOT "${PGFE_CONNECTION_SSL_CERTIFICATE_AUTHORITY_FILE}" STREQUAL "")
  set(PGFE_CONNECTION_SSL_CERTIFICATE_AUTHORITY_FILE_CPP "\"${PGFE_CONNECTION_SSL_CERTIFICATE_AUTHORITY_FILE}\"")
endif()

set(PGFE_CONNECTION_SSL_CERTIFICATE_REVOCATION_LIST_FILE "" CACHE
  FILEPATH "The SSL certificate revocation list (CRL) file" FORCE)
if (NOT "${PGFE_CONNECTION_SSL_CERTIFICATE_REVOCATION_LIST_FILE}" STREQUAL "")
  set(PGFE_CONNECTION_SSL_CERTIFICATE_REVOCATION_LIST_FILE_CPP "\"${PGFE_CONNECTION_SSL_CERTIFICATE_REVOCATION_LIST_FILE}\"")
endif()

# ------------------------------------------------------------------------------

add_custom_target(uninstall)

add_custom_command(TARGET uninstall PRE_BUILD
  COMMAND cmake -E remove -f ${CMAKE_INSTALL_PREFIX}/${PGFE_LIBRARY_INSTALL_DIR}/libdmitigr_pgfe*
  COMMAND cmake -E remove -f ${CMAKE_INSTALL_PREFIX}/${PGFE_LIBRARY_INSTALL_DIR}/dmitigr_pgfe*.dll
  COMMAND cmake -E remove -f ${CMAKE_INSTALL_PREFIX}/${PGFE_LIBRARY_INSTALL_DIR}/dmitigr_pgfe*.lib

  COMMAND cmake -E remove -f ${CMAKE_INSTALL_PREFIX}/${PGFE_INCLUDES_INSTALL_DIR}/dmitigr/pgfe.hpp
  COMMAND cmake -E remove_directory ${CMAKE_INSTALL_PREFIX}/${PGFE_INCLUDES_INSTALL_DIR}/dmitigr/pgfe
  COMMAND cmake -E remove_directory ${CMAKE_INSTALL_PREFIX}/${PGFE_CMAKE_INSTALL_DIR}
  COMMAND cmake -E remove_directory ${CMAKE_INSTALL_PREFIX}/${PGFE_DOC_INSTALL_DIR}
  )

# -------------------------------------------------------------------------------

add_subdirectory(doc)
add_subdirectory(lib)
