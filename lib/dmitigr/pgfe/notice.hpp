// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_NOTICE_HPP
#define DMITIGR_PGFE_NOTICE_HPP

#include "dmitigr/pgfe/problem.hpp"
#include "dmitigr/pgfe/signal.hpp"

#include <string>
#include <system_error>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief Represents unprompted notices from the server.
 *
 * The notice is a information about the some activity of a PostgreSQL server.
 * (For example, it might be the database administrator's commands.)
 *
 * @remarks It should not be confused with the Notification signal.
 */
class Notice : public Signal, public Problem {
private:
  friend detail::iNotice;

  Notice() = default;
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_NOTICE_HPP
