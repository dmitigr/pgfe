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
  conn->execute("create or replace function provoke_err_in_mid(a_i integer)"
    " returns integer"
    " language plpgsql"
    " as $f$"
    " begin"
    "   if a_i > 2 then"
    "     raise exception 'error: % > 2', a_i;"
    "   end if;"
    " return a_i;"
    " end;"
    " $f$");

  bool rows_processed{};
  try {
    conn->execute([&rows_processed](auto&& row)
    {
      const auto n = pgfe::to<int>(row[0]);
      ASSERT(n < 3);
      if (n > 1)
        rows_processed = true;
    }, "select provoke_err_in_mid(n) from generate_series(1,10) n");
  } catch (const pgfe::Server_exception& e) {
    // ok, expected.
    ASSERT(e.code() == pgfe::Server_errc::cp0_raise_exception);
    ASSERT(rows_processed);
  }
  ASSERT(conn->is_ready_for_nio_request());
  ASSERT(conn->is_ready_for_request());
} catch (const std::exception& e) {
  testo::report_failure(argv[0], e);
  return 1;
} catch (...) {
  testo::report_failure(argv[0]);
  return 1;
}
