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
# Sources
# ------------------------------------------------------------------------------

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

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

if(Pq_ROOT)
  find_package(Pq REQUIRED)
  list(APPEND dmitigr_pgfe_target_include_directories_public "${Pq_INCLUDE_DIRS}")
  list(APPEND dmitigr_pgfe_target_include_directories_interface "${Pq_INCLUDE_DIRS}")
  list(APPEND dmitigr_pgfe_target_link_libraries_public ${Pq_LIBRARIES})
  list(APPEND dmitigr_pgfe_target_link_libraries_interface ${Pq_LIBRARIES})
else()
  find_package(PostgreSQL REQUIRED)
  list(APPEND dmitigr_pgfe_target_include_directories_public
    "${PostgreSQL_INCLUDE_DIRS}")
  list(APPEND dmitigr_pgfe_target_include_directories_interface
    "${PostgreSQL_INCLUDE_DIRS}")
  list(APPEND dmitigr_pgfe_target_link_libraries_public
    ${PostgreSQL_LIBRARIES})
  list(APPEND dmitigr_pgfe_target_link_libraries_interface
    ${PostgreSQL_LIBRARIES})
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
