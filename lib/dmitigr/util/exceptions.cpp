// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#include "dmitigr/util/exceptions.hpp"
#ifdef _WIN32
#include "dmitigr/util/windows.hpp"
#endif
#include "dmitigr/util/implementation_header.hpp"

#include <cassert>
#include <cstdio>

#ifdef _WIN32
#include <Winsock2.h> // includes Ws2def.h
#endif

namespace dmitigr {

DMITIGR_UTIL_INLINE Sys_exception::Sys_exception(const std::string& func)
  : std::system_error{last_error(), std::system_category(), func}
{}

DMITIGR_UTIL_INLINE void Sys_exception::report(const char* const func) noexcept
{
  assert(func);
  std::fprintf(stderr, "%s(): error %d\n", func, last_error());
}

DMITIGR_UTIL_INLINE int Sys_exception::last_error() noexcept
{
#ifdef _WIN32
  return static_cast<int>(::GetLastError());
#else
  return errno;
#endif
}

// =============================================================================

#ifdef _WIN32

DMITIGR_UTIL_INLINE const char* Wsa_error_category::name() const noexcept
{
  return "dmitigr_wsa_error";
}

DMITIGR_UTIL_INLINE std::string Wsa_error_category::message(const int ev) const
{
  std::string result(name());
  result += ' ';
  result += std::to_string(ev);
  return result;
}

DMITIGR_UTIL_INLINE const Wsa_error_category& wsa_error_category() noexcept
{
  static const Wsa_error_category result;
  return result;
}

DMITIGR_UTIL_INLINE Wsa_exception::Wsa_exception(const std::string& func)
  : std::system_error{last_error(), wsa_error_category(), func}
{}

DMITIGR_UTIL_INLINE void Wsa_exception::report(const char* const func) noexcept
{
  assert(func);
  std::fprintf(stderr, "%s(): error %d\n", func, last_error());
}

DMITIGR_UTIL_INLINE int Wsa_exception::last_error() noexcept
{
  return ::WSAGetLastError();
}

#endif  // _WIN32

} // namespace dmitigr

#include "dmitigr/util/implementation_footer.hpp"
