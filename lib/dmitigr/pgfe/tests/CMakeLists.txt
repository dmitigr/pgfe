# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

if(NOT PGFE_BUILD_TESTS)
  return()
endif()

remove_definitions(-DDMITIGR_PGFE_DLL_BUILDING)

# ------------------------------------------------------------------------------

set(libraries_to_link dmitigr_pgfe ${dmitigr_internal_library})
add_executable(hello_world hello_world.cpp)
target_link_libraries(hello_world PRIVATE ${libraries_to_link})

add_executable(test-unit-composite unit-composite.cpp unit.hpp)
target_link_libraries(test-unit-composite PRIVATE ${libraries_to_link})

add_executable(test-unit-data unit-data.cpp unit.hpp)
target_link_libraries(test-unit-data PRIVATE ${libraries_to_link})

add_executable(test-unit-problem unit-problem.cpp ../problem.cpp ../sql.cxx unit.hpp)
target_link_libraries(test-unit-problem PRIVATE ${libraries_to_link})

add_executable(test-unit-sql_string unit-sql_string.cpp unit.hpp)
target_link_libraries(test-unit-sql_string PRIVATE ${libraries_to_link})

add_executable(test-unit-sql_vector unit-sql_vector.cpp unit.hpp)
target_link_libraries(test-unit-sql_vector PRIVATE ${libraries_to_link})

add_executable(test-unit-conversions unit-conversions.cpp unit.hpp)
target_link_libraries(test-unit-conversions PRIVATE ${libraries_to_link})

add_executable(test-unit-connection_options unit-connection_options.cpp unit.hpp)
target_link_libraries(test-unit-connection_options PRIVATE ${libraries_to_link})

add_executable(test-unit-connection unit-connection.cpp unit.hpp)
target_link_libraries(test-unit-connection PRIVATE ${libraries_to_link})

add_executable(test-unit-connection-ssl unit-connection-ssl.cpp unit.hpp)
target_link_libraries(test-unit-connection-ssl PRIVATE ${libraries_to_link})

add_executable(test-unit-connection-deferrable unit-connection-deferrable.cpp unit.hpp)
target_link_libraries(test-unit-connection-deferrable PRIVATE ${libraries_to_link})

add_executable(test-unit-connection-err_in_mid unit-connection-err_in_mid.cpp unit.hpp)
target_link_libraries(test-unit-connection-err_in_mid PRIVATE ${libraries_to_link})

add_executable(test-unit-ps unit-ps.cpp unit.hpp)
target_link_libraries(test-unit-ps PRIVATE ${libraries_to_link})

add_executable(test-unit-conversions-online unit-conversions-online.cpp unit.hpp)
target_link_libraries(test-unit-conversions-online PRIVATE ${libraries_to_link})

add_executable(test-benchmark-sql_string_replace benchmark-sql_string_replace.cpp unit.hpp)
target_link_libraries(test-benchmark-sql_string_replace PRIVATE ${libraries_to_link})

add_executable(test-benchmark-array-client benchmark-array-client.cpp unit.hpp)
target_link_libraries(test-benchmark-array-client PRIVATE ${libraries_to_link})

add_executable(test-benchmark-array-server benchmark-array-server.cpp unit.hpp)
target_link_libraries(test-benchmark-array-server PRIVATE ${libraries_to_link})

# ------------------------------------------------------------------------------
if (WIN32)
  set(PGFE_RESOURCE_DESTINATION_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>")
elseif (UNIX)
  set(PGFE_RESOURCE_DESTINATION_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

add_custom_target(copy_test_resources ALL
  COMMAND cmake -E make_directory "${PGFE_RESOURCE_DESTINATION_DIR}"

  COMMAND cmake -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/unit-sql_vector.sql"
  "${PGFE_RESOURCE_DESTINATION_DIR}")
# ------------------------------------------------------------------------------

add_test(NAME hello_world COMMAND hello_world)
add_test(NAME unit-composite COMMAND test-unit-composite)
add_test(NAME unit-data COMMAND test-unit-data)
add_test(NAME unit-problem COMMAND test-unit-problem)
add_test(NAME unit-sql_string COMMAND test-unit-sql_string)
add_test(NAME unit-sql_vector COMMAND test-unit-sql_vector)
add_test(NAME unit-conversions COMMAND test-unit-conversions)
add_test(NAME unit-connection_options COMMAND test-unit-connection_options)
add_test(NAME unit-connection COMMAND test-unit-connection)
add_test(NAME unit-connection-ssl COMMAND test-unit-connection-ssl)
add_test(NAME unit-connection-deferrable COMMAND test-unit-connection-deferrable)
add_test(NAME unit-connection-err_in_mid COMMAND test-unit-connection-err_in_mid)
add_test(NAME unit-ps COMMAND test-unit-ps)
add_test(NAME unit-conversions-online COMMAND test-unit-conversions-online)
add_test(NAME benchmark-sql_string_replace COMMAND test-benchmark-sql_string_replace)
add_test(NAME benchmark-array-client COMMAND test-benchmark-array-client)
add_test(NAME benchmark-array-server COMMAND test-benchmark-array-server)
