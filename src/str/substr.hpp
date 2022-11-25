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

#ifndef DMITIGR_STR_SUBSTR_HPP
#define DMITIGR_STR_SUBSTR_HPP

#include "predicate.hpp"
#include "exceptions.hpp"

#include <algorithm>
#include <cctype>
#include <string_view>

namespace dmitigr::str {

// -----------------------------------------------------------------------------
// Substrings
// -----------------------------------------------------------------------------

/**
 * @returns The position of the first non-space character of `str` in the range
 * [pos, str.size()), or `std::string_view::npos` if there is no such a position.
 *
 * @par Requires
 * `pos <= str.size()`.
 */
inline std::string_view::size_type
first_non_space_pos(const std::string_view str, const std::string_view::size_type pos)
{
  if (!(pos <= str.size()))
    throw Exception{"cannot get position of non space by using invalid offset"};

  const auto b = cbegin(str);
  const auto e = cend(str);
  const auto i = std::find_if(b + pos, e, is_non_space<char>);
  return i != e ? static_cast<std::string_view::size_type>(i - b)
    : std::string_view::npos;
}

} // namespace dmitigr::str

#endif  // DMITIGR_STR_SUBSTR_HPP
