// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_MESSAGE_HPP
#define DMITIGR_PGFE_MESSAGE_HPP

#include "dmitigr/pgfe/types_fwd.hpp"

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief Represents an abstraction of client/server
 * messages to/from the PostgreSQL server.
 */
class Message {
public:
  virtual ~Message() = default;

private:
  friend Server_message;

  Message() = default;
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_MESSAGE_HPP
