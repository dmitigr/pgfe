// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include <dmitigr/pgfe.hpp>
#include <cstdio>

namespace pgfe = dmitigr::pgfe;

int main() try {
  // Making the connection.
  pgfe::Connection conn{pgfe::Connection_options{pgfe::Communication_mode::net}
    .net_hostname("localhost").database("pgfe_test")
    .username("pgfe_test").password("pgfe_test")};

  // Connecting.
  conn.connect();

  // Using Pgfe's conversion function.
  using pgfe::to;

  // Executing query with positional parameters.
  conn.execute([](auto&& r)
  {
    std::printf("Number %i\n", to<int>(r.data()));
  }, "select generate_series($1::int, $2::int)", 1, 3);

  // Prepare and execute the statement with named parameters.
  conn.prepare("select :begin b, :end e")
    ->bind("begin", 0).bind("end", 1).execute([](auto&& r)
  {
    std::printf("Range [%i, %i]\n", to<int>(r["b"]), to<int>(r["e"]));
  });

  // Invoking the function.
  conn.invoke([](auto&& r)
  {
    std::printf("cos(%f) = %f\n", .5f, to<float>(r.data()));
  }, "cos", .5f);

  // Provoking the syntax error.
  conn.perform("provoke syntax error");
 } catch (const pgfe::c42_Syntax_error& e) {
  std::printf("Error %s is handled as expected.\n", e.error().sqlstate());
 } catch (const std::exception& e) {
  std::printf("Oops: %s\n", e.what());
  return 1;
 }
