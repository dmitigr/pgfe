// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_EXCEPTIONS_HPP
#define DMITIGR_PGFE_EXCEPTIONS_HPP

#include "dmitigr/pgfe/error.hpp"
#include "dmitigr/pgfe/std_system_error.hpp"
#include "dmitigr/pgfe/types_fwd.hpp"
#include <dmitigr/base/debug.hpp>

#include <memory>

namespace dmitigr::pgfe {

/**
 * @ingroup errors
 *
 * @brief The base class of exceptions thrown on a client side.
 */
class Client_exception : public std::system_error {
private:
  friend detail::iClient_exception;

  /**
   * @brief The constructor.
   */
  Client_exception(std::error_code ec) : system_error(ec)
  {}

  /**
   * @overload
   */
  Client_exception(std::error_code ec, const std::string& what) : system_error(ec, what)
  {}
};

/**
 * @ingroup errors
 *
 * @brief The base class of exceptions thrown on a server side.
 */
class Server_exception : public std::system_error {
public:
  /**
   * @returns The error response (aka error report).
   */
  virtual const Error* error() const noexcept = 0;

private:
  friend detail::iServer_exception;

  Server_exception(std::error_code ec) : system_error(ec)
  {}

  Server_exception(std::error_code ec, const std::string& what) : system_error(ec, what)
  {}
};

namespace detail {

/**
 * @brief The Client_exception implementation.
 */
class iClient_exception final : public Client_exception {
public:
  /**
   * @brief The constructor.
   */
  explicit iClient_exception(const Client_errc errc)
    : Client_exception(errc)
  {}

  /**
   * @overload
   */
  iClient_exception(const Client_errc errc, const std::string& what)
    : Client_exception(errc, what)
  {}
};

/**
 * @brief The Server_exception implementation.
 */
class iServer_exception final : public Server_exception {
public:
  /**
   * @brief The constructor.
   */
  explicit iServer_exception(std::shared_ptr<Error> error)
    : Server_exception(error->code())
    , error_(std::move(error))
  {
    DMITIGR_ASSERT(is_invariant_ok());
  }

  /**
   * @returns The error details.
   */
  const Error* error() const noexcept override
  {
    return error_.get();
  }

private:
  bool is_invariant_ok()
  {
    return static_cast<bool>(error_);
  }

  std::shared_ptr<Error> error_;
};

} // namespace detail
} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_EXCEPTIONS_HPP
