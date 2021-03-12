// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include "pgfe-unit.hpp"

#include <dmitigr/pgfe.hpp>

namespace pgfe = dmitigr::pgfe;
namespace testo = dmitigr::testo;

inline std::unique_ptr<pgfe::Connection_pool> pool;

int main(const int, const char* const argv[])
try {
  constexpr std::size_t pool_size = 3;
  pgfe::Connection_pool pool{pool_size, pgfe::test::connection_options()};
  ASSERT(pool.size() == pool_size);
  ASSERT(!pool.is_connected());
  pool.connect();
  ASSERT(pool.is_connected());

  pgfe::Connection* conn1p{};
  pgfe::Connection* conn2p{};
  pgfe::Connection* conn3p{};
  {
    auto conn1 = pool.connection();
    ASSERT(conn1);
    conn1p = &*conn1;
    conn1->execute([](auto&& row)
    {
      const auto d = row.data();
      ASSERT(d);
      const auto n = pgfe::to<int>(d);
      ASSERT(n == 1);
    }, "select 1");

    auto conn2 = pool.connection();
    ASSERT(conn2);
    conn2p = &*conn2;
    conn2->execute([](auto&& row)
    {
      const auto d = row.data();
      ASSERT(d);
      const auto n = pgfe::to<int>(d);
      ASSERT(n == 2);
    }, "select 2");

    auto conn3 = pool.connection();
    ASSERT(conn3);
    conn3p = &*conn3;

    auto conn4 = pool.connection();
    ASSERT(!conn4);
    pool.disconnect();
    ASSERT(!pool.is_connected());
    ASSERT(conn1->is_connected());
    ASSERT(conn2->is_connected());
    ASSERT(conn3->is_connected());
  }
  ASSERT(conn1p);
  ASSERT(conn2p);
  ASSERT(conn3p);
  ASSERT(!conn1p->is_connected());
  ASSERT(!conn2p->is_connected());
  ASSERT(!conn3p->is_connected());
} catch (const std::exception& e) {
  testo::report_failure(argv[0], e);
  return 1;
} catch (...) {
  testo::report_failure(argv[0]);
  return 1;
}
