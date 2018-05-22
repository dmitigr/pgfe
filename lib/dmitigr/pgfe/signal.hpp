// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_SIGNAL_HPP
#define DMITIGR_PGFE_SIGNAL_HPP

#include "dmitigr/pgfe/server_message.hpp"

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief Represents an abstraction of asynchronous
 * (unprompted) messages from PostgreSQL server.
 */
class Signal : public Server_message {
private:
  friend Notice;
  friend Notification;

  Signal() = default;
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_SIGNAL_HPP
