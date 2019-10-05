// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "pgfe-unit.hpp"

#include <dmitigr/pgfe/completion.hpp>
#include <dmitigr/pgfe/exceptions.hpp>

int main(int, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace dmitigr::test;

  try {
    auto conn = pgfe::test::make_connection();
    conn->connect();

    conn->perform("begin");
    ASSERT(conn->completion() && conn->completion()->operation_name() == "BEGIN");

    conn->perform_async
    ("create table test(id integer not null);"

     "create function test_constraint()"
     " returns trigger"
     " language plpgsql"
     " as $f$"
     " begin"
     " raise 'test: constraint violation';"
     " end;"
     " $f$;"

     "create constraint trigger test_constraint"
     " after insert or update or delete on test"
     " deferrable initially deferred"
     " for each row"
     " execute procedure test_constraint()");
    conn->wait_last_response_throw();
    ASSERT(conn->completion() && conn->completion()->operation_name() == "CREATE TRIGGER");

    conn->execute("insert into test(id) values($1)", 1);
    ASSERT(conn->completion()->operation_name() == "INSERT");

    try {
      conn->perform("commit");
    } catch (const pgfe::Server_exception& e) {
      // ok, expected.
      ASSERT(e.code() == pgfe::Server_errc::cp0_raise_exception);
    }
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
