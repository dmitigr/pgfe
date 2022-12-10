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

#include <string_view>
#include <vector>

#define ASSERT DMITIGR_ASSERT

int main()
try {
  namespace pgfe = dmitigr::pgfe;
  using pgfe::Transaction_guard;

  // Prepare.
  auto conn = pgfe::test::make_connection();
  conn->connect();
  ASSERT(!conn->is_transaction_uncommitted());
  {
    Transaction_guard tg{*conn};
    ASSERT(conn->is_transaction_uncommitted());
  } // rollback
  ASSERT(!conn->is_transaction_uncommitted());

  {
    Transaction_guard tg{*conn, true};
    ASSERT(conn->is_transaction_uncommitted());
    {
      Transaction_guard tg{*conn, true, "p2"};
    } // rollback to savepoint p2
    ASSERT(conn->is_transaction_uncommitted());
  } // rollback
  ASSERT(!conn->is_transaction_uncommitted());

  {
    Transaction_guard tg{*conn};
    ASSERT(conn->is_transaction_uncommitted());
    ASSERT(!tg.is_subtransaction());
    {
      Transaction_guard tg{*conn};
      ASSERT(conn->is_transaction_uncommitted());
      ASSERT(tg.is_subtransaction());
      {
        Transaction_guard tg{*conn};
        ASSERT(conn->is_transaction_uncommitted());
        ASSERT(tg.is_subtransaction());
      } // rollback to savepoint pgfe_savepoint
      ASSERT(conn->is_transaction_uncommitted());
    }  // rollback to savepoint pgfe_savepoint
    ASSERT(conn->is_transaction_uncommitted());
  } // rollback
  ASSERT(!conn->is_transaction_uncommitted());

  {
    ASSERT(!conn->is_transaction_uncommitted());
    Transaction_guard tg{*conn, false};
    ASSERT(!conn->is_transaction_uncommitted());
  }

  {
    ASSERT(!conn->is_transaction_uncommitted());
    Transaction_guard tg{*conn};
    ASSERT(conn->is_transaction_uncommitted());
    tg.commit();
    ASSERT(!conn->is_transaction_uncommitted());
  }
  ASSERT(!conn->is_transaction_uncommitted());

  {
    ASSERT(!conn->is_transaction_uncommitted());
    Transaction_guard tg{*conn};
    ASSERT(conn->is_transaction_uncommitted());
    tg.commit_and_chain();
    ASSERT(conn->is_transaction_uncommitted());
  } // rollback
  ASSERT(!conn->is_transaction_uncommitted());

  ASSERT(conn->is_connected() && !conn->is_transaction_uncommitted());
  try {
    Transaction_guard t{*conn};
    {
      Transaction_guard st1{*conn};
      {
        Transaction_guard st2{*conn};
        st2.commit(); // release pgfe_savepoint
        ASSERT(conn->is_transaction_uncommitted());
      }
      st1.commit();
    }
    t.commit_and_chain();
    {
      Transaction_guard t{*conn};
      throw 1;
    }
  } catch (...) {
    ASSERT(!conn->is_connected() || !conn->is_transaction_uncommitted());
  }
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
