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
  pool = pgfe::Connection_pool::make(pool_size, test::connection_options().get());
  ASSERT(pool);
  ASSERT(pool->size() == pool_size);
  ASSERT(!pool->is_connected());
  pool->connect();
  ASSERT(pool->is_connected());

  pgfe::Connection* conn1p{};
  pgfe::Connection* conn2p{};
  pgfe::Connection* conn3p{};
  {
    auto conn1 = pool->connection();
    conn1p = conn1.connection();
    ASSERT(conn1);
    conn1->perform("select 1");
    conn1->for_each([](const auto* const row)
    {
      const int n = pgfe::to<int>(row->data(0));
      ASSERT(n == 1);
    });

    auto conn2 = pool->connection();
    conn2p = conn2.connection();
    ASSERT(conn2);
    conn2->perform("select 2");
    conn2->for_each([](const auto* const row)
    {
      const int n = pgfe::to<int>(row->data(0));
      ASSERT(n == 2);
    });

    auto conn3 = pool->connection();
    conn3p = conn3.connection();
    ASSERT(conn3);

    auto conn4 = pool->connection();
    ASSERT(!conn4);
    pool->disconnect();
    ASSERT(!pool->is_connected());
    ASSERT(conn1->is_connected());
    ASSERT(conn2->is_connected());
    ASSERT(conn3->is_connected());
  }
  ASSERT(!conn1p->is_connected());
  ASSERT(!conn2p->is_connected());
  ASSERT(!conn3p->is_connected());
}
