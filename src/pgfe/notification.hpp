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
 * @remarks It should not be confused with the Notice signal.
 *
 * @see The <a href="https://www.postgresql.org/docs/current/static/sql-notify.html">NOTIFY</a> SQL command.
 */
class Notification final : public Signal {
public:
  /// Constructs invalid instance.
  Notification() = default;

  /// Non copy-constructible.
  Notification(const Notification&) = delete;

  /// Move-constructible.
  Notification(Notification&&) = default;

  /// Non copy-assignable.
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

  std::unique_ptr< ::PGnotify> pgnotify_;

  explicit DMITIGR_PGFE_API Notification(::PGnotify* const pgnotify);
  bool is_invariant_ok() const noexcept;
};

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "notification.cpp"
#endif

#endif  // DMITIGR_PGFE_NOTIFICATION_HPP
