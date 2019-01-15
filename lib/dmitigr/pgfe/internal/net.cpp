// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/internal/net.hxx"

#include <locale>
#include <system_error>

#ifdef _WIN32
#include <Winsock2.h> // includes Ws2def.h
#include <In6addr.h>  // must follows after Winsock2.h
#include <Ws2tcpip.h> // InetPton()
#else
#include <arpa/inet.h>
#endif

namespace net = dmitigr::pgfe::internal::net;

namespace {

inline int inet_pton__(const int family, const char* const address, void* const buffer)
{
#ifdef _WIN32
  return ::InetPton(family, address, buffer);
#else
  return ::inet_pton(family, address, buffer);
#endif
}

inline bool is_hostname_char__(const char ch)
{
  return std::isalnum(ch, std::locale{}) || (ch == '_') || (ch == '-');
}

} // namespace

bool net::is_ip_address_valid(const std::string& address)
{
  unsigned char buf[sizeof (::in6_addr)];
  for (const auto family : {AF_INET, AF_INET6}) {
    if (const int result = inet_pton__(family, address.c_str(), buf)) {
      if (result > 0)
        return true;
      else
        // Unreachable code, but just in case...
        throw std::system_error{int(std::errc::address_family_not_supported), std::system_category()};
    }
  }
  return false;
}

bool net::is_hostname_valid(const std::string& hostname)
{
  constexpr std::string::size_type max_length{253};
  if (hostname.empty() || hostname.size() > max_length)
    return false;

  constexpr std::string::size_type label_max_length{63};
  const auto limit = hostname.size();
  for (std::string::size_type i = 0, label_length = 0; i < limit; ++i) {
    const auto c = hostname[i];
    if (c == '.') {
      if (label_length == 0)
        return false; // empty label
      label_length = 0;
    } else if (is_hostname_char__(c)) {
      ++label_length;
      if (label_length > label_max_length)
        return false; // label too long
    } else
      return false; // invalid character
  }
  return true;
}
