// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "pgfe-unit.hpp"

#include <dmitigr/pgfe/completion.hpp>
#include <dmitigr/pgfe/conversions.hpp>
#include <dmitigr/pgfe/exceptions.hpp>
#include <dmitigr/pgfe/row.hpp>

int main(int, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace dmitigr::testo;

  try {
    auto conn = pgfe::test::make_connection();
    conn->connect();

    conn->perform("begin");
    auto comp = conn->wait_completion();
    ASSERT(comp && comp.operation_name() == "BEGIN");

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
    comp = conn->wait_completion();
    ASSERT(comp && comp.operation_name() == "CREATE FUNCTION");

    bool rows_processed{};
    try {
      conn->execute("select provoke_err_in_mid(n) from generate_series(1,10) n");
      while (const auto r = conn->wait_row()) {
        const auto n = pgfe::to<int>(r.data());
        ASSERT(n < 3);
        if (n > 1)
          rows_processed = true;
      }
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
