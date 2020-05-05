// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or net.hpp

#include "dmitigr/net/exceptions.hpp"
#ifdef _WIN32
#include "dmitigr/os/windows.hpp"

#include <cassert>
#include <cstdio>
#include <string>

#include <Winsock2.h> // includes Ws2def.h
#endif

namespace dmitigr::net {

#ifdef _WIN32

DMITIGR_NET_INLINE const char* Wsa_error_category::name() const noexcept
{
  return "dmitigr_wsa_error";
}

DMITIGR_NET_INLINE std::string Wsa_error_category::message(const int ev) const
{
  std::string result(name());
  result += ' ';
  result += std::to_string(ev);
  return result;
}

DMITIGR_NET_INLINE const Wsa_error_category& wsa_error_category() noexcept
{
  static const Wsa_error_category result;
  return result;
}

DMITIGR_NET_INLINE Wsa_exception::Wsa_exception(const std::string& func)
  : std::system_error{last_error(), wsa_error_category(), func}
{}

DMITIGR_NET_INLINE void Wsa_exception::report(const char* const func) noexcept
{
  assert(func);
  std::fprintf(stderr, "%s(): error %d\n", func, last_error());
}

DMITIGR_NET_INLINE int Wsa_exception::last_error() noexcept
{
  return ::WSAGetLastError();
}

#endif  // _WIN32

} // namespace dmitigr::net
