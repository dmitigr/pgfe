# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

set(dmitigr_pgfe_tests
  # benchmark_array_client
  # benchmark_array_server
  # benchmark_sql_string_replace
  composite
  # connection
  # connection_deferrable
  # connection-err_in_mid
  connection_options
  # connection_pool
  # connection-rows
  # connection_ssl
  conversions
  # conversions_online data
  hello_world
  pq_vs_pgfe
  # ps
  # row_info
  sql_string
  # sql_vector
  )

set(dmitigr_pgfe_tests_target_link_libraries dmitigr::os dmitigr::str)

add_custom_target(dmitigr_pgfe_copy_test_resources ALL
  COMMAND cmake -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/pgfe-unit-sql_vector.sql"
  "${dmitigr_cefeika_resource_destination_dir}"
  )

dmitigr_propagate_tests_settings(pgfe)
