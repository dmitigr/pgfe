// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_NOTIFICATION_HPP
#define DMITIGR_PGFE_NOTIFICATION_HPP

#include "dmitigr/pgfe/signal.hpp"

#include <cstdint>
#include <string>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief Represents unprompted notifications from the server.
 *
 * @remarks It should not be confused with the Notice signal.
 *
 * @see The <a href="https://www.postgresql.org/docs/current/static/sql-notify.html">NOTIFY</a> SQL command.
 */
class Notification : public Signal {
public:
  /**
   * @returns The identifier of the server process that produced this notification.
   */
  virtual std::int_fast32_t server_pid() const noexcept = 0;

  /**
   * @returns The name of the notification channel (which might be any identifier)
   * of the server that produced this notification.
   */
  virtual const std::string& channel_name() const noexcept = 0;

  /**
   * @returns The payload data.
   */
  virtual const Data* payload() const noexcept = 0;

private:
  friend detail::iNotification;

  Notification() = default;
};

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/notification.cpp"
#endif

#endif  // DMITIGR_PGFE_NOTIFICATION_HPP
