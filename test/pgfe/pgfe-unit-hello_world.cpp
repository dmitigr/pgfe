// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include <dmitigr/pgfe.hpp>
#include <iostream>

namespace pgfe = dmitigr::pgfe;

int main() try {
  // Making the connection.
  const auto conn = pgfe::Connection_options::make(pgfe::Communication_mode::net)->
    set_net_hostname("localhost")->
    set_database("pgfe_test")->
    set_username("pgfe_test")->
    set_password("pgfe_test")->
    make_connection();

  // Connecting.
  conn->connect();

  // Executing the query (positional parameters).
  conn->execute("select generate_series($1::int, $2::int)", 1, 3);
  conn->for_each([](auto* row){ std::cout << pgfe::to<int>(row->data()) << "\n"; });

  // Invoking the function.
  conn->invoke("current_database");
  conn->for_each([](auto* row){ std::cout << pgfe::to<std::string>(row->data()) << "\n"; });

  // Provoking the syntax error.
  conn->perform("provoke syntax error");
 } catch (const pgfe::c42_Syntax_error& e) {
  std::cout << "Error " << e.error()->sqlstate() << " is handled as expected.\n";
 } catch (const std::exception& e) {
  std::cerr << "Oops: " << e.what() << std::endl;
  return 1;
 }
