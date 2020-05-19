// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or net.hpp

#ifndef DMITIGR_NET_EXCEPTIONS_HPP
#define DMITIGR_NET_EXCEPTIONS_HPP

#include <dmitigr/os/exceptions.hpp>

#ifdef _WIN32
#include <dmitigr/os/windows.hpp>

#include <Winsock2.h> // includes Ws2def.h
#endif

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
  const char* name() const noexcept override
  {
    return "dmitigr_wsa_error";
  }

  /**
   * @returns The string that describes the error condition denoted by `ev`.
   *
   * @remarks The caller should not rely on the
   * return value since it is a subject to change.
   */
  std::string message(const int ev) const override
  {
    std::string result(name());
    result += ' ';
    result += std::to_string(ev);
    return result;
  }
};

/**
 * @returns The reference to instance of type Wsa_error_category.
 */
inline const Wsa_error_category& wsa_error_category() noexcept
{
  static const Wsa_error_category result;
  return result;
}

/**
 * @brief An exception thrown on Windows Socket Application (WSA) error.
 */
class Wsa_exception final : public std::system_error {
public:
  /**
   * @brief The constructor.
   */
  explicit Wsa_exception(const std::string& func)
    : std::system_error{last_error(), wsa_error_category(), func}
  {}

  /**
   * @brief Prints the last WSA error to the standard error.
   */
  static void report(const char* const func) noexcept
  {
    assert(func);
    std::fprintf(stderr, "%s(): error %d\n", func, last_error());
  }

  /**
   * @returns The last WSA error code.
   */
  static int last_error() noexcept
  {
    return ::WSAGetLastError();
  }
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

#endif  // DMITIGR_NET_EXCEPTIONS_HPP
