// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or net.hpp

#ifndef DMITIGR_NET_EXCEPTIONS_HPP
#define DMITIGR_NET_EXCEPTIONS_HPP

#include "dmitigr/net/dll.hpp"
#include "dmitigr/os/exceptions.hpp"

namespace dmitigr::net {

#ifdef _WIN32
/**
 * @brief A category of Windows Socket Application (WSA) errors.
 */
class Wsa_error_category final : public std::error_category {
public:
  /**
   * @returns The literal `dmitigr_wsa_error`.
   */
  const char* name() const noexcept override;

  /**
   * @returns The string that describes the error condition denoted by `ev`.
   *
   * @remarks The caller should not rely on the
   * return value since it is a subject to change.
   */
  std::string message(int ev) const override;
};

/**
 * @returns The reference to instance of type Wsa_error_category.
 */
DMITIGR_NET_API const Wsa_error_category& wsa_error_category() noexcept;

/**
 * @brief An exception thrown on Windows Socket Application (WSA) error.
 */
class Wsa_exception final : public std::system_error {
public:
  /**
   * @brief The constructor.
   */
  DMITIGR_NET_API explicit Wsa_exception(const std::string& func);

  /**
   * @brief Prints the last WSA error to the standard error.
   */
  static DMITIGR_NET_API void report(const char* const func) noexcept;

  /**
   * @returns The last WSA error code.
   */
  static DMITIGR_NET_API int last_error() noexcept;
};
#endif

/**
 * Convenient macro for cross-platform network programming.
 *
 * This macro can be useful because on Windows some network functions
 * reports errors via GetLastError(), while others - via WSAGetLastError().
 */
#ifdef _WIN32
#define DMITIGR_NET_EXCEPTION dmitigr::net::Wsa_exception
#else
#define DMITIGR_NET_EXCEPTION dmitigr::os::Sys_exception
#endif

} // namespace dmitigr::net

#ifdef DMITIGR_NET_HEADER_ONLY
#include "dmitigr/net/exceptions.cpp"
#endif

#endif  // DMITIGR_NET_EXCEPTIONS_HPP
