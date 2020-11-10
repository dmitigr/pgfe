// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or net.hpp

#ifndef DMITIGR_NET_UTIL_HPP
#define DMITIGR_NET_UTIL_HPP

#include <locale>
#include <string>

namespace dmitigr::net {

/**
 * @returns `true` if the `hostname` denotes
 * a valid hostname, or `false` otherwise.
 */
inline bool is_hostname_valid(const std::string& hostname)
{
  /// Returns `true` if `ch` is a valid hostname character.
  static const auto is_hostname_char = [](const char ch)
  {
    static const std::locale l;
    return std::isalnum(ch, l) || (ch == '_') || (ch == '-');
  };

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
    } else if (is_hostname_char(c)) {
      ++label_length;
      if (label_length > label_max_length)
        return false; // label too long
    } else
      return false; // invalid character
  }
  return true;
}

} // namespace dmitigr::net

#endif  // DMITIGR_NET_UTIL_HPP
