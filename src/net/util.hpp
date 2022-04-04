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
