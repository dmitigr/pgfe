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

#ifndef DMITIGR_OS_ENVIRONMENT_HPP
#define DMITIGR_OS_ENVIRONMENT_HPP

#include "exceptions.hpp"

#include <cstddef>
#include <cstdlib>
#include <optional>
#include <memory>
#include <string>

#ifdef _WIN32

#include "windows.hpp"

#include <Winnls.h>
#include <Lmcons.h>

// IO headers
#include <io.h>
#include <stdio.h>

#else // Unix

#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>

#endif

namespace dmitigr::os {

/// @returns The current username of the running process.
inline std::string current_username()
{
  std::string result;
#ifdef _WIN32
  constexpr DWORD max_size = UNLEN + 1;
  result.resize(max_size);
  DWORD sz{max_size};
  if (::GetUserName(result.data(), &sz) != 0)
    result.resize(sz - 1);
  else
    throw Sys_exception{"cannot get current username of the running process"};
#else
  struct passwd pwd;
  struct passwd *pwd_ptr{};
  const uid_t uid = geteuid();
  const std::size_t bufsz = []()
  {
    auto result = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (result == -1)
      result = 16384;
    return result;
  }();
  const std::unique_ptr<char[]> buf{new char[bufsz]};
  const int s = getpwuid_r(uid, &pwd, buf.get(), bufsz, &pwd_ptr);
  if (!pwd_ptr) {
    if (s)
      throw Sys_exception{s, "cannot get current username of the running process"};
    else
      result = std::to_string(uid);
  } else
    result = pwd.pw_name;
#endif
  return result;
}

/**
 * @returns The value of the environment variable `name` that is accessible
 * from the running process, or `std::nullopt` if there is no such a variable.
 *
 * @remarks Cannot be used in applications that execute in the Windows Runtime,
 * because environment variables are not available to UWP applications.
 */
inline std::optional<std::string> environment_variable(const std::string& name)
{
#if defined(_WIN32) && defined(_MSC_VER)
  const std::unique_ptr<char, void(*)(void*)> buffer{nullptr, &std::free};
  char* result = buffer.get();
  if (const auto err = _dupenv_s(&result, nullptr, name.c_str()))
    throw Sys_exception{err, "cannot get the environment variable \""+name+"\""};
#else
  const char* const result = std::getenv(name.c_str());
#endif
  return result ? std::make_optional(std::string{result}) : std::nullopt;
}

} // namespace dmitigr::os

#endif  // DMITIGR_OS_ENVIRONMENT_HPP
