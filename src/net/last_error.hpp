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
