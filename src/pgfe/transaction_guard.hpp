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

#ifndef DMITIGR_PGFE_TRANSACTION_GUARD_HPP
#define DMITIGR_PGFE_TRANSACTION_GUARD_HPP

#include "../base/assert.hpp"
#include "connection.hpp"
#include "statement.hpp"

#include <iostream>
#include <string>

namespace dmitigr::pgfe {

/// @brief A transaction guard.
class Transaction_guard final {
public:
  /// Not copy-constructible.
  Transaction_guard(const Transaction_guard&) = delete;
  /// Not copy-assignable.
  Transaction_guard& operator=(const Transaction_guard&) = delete;
  /// Not move-constructible.
  Transaction_guard(Transaction_guard&&) = delete;
  /// Not move-assignable.
  Transaction_guard& operator=(Transaction_guard&&) = delete;

  /**
   * @brief Cleanup the controlled resources.
   *
   * @details Attempts to rollback the uncommitted transaction. If that failed,
   * closes the controlled connection, since failed rollback might indicate a
   * total mess.
   */
  ~Transaction_guard()
  {
    try {
      try {
        rollback();
      } catch (...) {
        conn_.disconnect();
        throw;
      }
    } catch (const std::exception& e) {
      std::clog << "rollback error:" << e.what() << '\n';
    } catch (...) {
      std::clog << "rollback error: unknown error\n";
    }
  }

  /// Begins the transaction (or defines a savepoint) if `auto_begin`.
  Transaction_guard(Connection& conn,
    const bool auto_begin = true, std::string savepoint = {})
    : conn_{conn}
    , is_subtransaction_{conn_.is_transaction_uncommitted()}
    , savepoint_{std::move(savepoint)}
  {
    if (is_subtransaction_) {
      if (savepoint_.empty())
        savepoint_ = "pgfe_savepoint";
      rollback_stmt_ = R"(rollback to savepoint :"s")";
      rollback_stmt_.bind("s", savepoint_);
    } else
      rollback_stmt_ = "rollback";

    if (auto_begin)
      begin();
  }

  /// @returns `true` if this instance guards a subtransaction.
  bool is_subtransaction() const noexcept
  {
    return is_subtransaction_;
  }

  /// @returns A savepoint name.
  const std::string& savepoint() const noexcept
  {
    return savepoint_;
  }

  /// Begins a transaction (or opens savepoint) if it hasn't already begun.
  void begin()
  {
    if (!conn_.is_transaction_uncommitted())
      conn_.execute("begin");

    if (is_subtransaction_)
      conn_.execute(savepoint_stmt__(R"(savepoint :"s")"));
  }

  /**
   * @brief Commits the transaction (or destroys a savepoint) if the
   * transaction is uncommitted.
   */
  void commit()
  {
    commit__("commit");
  }

  /**
   * @brief Similar to commit().
   *
   * @details If `!is_subtransaction()`, immediately begins a new transaction
   * with the same transaction characteristics as the just committed one.
   */
  void commit_and_chain()
  {
    commit__("commit and chain");
  }

  /**
   * @brief Rollbacks the transaction (or savepoint if `is_subtransaction()`)
   * if the transaction is uncommitted.
   */
  void rollback()
  {
    if (conn_.is_transaction_uncommitted() && !is_subtransaction_committed_)
      conn_.execute(rollback_stmt_);
  }

private:
  Connection& conn_;
  bool is_subtransaction_{};
  bool is_subtransaction_committed_{};
  std::string savepoint_;
  Statement rollback_stmt_;

  Statement savepoint_stmt__(const std::string_view input) const
  {
    DMITIGR_ASSERT(!savepoint_.empty());
    return Statement{input}.bind("s", savepoint_);
  }

  void commit__(const Statement commit_query)
  {
    if (conn_.is_transaction_uncommitted()) {
      if (is_subtransaction_) {
        conn_.execute(savepoint_stmt__(R"(release :"s")"));
        is_subtransaction_committed_ = true;
      } else
        conn_.execute(commit_query);
    }
  }
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_TRANSACTION_GUARD_HPP
