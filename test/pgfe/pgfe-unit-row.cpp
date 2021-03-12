// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include "pgfe-unit.hpp"

namespace pgfe = dmitigr::pgfe;
namespace testo = dmitigr::testo;

int main(int, char* argv[])
try {
  auto conn = pgfe::test::make_connection();
  conn->connect();

  // ---------------------------------------------------------------------------
  // Row_info
  // ---------------------------------------------------------------------------

  conn->execute([](auto&& row)
  {
    ASSERT(row.info().name_of(0) == "thenumberone");
    ASSERT(row.info().name_of(1) == "theNumberOne");
    ASSERT(row.info().index_of("thenumberone") == 0);
    ASSERT(row.info().index_of("theNumberOne") == 1);
  }, R"(select 1::integer theNumberOne, 1::integer "theNumberOne")");

  // ---------------------------------------------------------------------------
  // Row
  // ---------------------------------------------------------------------------

  conn->execute([](auto&& row)
  {
    for (const auto& col : row) {
      std::cout << col.first << ": " << pgfe::to<std::string_view>(col.second) << std::endl;
    }
  }, R"(select 1::int4 one, 2::int4 two, 3::int4 three)");
} catch (const std::exception& e) {
  testo::report_failure(argv[0], e);
  return 1;
} catch (...) {
  testo::report_failure(argv[0]);
  return 1;
}
