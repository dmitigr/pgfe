// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "pgfe-unit.hpp"

#include <dmitigr/pgfe/completion.hpp>
#include <dmitigr/pgfe/row.hpp>
#include <dmitigr/pgfe/row_info.hpp>

int main(int, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace dmitigr::test;

  try {
    auto conn = pgfe::test::make_connection();
    conn->connect();

    conn->perform(R"(select 1::integer theNumberOne, 1::integer "theNumberOne")");
    ASSERT(conn->row());
    ASSERT(conn->row()->info()->field_name(0) == "thenumberone");
    ASSERT(conn->row()->info()->field_name(1) == "theNumberOne");
    ASSERT(conn->row()->info()->field_index("thenumberone") == 0);
    ASSERT(conn->row()->info()->field_index("theNumberOne") == 1);
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
