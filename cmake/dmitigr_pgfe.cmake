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

# ------------------------------------------------------------------------------
# Info
# ------------------------------------------------------------------------------

dmitigr_libs_set_library_info(pgfe 2 1 2 "PostgreSQL client API")

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(dmitigr_pgfe_headers
  array_aliases.hpp
  array_conversions.hpp
  basic_conversions.hpp
  basics.hpp
  copier.hpp
  completion.hpp
  compositional.hpp
  composite.hpp
  connection.hpp
  connection_options.hpp
  connection_pool.hpp
  contract.hpp
  conversions_api.hpp
  conversions.hpp
  data.hpp
  errc.hpp
  errctg.hpp
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
  ready_for_query.hpp
  response.hpp
  row.hpp
  row_info.hpp
  signal.hpp
  statement.hpp
  statement_vector.hpp
  transaction_guard.hpp
  types_fwd.hpp
  )

set(dmitigr_pgfe_implementations
  copier.cpp
  completion.cpp
  composite.cpp
  compositional.cpp
  connection.cpp
  connection_options.cpp
  connection_pool.cpp
  data.cpp
  errc.cpp
  errctg.cpp
  error.cpp
  exceptions.cpp
  large_object.cpp
  misc.cpp
  notice.cpp
  notification.cpp
  parameterizable.cpp
  prepared_statement.cpp
  problem.cpp
  ready_for_query.cpp
  row.cpp
  row_info.cpp
  statement.cpp
  statement_vector.cpp
  tuple.cpp
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

if(DMITIGR_LIBS_TESTS)
  set(dmitigr_pgfe_tests
    array_dimension
    benchmark_array_client
    benchmark_array_server
    benchmark_statement_replace
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
    copier
    data
    exceptions
    hello_world
    pipeline
    pq_vs_pgfe
    ps
    lob
    row
    statement
    statement_vector
    transaction_guard
    )

  set(dmitigr_pgfe_tests_target_link_libraries dmitigr_base dmitigr_os dmitigr_str
    dmitigr_util)

  set(prefix ${dmitigr_libs_SOURCE_DIR}/test/pgfe)
  add_custom_target(dmitigr_pgfe_copy_test_resources ALL
    COMMAND cmake -E copy_if_different
    "${prefix}/pgfe-unit-statement_vector.sql"
    "${dmitigr_libs_resource_destination_dir}"
    )
  add_dependencies(dmitigr_pgfe_copy_test_resources
    dmitigr_libs_create_resource_destination_dir)
endif()
