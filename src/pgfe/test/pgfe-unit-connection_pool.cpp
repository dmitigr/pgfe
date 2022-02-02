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
#include "../../pgfe.hpp"

namespace pgfe = dmitigr::pgfe;

inline std::unique_ptr<pgfe::Connection_pool> pool;

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
