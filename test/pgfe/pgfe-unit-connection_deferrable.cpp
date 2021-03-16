// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include "pgfe-unit.hpp"

#include <dmitigr/pgfe.hpp>

namespace pgfe = dmitigr::pgfe;
namespace testo = dmitigr::testo;

int main(int, char* argv[])
try {
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
    ASSERT(e.error().condition() == pgfe::Server_errc::cp0_raise_exception);
  }
} catch (const std::exception& e) {
  testo::report_failure(argv[0], e);
  return 1;
} catch (...) {
  testo::report_failure(argv[0]);
  return 1;
}
