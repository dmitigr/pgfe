// -*- C++ -*-
//
// Copyright 2022 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "pgfe-unit.hpp"

int main()
try {
  namespace pgfe = dmitigr::pgfe;

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
      DMITIGR_ASSERT(n < 3);
      if (n > 1)
        rows_processed = true;
    }, "select provoke_err_in_mid(n) from generate_series(1,10) n");
  } catch (const pgfe::Server_exception& e) {
    // ok, expected.
    DMITIGR_ASSERT(e.error().condition() == pgfe::Server_errc::cp0_raise_exception);
    DMITIGR_ASSERT(rows_processed);
  }
  DMITIGR_ASSERT(conn->is_ready_for_nio_request());
  DMITIGR_ASSERT(conn->is_ready_for_request());
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
