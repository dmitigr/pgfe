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

#include "contract.hpp"
#include "data.hpp"
#include "pq.hpp"
#include "signal.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>

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
  /// The constructor.
  explicit Notification(::PGnotify* const pgnotify)
    : pgnotify_{detail::not_false(pgnotify)}
    , payload_{pgnotify_->extra}
  {
    assert(is_invariant_ok());
  }

  /// Non copy-constructible.
  Notification(const Notification&) = delete;

  /// Non copy-assignable.
  Notification& operator=(const Notification&) = delete;

  /// Move-constructible.
  Notification(Notification&&) = default;

  /// Move-assignable.
  Notification& operator=(Notification&&) = default;

  /// @see Message::is_valid().
  bool is_valid() const noexcept override
  {
    return static_cast<bool>(pgnotify_);
  }

  /**
   * @returns The identifier of the PostgreSQL server process that
   * produced this notification.
   */
  std::int_fast32_t server_pid() const noexcept
  {
    return pgnotify_->be_pid;
  }

  /**
   * @returns The name of the notification channel (which might be
   * any identifier) of the PostgreSQL server that produced this notification.
   */
  std::string_view channel_name() const noexcept
  {
    return pgnotify_->relname;
  }

  /// @returns The payload data.
  Data_view payload() const noexcept
  {
    return payload_;
  }

private:
  friend Connection;

  std::unique_ptr< ::PGnotify> pgnotify_;
  Data_view payload_;

  Notification() = default;

  bool is_invariant_ok() const noexcept
  {
    const bool server_pid_ok{server_pid() >= 0};
    const bool pgnotify_ok{pgnotify_ &&
      (!payload_ || (payload_ && pgnotify_->extra == payload_.bytes()))};
    const bool channel_ok{!channel_name().empty()};
    return server_pid_ok && pgnotify_ok && channel_ok;
  }
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_NOTIFICATION_HPP
