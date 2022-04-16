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
    return buf;
#else
  char buf[1024];
#if ((_POSIX_C_SOURCE >= 200112L) && !_GNU_SOURCE) || __APPLE__
  const int e = ::strerror_r(code, buf, sizeof(buf));
  return !e ? buf : "unknown error";
#elif _GNU_SOURCE
  const char* const msg = ::strerror_r(code, buf, sizeof(buf));
  return msg ? msg : "unknown error";
#else
#error Supported version of strerror_r() is not available.
#endif
#endif
}

} // namespace dmitigr::os

#endif  // DMITIGR_OS_ERROR_HPP
