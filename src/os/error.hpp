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

#ifndef DMITIGR_OS_ERROR_HPP
#define DMITIGR_OS_ERROR_HPP

#include "exceptions.hpp"

#include <cstring>
#include <string>

namespace dmitigr::os {

/// @returns String describing OS error code.
inline std::string error_message(const int code)
{
#ifdef _WIN32
  char buf[128];
  if (const int e = ::strerror_s(buf, code))
    throw Sys_exception{e, "cannot get an OS error message"};
  else
    return std::string{buf};
#else
  char buf[1024];
#if (_POSIX_C_SOURCE >= 200112L) && !_GNU_SOURCE
  const int e = ::strerror_r(code, buf, sizeof(buf));
  if (e < 0)
    throw Sys_exception{e, "cannot get an OS error message"};
  else if (e > 0)
    return "unknown error";
  else
    return std::string{buf};
#elif _GNU_SOURCE
  const char* const msg = ::strerror_r(code, buf, sizeof(buf));
  if (msg)
    return msg;
  else
    return "unknown error";
#else
#error Supported version of strerror_r() is not available.
#endif
#endif
}

} // namespace dmitigr::os

#endif  // DMITIGR_OS_ERROR_HPP
