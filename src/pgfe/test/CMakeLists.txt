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

add_custom_target(dmitigr_pgfe_copy_test_resources ALL
  COMMAND cmake -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/pgfe-unit-sql_vector.sql"
  "${dmitigr_cpplipa_resource_destination_dir}"
  )

dmitigr_cpplipa_propagate_tests_settings(pgfe)
