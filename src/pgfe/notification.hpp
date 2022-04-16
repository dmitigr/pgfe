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

#ifndef DMITIGR_PGFE_NOTIFICATION_HPP
#define DMITIGR_PGFE_NOTIFICATION_HPP

#include "dll.hpp"
#include "pq.hpp"
#include "signal.hpp"
#include "types_fwd.hpp"

#include <cstdint>
#include <memory>
#include <string_view>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief An unprompted (asynchronous) notification from a PostgreSQL server.
 *
 * @see The <a href="https://www.postgresql.org/docs/current/static/sql-notify.html">NOTIFY</a> SQL command.
 *
 * @see Notice.
 */
class Notification final : public Signal {
public:
  /// Constructs invalid instance.
  Notification() = default;

  /// Not copy-constructible.
  Notification(const Notification&) = delete;

  /// Move-constructible.
  Notification(Notification&&) = default;

  /// Not copy-assignable.
  Notification& operator=(const Notification&) = delete;

  /// Move-assignable.
  Notification& operator=(Notification&&) = default;

  /// @see Message::is_valid().
  DMITIGR_PGFE_API bool is_valid() const noexcept override;

  /**
   * @returns The identifier of the PostgreSQL server process that
   * produced this notification, or `0` if `!is_valid()`.
   */
  DMITIGR_PGFE_API std::int_fast32_t server_pid() const noexcept;

  /**
   * @returns The name of the notification channel (which might be any
   * identifier) of the PostgreSQL server that produced this notification,
   * or empty view if `!is_valid()`.
   */
  DMITIGR_PGFE_API std::string_view channel_name() const noexcept;

  /// @returns The payload data, or empty view if `!is_valid()`.
  DMITIGR_PGFE_API Data_view payload() const noexcept;

private:
  friend Connection;

  std::unique_ptr<PGnotify> pgnotify_;

  explicit DMITIGR_PGFE_API Notification(PGnotify* const pgnotify) noexcept;
  bool is_invariant_ok() const noexcept;
};

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "notification.cpp"
#endif

#endif  // DMITIGR_PGFE_NOTIFICATION_HPP
