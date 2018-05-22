// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/internal/net/inet.hxx"

#include <locale>
#include <system_error>

#ifdef _WIN32
#include <Winsock2.h> // includes Ws2def.h
#include <In6addr.h>  // must follows after Winsock2.h
#include <Ws2tcpip.h> // InetPton()
#else
#include <arpa/inet.h>
#endif

namespace net = dmitigr::internal::net;

namespace {

inline bool is_domain_name_char__(const char ch)
{
  return std::isalnum(ch, std::locale{}) || (ch == '_') || (ch == '-');
}

inline int inet_pton__(const int family, const char* const address, void* const buffer)
{
#ifdef _WIN32
  return ::InetPton(family, address, buffer);
#else
  return ::inet_pton(family, address, buffer);
#endif
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
        // Impossible, but who knows...
        throw std::system_error(int(std::errc::address_family_not_supported), std::system_category());
    }
  }
  return false;
}

bool net::is_domain_name_valid(const std::string& domain_name)
{
  constexpr std::string::size_type max_length{253};
  if (domain_name.empty() || domain_name.size() > max_length)
    return false;

  constexpr std::string::size_type label_max_length{63};
  const auto limit = domain_name.size();
  for (std::string::size_type i = 0, label_length = 0; i < limit; ++i) {
    const auto c = domain_name[i];
    if (c == '.') {
      if (label_length == 0)
        return false; // empty label
      label_length = 0;
    } else if (is_domain_name_char__(c)) {
      ++label_length;
      if (label_length > label_max_length)
        return false; // label too long
    } else
      return false; // invalid character
  }
  return true;
}
