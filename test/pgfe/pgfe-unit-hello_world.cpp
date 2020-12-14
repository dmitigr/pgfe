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

  // Using Pgfe's helpers.
  using pgfe::a;  // for named arguments
  using pgfe::to; // for data conversions

  // Executing statement with positional parameters.
  conn.execute([](auto&& r)
  {
    std::printf("Number %i\n", to<int>(r.data()));
  }, "select generate_series($1::int, $2::int)", 1, 3);

  // Execute statement with named parameters.
  conn.execute([](auto&& r)
  {
    std::printf("Range [%i, %i]\n", to<int>(r["b"]), to<int>(r["e"]));
  },"select :begin b, :end e", a{"end", 1}, a{"begin", 0});

  // Prepare and execute the statement.
  auto* const ps = conn.prepare("select $1::int i");
  for (int i = 0; i < 3; ++i)
    ps->execute([](auto&& r){ std::printf("%i\n", to<int>(r["i"])); }, i);

  // Invoking the function.
  conn.invoke([](auto&& r)
  {
    std::printf("cos(%f) = %f\n", .5f, to<float>(r.data()));
  }, "cos", .5f);

  // Provoking the syntax error.
  conn.execute("provoke syntax error");
 } catch (const pgfe::c42_Syntax_error& e) {
  std::printf("Error %s is handled as expected.\n", e.error().sqlstate());
 } catch (const std::exception& e) {
  std::printf("Oops: %s\n", e.what());
  return 1;
 }
