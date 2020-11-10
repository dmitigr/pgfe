// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or str.hpp

#ifndef DMITIGR_STR_LINES_HPP
#define DMITIGR_STR_LINES_HPP

#include "dmitigr/base/debug.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace dmitigr::str {

/**
 * @returns The line number (which starts at 0) by the given absolute position.
 *
 * @par Requires
 * `(pos < str.size())`.
 */
inline auto
line_number_by_position(const std::string& str, const std::string::size_type pos)
{
  DMITIGR_REQUIRE(pos < str.size(), std::out_of_range,
    "invalid position for dmitigr::str::line_number_by_position()");
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
line_column_numbers_by_position(const std::string& str, const std::string::size_type pos)
{
  DMITIGR_REQUIRE(pos < str.size(), std::out_of_range,
    "invalid position for dmitigr::str::line_column_numbers_by_position()");
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

#endif  // DMITIGR_STR_LINES_HPP
