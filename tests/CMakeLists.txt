# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

if(NOT DMITIGR_PGFE_BUILD_TESTS)
  return()
endif()

remove_definitions(-DDMITIGR_PGFE_DLL_BUILDING)

foreach(t
    hello_world unit-composite unit-data unit-problem unit-sql_string unit-sql_vector
    unit-conversions unit-connection_options unit-connection unit-connection-ssl
    unit-connection-deferrable unit-connection-err_in_mid unit-ps unit-conversions-online
    benchmark-sql_string_replace benchmark-array-client benchmark-array-server)
  set(test_sources ${t}.cpp unit.hpp)
  if("${t}" STREQUAL "unit-problem")
    list(APPEND test_sources
      ${dmitigr_pgfe_SOURCE_DIR}/lib/dmitigr/pgfe/problem.cpp
      ${dmitigr_pgfe_SOURCE_DIR}/lib/dmitigr/pgfe/sql.cxx)
  endif()
  add_executable(${t} ${test_sources})
  target_link_libraries(${t} PRIVATE ${dmitigr_pgfe_target} ${dmitigr_internal_library})
  dmitigr_target_compile_options(${t})
  add_test(NAME ${t} COMMAND ${t})
endforeach()

if (WIN32)
  set(dmitigr_pgfe_resource_destination_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>")
elseif (UNIX)
  set(dmitigr_pgfe_resource_destination_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

add_custom_target(copy_test_resources ALL
  COMMAND cmake -E make_directory "${dmitigr_pgfe_resource_destination_dir}"

  COMMAND cmake -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/unit-sql_vector.sql"
  "${dmitigr_pgfe_resource_destination_dir}")
