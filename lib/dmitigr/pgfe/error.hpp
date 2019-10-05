// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_ERROR_HPP
#define DMITIGR_PGFE_ERROR_HPP

#include "dmitigr/pgfe/problem.hpp"
#include "dmitigr/pgfe/response.hpp"

#include <string>
#include <system_error>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief A error message from a PostgreSQL server.
 */
class Error : public Response, public Problem {
public:
  /// @name Conversions
  /// @{

  /**
   * @returns The copy of this instance.
   */
  virtual std::unique_ptr<Error> to_error() const = 0;

  /// @}

private:
  friend detail::iError;

  Error() = default;
};

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/error.cpp"
#endif

#endif  // DMITIGR_PGFE_ERROR_HPP
