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

#ifndef DMITIGR_NET_LAST_ERROR_HPP
#define DMITIGR_NET_LAST_ERROR_HPP

#include "../base/assert.hpp"

#ifdef _WIN32
#include <Winsock2.h>
#else
#include "../os/last_error.hpp"
#endif

namespace dmitigr::net {
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
  std::fprintf(stderr, "%s(): error %d\n", context, last_error());
}
} // namespace dmitigr::net

#endif  // DMITIGR_NET_LAST_ERROR_HPP
