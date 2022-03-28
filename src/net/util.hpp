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

#ifndef DMITIGR_NET_UTIL_HPP
#define DMITIGR_NET_UTIL_HPP

#include <locale>
#include <string>

namespace dmitigr::net {

/// @returns `true` if the `hostname` denotes a valid hostname.
inline bool is_hostname_valid(const std::string& hostname)
{
  // Returns `true` if `ch` is a valid hostname character.
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
  for (std::string::size_type i{}, label_length{}; i < limit; ++i) {
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
