// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_SERVER_MESSAGE_HPP
#define DMITIGR_PGFE_SERVER_MESSAGE_HPP

#include "dmitigr/pgfe/message.hpp"

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief Represents an abstraction of PostgreSQL server
 * messages (either synchronous or asynchronous).
 */
class Server_message : public Message {
private:
  friend Response;
  friend Signal;

  Server_message() = default;
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_SERVER_MESSAGE_HPP
