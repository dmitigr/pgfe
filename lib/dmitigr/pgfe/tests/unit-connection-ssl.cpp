// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/completion.hpp"
#include "dmitigr/pgfe/tests/unit.hpp"

int main(int argc, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace pgfe::tests;

  try {
    auto conn = pgfe::tests::make_ssl_connection();
    conn->connect();
    assert(conn->is_ssl_secured());
    conn->perform("begin");
    assert(conn->completion() && conn->completion()->operation_name() == "BEGIN");
    conn->perform("commit");
    assert(conn->completion() && conn->completion()->operation_name() == "COMMIT");
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
