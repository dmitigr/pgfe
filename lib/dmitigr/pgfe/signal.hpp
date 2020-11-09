// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_SIGNAL_HPP
#define DMITIGR_PGFE_SIGNAL_HPP

#include "dmitigr/pgfe/message.hpp"

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief An asynchronous (unprompted) message from a PostgreSQL server.
 */
class Signal : public Message {
  friend Notice;
  friend Notification;

  Signal() = default;
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_SIGNAL_HPP
