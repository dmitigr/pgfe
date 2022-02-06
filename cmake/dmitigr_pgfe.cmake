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

# ------------------------------------------------------------------------------
# Info
# ------------------------------------------------------------------------------

dmitigr_cpplipa_set_library_info(pgfe 1 0 "Client API for PostgreSQL")

# ------------------------------------------------------------------------------
# Default connection options
# ------------------------------------------------------------------------------

if (UNIX AND NOT CMAKE_SYSTEM_NAME MATCHES MSYS|MinGW|Cygwin)
  if(NOT DEFINED DMITIGR_PGFE_CONNECTION_COMMUNICATION_MODE)
    set(DMITIGR_PGFE_CONNECTION_COMMUNICATION_MODE "uds" CACHE
      STRING "The connection communication mode: \"uds\" or \"net\"")
  endif()
  if(NOT "${DMITIGR_PGFE_CONNECTION_COMMUNICATION_MODE}" STREQUAL "uds" AND
      NOT "${DMITIGR_PGFE_CONNECTION_COMMUNICATION_MODE}" STREQUAL "net")
    message(FATAL_ERROR "Invalid value of DMITIGR_PGFE_CONNECTION_COMMUNICATION_MODE")
  endif()

  if(NOT DEFINED DMITIGR_PGFE_CONNECTION_CONNECT_TIMEOUT)
    set(DMITIGR_PGFE_CONNECTION_CONNECT_TIMEOUT "10000" CACHE
      STRING "Integer (in milliseconds).")
  endif()
  if (NOT "${DMITIGR_PGFE_CONNECTION_CONNECT_TIMEOUT}" STREQUAL "")
    if(NOT "${DMITIGR_PGFE_CONNECTION_CONNECT_TIMEOUT}" MATCHES ^[0-9]+$)
      message(FATAL_ERROR "Invalid value of DMITIGR_PGFE_CONNECTION_CONNECT_TIMEOUT")
    endif()
  endif()

  if(NOT DEFINED DMITIGR_PGFE_CONNECTION_WAIT_RESPONSE_TIMEOUT)
    set(DMITIGR_PGFE_CONNECTION_WAIT_RESPONSE_TIMEOUT "" CACHE
      STRING "Integer (in milliseconds).")
  endif()
  if (NOT "${DMITIGR_PGFE_CONNECTION_WAIT_RESPONSE_TIMEOUT}" STREQUAL "")
    if(NOT "${DMITIGR_PGFE_CONNECTION_WAIT_RESPONSE_TIMEOUT}" MATCHES ^[0-9]+$)
      message(FATAL_ERROR "Invalid value of DMITIGR_PGFE_CONNECTION_WAIT_RESPONSE_TIMEOUT")
    endif()
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
  set(DMITIGR_PGFE_CONNECTION_COMMUNICATION_MODE "net")
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

if ((NOT DEFINED DMITIGR_PGFE_CONNECTION_NET_ADDRESS) AND
    (NOT DEFINED DMITIGR_PGFE_CONNECTION_NET_HOSTNAME))
  set(net_hostname_default_value "localhost")
endif()

if (NOT DEFINED DMITIGR_PGFE_CONNECTION_NET_ADDRESS)
  set(DMITIGR_PGFE_CONNECTION_NET_ADDRESS "" CACHE
    STRING "Numeric IP address of the host to connect to")
endif()
if (NOT "${DMITIGR_PGFE_CONNECTION_NET_ADDRESS}" STREQUAL "")
  if (NOT "${DMITIGR_PGFE_CONNECTION_NET_ADDRESS}" MATCHES ^[a-zA-Z0-9:]+[a-zA-Z0-9.:]+[a-zA-Z0-9]+$)
    message(FATAL_ERROR "Invalid value of DMITIGR_PGFE_CONNECTION_NET_ADDRESS")
  else()
    set(DMITIGR_PGFE_CONNECTION_NET_ADDRESS_CPP "\"${DMITIGR_PGFE_CONNECTION_NET_ADDRESS}\"")
  endif()
endif()

if (NOT DEFINED DMITIGR_PGFE_CONNECTION_NET_HOSTNAME)
  set(DMITIGR_PGFE_CONNECTION_NET_HOSTNAME "${net_hostname_default_value}" CACHE
    STRING "Name of the host to connect to")
endif()
if(NOT "${DMITIGR_PGFE_CONNECTION_NET_HOSTNAME}" STREQUAL "")
  if (NOT "${DMITIGR_PGFE_CONNECTION_NET_HOSTNAME}" MATCHES ^[a-zA-Z0-9._-]+$)
    message(FATAL_ERROR "Invalid value of DMITIGR_PGFE_CONNECTION_NET_HOSTNAME")
  else()
    set(DMITIGR_PGFE_CONNECTION_NET_HOSTNAME_CPP "\"${DMITIGR_PGFE_CONNECTION_NET_HOSTNAME}\"")
  endif()
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
# Preprocessing
# ------------------------------------------------------------------------------

if (UNIX)
  configure_file(${dmitigr_cpplipa_SOURCE_DIR}/src/pgfe/defaults.hpp.in
    ${dmitigr_cpplipa_SOURCE_DIR}/src/pgfe/defaults_unix.hpp
    @ONLY NEWLINE_STYLE UNIX)
elseif (WIN32)
  configure_file(${dmitigr_cpplipa_SOURCE_DIR}/src/pgfe/defaults.hpp.in
    ${dmitigr_cpplipa_SOURCE_DIR}/src/pgfe/defaults_windows.hpp
    @ONLY NEWLINE_STYLE UNIX)
endif()

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(dmitigr_pgfe_preprocessed_headers
  )

if (UNIX)
  list(APPEND dmitigr_pgfe_preprocessed_headers defaults_unix.hpp)
elseif (WIN32)
  list(APPEND dmitigr_pgfe_preprocessed_headers defaults_windows.hpp)
endif()

set(dmitigr_pgfe_headers
  array_conversions.hpp
  basic_conversions.hpp
  basics.hpp
  completion.hpp
  compositional.hpp
  composite.hpp
  connection.hpp
  connection_options.hpp
  connection_pool.hpp
  conversions_api.hpp
  conversions.hpp
  data.hpp
  errc.hpp
  error.hpp
  exceptions.hpp
  large_object.hpp
  message.hpp
  misc.hpp
  notice.hpp
  notification.hpp
  parameterizable.hpp
  pq.hpp
  prepared_statement.hpp
  problem.hpp
  response.hpp
  row.hpp
  row_info.hpp
  signal.hpp
  sql_string.hpp
  sql_vector.hpp
  std_system_error.hpp
  types_fwd.hpp
  )

set(dmitigr_pgfe_implementations
  completion.cpp
  connection.cpp
  connection_options.cpp
  connection_pool.cpp
  data.cpp
  errc.cpp
  large_object.cpp
  misc.cpp
  prepared_statement.cpp
  problem.cpp
  row_info.cpp
  sql_string.cpp
  sql_vector.cpp
  std_system_error.cpp
  )

set(dmitigr_pgfe_cmake_unpreprocessed
  defaults.hpp.in
  )

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

#
# libpq
#

if(Pq_ROOT)
  find_package(Pq REQUIRED)
  list(APPEND dmitigr_pgfe_target_include_directories_public "${Pq_INCLUDE_DIRS}")
  list(APPEND dmitigr_pgfe_target_include_directories_interface "${Pq_INCLUDE_DIRS}")
  list(APPEND dmitigr_pgfe_target_link_libraries_public ${Pq_LIBRARIES})
  list(APPEND dmitigr_pgfe_target_link_libraries_interface ${Pq_LIBRARIES})
else()
  find_package(PostgreSQL REQUIRED)
  list(APPEND dmitigr_pgfe_target_include_directories_public "${PostgreSQL_INCLUDE_DIRS}")
  list(APPEND dmitigr_pgfe_target_include_directories_interface "${PostgreSQL_INCLUDE_DIRS}")
  list(APPEND dmitigr_pgfe_target_link_libraries_public ${PostgreSQL_LIBRARIES})
  list(APPEND dmitigr_pgfe_target_link_libraries_interface ${PostgreSQL_LIBRARIES})
endif()

# ------------------------------------------------------------------------------
# Tests
# ------------------------------------------------------------------------------

if(DMITIGR_CPPLIPA_TESTS)
  set(dmitigr_pgfe_tests
    benchmark_array_client
    benchmark_array_server
    benchmark_sql_string_replace
    composite
    connection
    connection_deferrable
    connection-err_in_mid
    connection_options
    connection_pool
    connection-rows
    connection_ssl
    conversions
    conversions_online
    data
    hello_world
    pq_vs_pgfe
    ps
    row
    sql_string
    sql_vector
    )

  set(dmitigr_pgfe_tests_target_link_libraries dmitigr_base dmitigr_os dmitigr_str
    dmitigr_util)

  set(prefix ${dmitigr_cpplipa_SOURCE_DIR}/test/pgfe)
  add_custom_target(dmitigr_pgfe_copy_test_resources ALL
    COMMAND cmake -E copy_if_different
    "${prefix}/pgfe-unit-sql_vector.sql"
    "${dmitigr_cpplipa_resource_destination_dir}"
    )
  add_dependencies(dmitigr_pgfe_copy_test_resources
    dmitigr_cpplipa_create_resource_destination_dir)
endif()
