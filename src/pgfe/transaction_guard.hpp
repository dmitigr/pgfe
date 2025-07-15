// -*- C++ -*-
//
// Copyright 2025 Dmitry Igrishin
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
      rollback();
    } catch (...) {
      conn_.disconnect();
    }
  }

  /// Begins the transaction (or defines a savepoint).
  explicit Transaction_guard(Connection& conn)
    : Transaction_guard{conn, std::string{}}
  {}

  /// @overload
  Transaction_guard(Connection& conn, std::string savepoint)
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

  /// Begins a transaction (or opens savepoint) if `!has_begun()`.
  void begin()
  {
    if (!has_begun_) {
      if (is_subtransaction_)
        conn_.execute(savepoint_stmt__(R"(savepoint :"s")"));
      else
        conn_.execute("begin");
      has_begun_ = true;
    }
  }

  /// @returns `true` if a transaction guarded by this instance has begun.
  bool has_begun() const noexcept
  {
    return has_begun_;
  }

  /// Commits the transaction (or destroys a savepoint) if `has_begun()`.
  void commit()
  {
    commit__(commit_stmt_);
  }

  /**
   * @brief Similar to commit().
   *
   * @details Immediately begins a new (sub-)transaction with the same
   * transaction characteristics as the just committed one.
   */
  void commit_and_chain()
  {
    commit__(commit_and_chain_stmt_);
    if (is_subtransaction_)
      begin();
    has_begun_ = true;
  }

  /**
   * @brief Rollbacks the transaction (or savepoint if `is_subtransaction()`)
   * if `has_begun()`.
   */
  void rollback()
  {
    if (has_begun_) {
      conn_.execute(rollback_stmt_);
      has_begun_ = false;
    }
  }

private:
  Connection& conn_;
  bool is_subtransaction_{};
  bool has_begun_{};
  std::string savepoint_;
  Statement rollback_stmt_;
  inline static Statement commit_stmt_{"commit"};
  inline static Statement commit_and_chain_stmt_{"commit and chain"};

  Statement savepoint_stmt__(const std::string_view input) const
  {
    DMITIGR_ASSERT(!savepoint_.empty());
    return Statement{input}.bind("s", savepoint_);
  }

  void commit__(const Statement& commit_query)
  {
    if (has_begun_) {
      if (is_subtransaction_)
        conn_.execute(savepoint_stmt__(R"(release :"s")"));
      else
        conn_.execute(commit_query);
      has_begun_ = false;
    }
  }
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_TRANSACTION_GUARD_HPP
