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

#ifndef DMITIGR_STR_LINE_HPP
#define DMITIGR_STR_LINE_HPP

#include "exceptions.hpp"
#include "version.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

namespace dmitigr::str {

// -----------------------------------------------------------------------------
// Lines
// -----------------------------------------------------------------------------

/**
 * @returns The line number (which starts at 0) by the given absolute position.
 *
 * @par Requires
 * `(pos < str.size())`.
 */
inline auto
line_number_by_position(const std::string& str, const std::string::size_type pos)
{
  if (!(pos < str.size()))
    throw Exception{"cannot get line number by invalid position"};

  using Diff = decltype(cbegin(str))::difference_type;
  return std::count(cbegin(str), cbegin(str) + static_cast<Diff>(pos), '\n');
}

/**
 * @returns The line and column numbers (both starts at 0) by the given absolute
 * position.
 *
 * @par Requires
 * `(pos < str.size())`.
 */
inline std::pair<std::size_t, std::size_t>
line_column_numbers_by_position(const std::string& str,
  const std::string::size_type pos)
{
  if (!(pos < str.size()))
    throw Exception{"cannot get line and column numbers by invalid position"};

  std::size_t line{};
  std::size_t column{};
  for (std::size_t i = 0; i < pos; ++i) {
    ++column;
    if (str[i] == '\n') {
      ++line;
      column = 0;
    }
  }
  return std::make_pair(line, column);
}

} // namespace dmitigr::str

#endif  // DMITIGR_STR_LINE_HPP
