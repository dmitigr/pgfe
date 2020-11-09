// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_NOTIFICATION_HPP
#define DMITIGR_PGFE_NOTIFICATION_HPP

#include "dmitigr/pgfe/data.hpp"
#include "dmitigr/pgfe/pq.hpp"
#include "dmitigr/pgfe/signal.hpp"

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
    : pgnotify_{pgnotify}
    , payload_{}
    , channel_name_{pgnotify_->relname}
  {
    if (pgnotify_->extra)
      payload_ = Data_view{pgnotify_->extra,
        static_cast<int>(std::strlen(pgnotify_->extra)), Data_format::text};

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
  const std::string& channel_name() const noexcept
  {
    return channel_name_;
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
  std::string channel_name_;

  Notification() = default;

  bool is_invariant_ok() const noexcept
  {
    const bool server_pid_ok = server_pid() >= 0;
    const bool pgnotify_ok = pgnotify_ && (!payload_ || (payload_ && pgnotify_->extra == payload_.bytes()));
    const bool channel_ok = !channel_name_.empty();
    return server_pid_ok && pgnotify_ok && channel_ok;
  }
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_NOTIFICATION_HPP
