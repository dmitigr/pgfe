// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "unit.hpp"

#include "dmitigr/pgfe/completion.hpp"

int main(int, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace pgfe::tests;

  try {
    auto conn = pgfe::tests::make_ssl_connection();
    conn->connect();
    ASSERT(conn->is_ssl_secured());
    conn->perform("begin");
    ASSERT(conn->completion() && conn->completion()->operation_name() == "BEGIN");
    conn->perform("commit");
    ASSERT(conn->completion() && conn->completion()->operation_name() == "COMMIT");
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
