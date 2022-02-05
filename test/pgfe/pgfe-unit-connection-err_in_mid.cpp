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
