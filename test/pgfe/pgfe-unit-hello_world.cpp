// -*- C++ -*-
// Copyright (C) 2022 Dmitry Igrishin
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// Dmitry Igrishin
// dmitigr@gmail.com

#include "../../src/pgfe/pgfe.hpp"

#include <cstdio>

namespace pgfe = dmitigr::pgfe;

int main() try {
  // Making the connection.
  pgfe::Connection conn{pgfe::Connection_options{pgfe::Communication_mode::net}
    .set_net_hostname("localhost")
    .set_database("pgfe_test")
    .set_username("pgfe_test")
    .set_password("pgfe_test")};

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
  auto& ps = conn.prepare("select $1::int i");
  for (int i{}; i < 3; ++i)
    ps.execute([](auto&& r){ std::printf("%i\n", to<int>(r["i"])); }, i);

  // Invoking the function.
  conn.invoke([](auto&& r)
  {
    std::printf("cos(%f) = %f\n", .5f, to<float>(r.data()));
  }, "cos", .5f);

  // Provoking the syntax error.
  conn.execute("provoke syntax error");
 } catch (const pgfe::Server_exception& e) {
  assert(e.error().condition() == pgfe::Server_errc::c42_syntax_error);
  std::printf("Error %s is handled as expected.\n", e.error().sqlstate());
 } catch (const std::exception& e) {
  std::printf("Oops: %s\n", e.what());
  return 1;
 }
