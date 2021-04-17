// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_EXCEPTIONS_HPP
#define DMITIGR_PGFE_EXCEPTIONS_HPP

#include "dmitigr/pgfe/dll.hpp"
#include "dmitigr/pgfe/error.hpp"
#include "dmitigr/pgfe/std_system_error.hpp"
#include "dmitigr/pgfe/types_fwd.hpp"

#include <memory>

namespace dmitigr::pgfe {

// -----------------------------------------------------------------------------
// Client_exception
// -----------------------------------------------------------------------------

/**
 * @ingroup errors
 *
 * @brief The base class of exceptions thrown on a client side.
 */
class Client_exception : public std::exception {
public:
  /// The constructor.
  explicit Client_exception(const Client_errc errc, std::string what = {})
    : condition_{errc}
    , what_holder_{what.empty() ? to_literal(errc) :
    what.append(" (").append(to_literal(errc)).append(")")}
  {}

  /// @returns The error condition.
  const std::error_condition& condition() const noexcept
  {
    return condition_;
  }

  /// @returns An explanatory string.
  const char* what() const noexcept override
  {
    return what_holder_.what();
  }

private:
  std::error_condition condition_;
  std::runtime_error what_holder_;
};

// -----------------------------------------------------------------------------
// Server_exception
// -----------------------------------------------------------------------------

/**
 * @ingroup errors
 *
 * @brief The base class of exceptions thrown on a server side.
 */
class Server_exception : public std::exception {
public:
  /// The constructor.
  explicit Server_exception(std::shared_ptr<Error>&& error)
    : error_{std::move(error)}
  {
    assert(error_);
  }

  /// @returns The error response (aka error report).
  const Error& error() const noexcept
  {
    return *error_;
  }

  /// @returns The brief human-readable description of the error.
  const char* what() const noexcept override
  {
    return error_->brief();
  }

private:
  std::shared_ptr<Error> error_;
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_EXCEPTIONS_HPP
