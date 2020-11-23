// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or os.hpp

#ifndef DMITIGR_OS_ENV_HPP
#define DMITIGR_OS_ENV_HPP

#include <cstddef>
#include <cstdlib>
#include <optional>
#include <memory>
#include <string>
#include <system_error>

#ifdef _WIN32

#include "dmitigr/os/windows.hpp"

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

namespace dmitigr::os::env {

/**
 * @returns The current username of the running process.
 */
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
    throw std::system_error{int(::GetLastError()), std::system_category(), "dmitigr::os::current_username()"};
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
  if (pwd_ptr == nullptr) {
    if (s == 0)
      throw std::runtime_error{"current username is unavailable (possible something wrong with the OS)"};
    else
      throw std::system_error{s, std::system_category(), "dmitigr::os::current_username()"};
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
  const auto err = _dupenv_s(&result, nullptr, name.c_str());
  if (err)
    throw std::system_error{err, std::system_category(), "dmitigr::os::environment_variable()"};
#else
  const char* const result = std::getenv(name.c_str());
#endif
  return result ? std::make_optional(std::string{result}) : std::nullopt;
}

} // namespace dmitigr::os::env

#endif  // DMITIGR_OS_ENV_HPP
