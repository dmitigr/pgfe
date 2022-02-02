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

#include "pgfe-unit.hpp"
#include "../../pgfe.hpp"

int main()
try {
  namespace pgfe = dmitigr::pgfe;

  auto conn = pgfe::test::make_connection();
  conn->connect();

  conn->execute("begin");
  conn->execute("create table test(id integer not null);");
  conn->execute(
    "create function test_constraint()"
    " returns trigger"
    " language plpgsql"
    " as $f$"
    " begin"
    " raise 'test: constraint violation';"
    " end;"
    " $f$;");
  conn->execute(
    "create constraint trigger test_constraint"
    " after insert or update or delete on test"
    " deferrable initially deferred"
    " for each row"
    " execute procedure test_constraint()");
  conn->execute("insert into test(id) values($1)", 1);

  try {
    conn->execute("commit");
  } catch (const pgfe::Server_exception& e) {
    // ok, expected.
    DMITIGR_ASSERT(e.error().condition() == pgfe::Server_errc::cp0_raise_exception);
  }
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
