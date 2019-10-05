// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_RESPONSE_HPP
#define DMITIGR_PGFE_RESPONSE_HPP

#include "dmitigr/pgfe/server_message.hpp"

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief A synchronous (requested) message from a PostgreSQL server.
 */
class Response : public Server_message {
private:
  friend Completion;
  friend Error;
  friend Prepared_statement;
  friend Row;

  Response() = default;
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_RESPONSE_HPP
