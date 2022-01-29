// -*- C++ -*-
// Copyright (C) 2022 Dmitry Igrishin
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// Dmitry Igrishin
// dmitigr@gmail.com

#ifndef DMITIGR_OS_LAST_ERROR_HPP
#define DMITIGR_OS_LAST_ERROR_HPP

#include "../base/assert.hpp"

#ifdef _WIN32
#include "windows.hpp"
#else
#include <cerrno>
#endif

#include <cstdio>

namespace dmitigr::os {
/**
 * @returns The last system error code.
 *
 * @par Thread safety
 * Thread-safe.
 */
inline int last_error() noexcept
{
#ifdef _WIN32
  // Note: the last-error code is maintained on a per-thread basis.
  return static_cast<int>(::GetLastError());
#else
  /*
   * Note: errno is thread-local on Linux.
   * See also http://www.unix.org/whitepapers/reentrant.html
   */
  return errno;
#endif
}

/// Prints the last system error to the standard error.
inline void print_last_error(const char* const context) noexcept
{
  DMITIGR_ASSERT(context);
  std::fprintf(stderr, "%s: error %d\n", context, last_error());
}

namespace net {
/// @returns The last network subsystem's error code.
inline int last_error() noexcept
{
#ifdef _WIN32
  return ::WSAGetLastError();
#else
  return os::last_error();
#endif
}

/// Prints the last network subsystem's error to the standard error.
inline void print_last_error(const char* const context) noexcept
{
  DMITIGR_ASSERT(context);
  std::fprintf(stderr, "%s(): error %d\n", context, net::last_error());
}
} // namespace net
} // namespace dmitigr::os

#endif  // DMITIGR_OS_LAST_ERROR_HPP
