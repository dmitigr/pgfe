// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include "pgfe-unit.hpp"

namespace pgfe = dmitigr::pgfe;
namespace testo = dmitigr::testo;

int main(int, char* argv[])
try {
  auto conn = pgfe::test::make_ssl_connection();
  conn->connect();
  ASSERT(conn->is_ssl_secured());
  conn->execute([](auto&& row)
  {
    ASSERT(row[0]);
    ASSERT(pgfe::to<int>(row[0]) == 1);
  }, "select 1::int");
} catch (const std::exception& e) {
  // Only report a failure if a server supports SSL.
  if (std::string_view{e.what()}.find("not support SSL") == std::string_view::npos) {
    testo::report_failure(argv[0], e);
    return 1;
  }
} catch (...) {
  testo::report_failure(argv[0]);
  return 1;
}
