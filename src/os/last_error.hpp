// -*- C++ -*-
//
// Copyright 2022 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
} // namespace dmitigr::os

#endif  // DMITIGR_OS_LAST_ERROR_HPP
