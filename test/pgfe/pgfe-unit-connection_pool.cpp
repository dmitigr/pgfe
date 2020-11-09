// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "pgfe-unit.hpp"

#include <dmitigr/pgfe/connection_pool.hpp>

namespace pgfe = dmitigr::pgfe;
namespace test = pgfe::test;

std::unique_ptr<pgfe::Connection_pool> pool;

int main()
{
  constexpr std::size_t pool_size = 3;
  pgfe::Connection_pool pool{pool_size, test::connection_options()};
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
    conn1->perform("select 1");
    {
      const auto r = conn1->wait_row();
      ASSERT(r);
      const auto d = r.data();
      ASSERT(d);
      const auto n = pgfe::to<int>(d);
      ASSERT(n == 1);
    };

    auto conn2 = pool.connection();
    ASSERT(conn2);
    conn2p = &*conn2;
    conn2->perform("select 2");
    {
      const auto r = conn2->wait_row();
      ASSERT(r);
      const auto d = r.data();
      ASSERT(d);
      const auto n = pgfe::to<int>(d);
      ASSERT(n == 2);
    };

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
  ASSERT(!conn1p->is_connected());
  ASSERT(!conn2p->is_connected());
  ASSERT(!conn3p->is_connected());
}
