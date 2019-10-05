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
 * @brief An unprompted (asynchronous) notice from a PostgreSQL server.
 *
 * The notice is an information about an activity of the PostgreSQL server.
 * (For example, it might be the database administrator's commands.)
 *
 * @remarks It should not be confused with the Notification signal.
 */
class Notice : public Signal, public Problem {
public:
  /// @name Conversions
  /// @{

  /**
   * @returns The copy of this instance.
   */
  virtual std::unique_ptr<Notice> to_notice() const = 0;

  /// @}

private:
  friend detail::iNotice;

  Notice() = default;
};

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/notice.cpp"
#endif

#endif  // DMITIGR_PGFE_NOTICE_HPP
