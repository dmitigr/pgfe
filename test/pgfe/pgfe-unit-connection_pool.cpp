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

namespace pgfe = dmitigr::pgfe;

int main()
try {
  constexpr std::size_t pool_size = 3;
  pgfe::Connection_pool pool{pool_size, pgfe::test::connection_options()};
  DMITIGR_ASSERT(pool.size() == pool_size);
  DMITIGR_ASSERT(!pool.is_connected());
  pool.connect();
  DMITIGR_ASSERT(pool.is_connected());

  pgfe::Connection* conn1p{};
  pgfe::Connection* conn2p{};
  pgfe::Connection* conn3p{};
  {
    auto conn1 = pool.connection();
    DMITIGR_ASSERT(conn1);
    conn1p = &*conn1;
    conn1->execute([](auto&& row)
    {
      const auto d = row.data();
      DMITIGR_ASSERT(d);
      const auto n = pgfe::to<int>(d);
      DMITIGR_ASSERT(n == 1);
    }, "select 1");

    auto conn2 = pool.connection();
    DMITIGR_ASSERT(conn2);
    conn2p = &*conn2;
    conn2->execute([](auto&& row)
    {
      const auto d = row.data();
      DMITIGR_ASSERT(d);
      const auto n = pgfe::to<int>(d);
      DMITIGR_ASSERT(n == 2);
    }, "select 2");

    auto conn3 = pool.connection();
    DMITIGR_ASSERT(conn3);
    conn3p = &*conn3;

    auto conn4 = pool.connection();
    DMITIGR_ASSERT(!conn4);
    pool.disconnect();
    DMITIGR_ASSERT(!pool.is_connected());
    DMITIGR_ASSERT(conn1->is_connected());
    DMITIGR_ASSERT(conn2->is_connected());
    DMITIGR_ASSERT(conn3->is_connected());
  }
  DMITIGR_ASSERT(conn1p);
  DMITIGR_ASSERT(conn2p);
  DMITIGR_ASSERT(conn3p);
  DMITIGR_ASSERT(!conn1p->is_connected());
  DMITIGR_ASSERT(!conn2p->is_connected());
  DMITIGR_ASSERT(!conn3p->is_connected());
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
