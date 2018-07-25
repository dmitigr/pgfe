// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/completion.hpp"
#include "dmitigr/pgfe/conversions.hpp"
#include "dmitigr/pgfe/exceptions.hpp"
#include "dmitigr/pgfe/row.hpp"
#include "dmitigr/pgfe/tests/unit.hpp"

int main(int argc, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace pgfe::tests;

  try {
    auto conn = pgfe::tests::make_connection();
    conn->connect();

    conn->perform("begin");
    ASSERT(conn->completion() && conn->completion()->operation_name() == "BEGIN");

    conn->perform("create or replace function provoke_err_in_mid(a_i integer)"
                  " returns integer"
                  " language plpgsql"
                  " as $f$"
                  " begin"
                  "   if a_i > 2 then"
                  "     raise exception 'error: % > 2', a_i;"
                  "   end if;"
                  " return a_i;"
                  " end;"
                  " $f$"
                  );
    ASSERT(conn->completion() && conn->completion()->operation_name() == "CREATE FUNCTION");

    bool rows_processed{};
    try {
      conn->execute("select provoke_err_in_mid(n) from generate_series(1,10) n");
      conn->for_each([&](const pgfe::Row* const row)
                     {
                       const auto n = pgfe::to<int>(row->data(0));
                       ASSERT(n < 3);
                       if (n > 1)
                         rows_processed = true;
                     });
    } catch (const pgfe::Server_exception& e) {
      // ok, expected.
      ASSERT(e.code() == pgfe::Server_errc::cp0_raise_exception);
      ASSERT(rows_processed);
    }
    ASSERT(conn->is_ready_for_async_request());
    ASSERT(!conn->is_awaiting_response());
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
