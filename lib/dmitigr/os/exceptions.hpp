// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#ifndef DMITIGR_OS_EXCEPTIONS_HPP
#define DMITIGR_OS_EXCEPTIONS_HPP

#include <cassert>
#include <cstdio>
#include <string>
#include <system_error>

#ifdef _WIN32
#include "dmitigr/os/windows.hpp"
#endif

namespace dmitigr::os {

/**
 * @brief An exception thrown on system error.
 */
class Sys_exception final : public std::system_error {
public:
  /**
   * @brief The constructor.
   */
  explicit Sys_exception(const std::string& what)
    : std::system_error{last_error(), std::system_category(), what}
  {}

  /**
   * @brief Prints the last system error to the standard error.
   */
  static void report(const char* const what) noexcept
  {
    assert(what);
    std::fprintf(stderr, "%s: error %d\n", what, last_error());
  }

  /**
   * @returns The last system error code.
   */
  static int last_error() noexcept
  {
#ifdef _WIN32
    return static_cast<int>(::GetLastError());
#else
    return errno;
#endif
  }
};

} // namespace dmitigr::os

#endif  // DMITIGR_OS_EXCEPTIONS_HPP
