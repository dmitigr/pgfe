// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include <dmitigr/pgfe.hpp>
#include <cstdio>

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

  // Using Pgfe's conversion function.
  using pgfe::to;

  // Executing query with positional parameters.
  conn->execute("select generate_series($1::int, $2::int)", 1, 3);
  conn->for_each([](auto* r){ std::printf("Number %i\n", to<int>(r->data())); });

  // Prepare and execute the statement with named parameters.
  auto* ps = conn->prepare_statement("select :begin b, :end e");
  ps->set_parameter("begin", 0);
  ps->set_parameter("end", 1);
  ps->execute();
  ps->connection()->for_each([](auto* r) {
    std::printf("Range [%i, %i]\n", to<int>(r->data("b")), to<int>(r->data("e")));
  });

  // Invoking the function.
  conn->invoke("cos", .5f);
  conn->for_each([](auto* r){
    std::printf("cos(%f) = %f\n", .5f, to<float>(r->data()));
  });

  // Provoking the syntax error.
  conn->perform("provoke syntax error");
 } catch (const pgfe::c42_Syntax_error& e) {
  std::printf("Error %s is handled as expected.\n", e.error()->sqlstate().c_str());
 } catch (const std::exception& e) {
  std::printf("Oops: %s\n", e.what());
  return 1;
 }
