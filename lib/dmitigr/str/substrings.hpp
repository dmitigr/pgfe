// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or str.hpp

#ifndef DMITIGR_STR_SUBSTRINGS_HPP
#define DMITIGR_STR_SUBSTRINGS_HPP

#include "dmitigr/base/debug.hpp"

#include <algorithm>
#include <functional>
#include <locale>
#include <string>
#include <string_view>
#include <utility>

namespace dmitigr::str {

/**
 * @returns The position of the first non-space character of `str` in the range
 * [pos, str.size()), or `std::string_view::npos` if there is no such a position.
 */
inline std::string_view::size_type
position_of_non_space(const std::string_view str, const std::string_view::size_type pos, const std::locale& loc = {})
{
  DMITIGR_REQUIRE(pos <= str.size(), std::out_of_range);
  const auto b = cbegin(str);
  const auto e = cend(str);
  const auto i = std::find_if(b + pos, e, std::bind(is_non_space_character, std::placeholders::_1, loc));
  return (i != e) ? i - b : std::string_view::npos;
}

/**
 * @returns The substring of `str` from position of `pos` until the position
 * of the character `c` that `(pred(c) == false)` as the first element, and
 * the position of the character followed `c` as the second element.
 */
template<typename Pred>
std::pair<std::string, std::string::size_type>
substring_if(const std::string& str, Pred&& pred, std::string::size_type pos, const std::locale& loc = {})
{
  DMITIGR_ASSERT(pos <= str.size());
  std::pair<std::string, std::string::size_type> result;
  const auto input_size = str.size();
  for (; pos < input_size; ++pos) {
    if (pred(str[pos], loc))
      result.first += str[pos];
    else
      break;
  }
  result.second = pos;
  return result;
}

/**
 * @returns The substring of `str` with the "simple identifier" that starts
 * from the position of `pos` in pair with the position of a character following
 * that substring.
 */
inline std::pair<std::string, std::string::size_type>
substring_if_simple_identifier(const std::string& str, const std::string::size_type pos, const std::locale& loc = {})
{
  DMITIGR_REQUIRE(pos <= str.size(), std::out_of_range);
  return std::isalpha(str[pos], loc) ?
    substring_if(str, std::bind(is_simple_identifier_character, std::placeholders::_1, loc), pos) :
    std::make_pair(std::string{}, pos);
}

/**
 * @returns The substring of `str` without spaces that starts from the position
 * of `pos` in pair with the position of a character following that substring.
 */
inline std::pair<std::string, std::string::size_type>
substring_if_no_spaces(const std::string& str, const std::string::size_type pos, const std::locale& loc = {})
{
  return substring_if(str, std::bind(is_non_space_character, std::placeholders::_1, loc), pos);
}

/**
 * @returns The unquoted substring of `str` if `(str[pos] == '\'')` or the
 * substring without spaces from the position of `pos` in pair with the position
 * of a character following that substring.
 */
inline std::pair<std::string, std::string::size_type>
unquoted_substring(const std::string& str, std::string::size_type pos, const std::locale& loc = {})
{
  DMITIGR_REQUIRE(pos <= str.size(), std::out_of_range);
  if (pos == str.size())
    return {std::string{}, pos};

  std::pair<std::string, std::string::size_type> result;
  constexpr char quote_char = '\'';
  constexpr char escape_char = '\\';
  if (str[pos] == quote_char) {
    // Trying to reach the trailing quote character.
    const auto input_size = str.size();
    enum { normal, escape } state = normal;
    for (++pos; pos < input_size; ++pos) {
      const auto ch = str[pos];
      switch (state) {
      case normal:
        if (ch == quote_char)
          goto finish;
        else if (ch == escape_char)
          state = escape;
        else
          result.first += ch;
        break;
      case escape:
        if (ch != quote_char)
          result.first += escape_char; // it's not escape, so preserve
        result.first += ch;
        state = normal;
        break;
      }
    }

  finish:
    if ((pos == input_size && str.back() != quote_char) || (pos < input_size && str[pos] != quote_char))
      throw std::runtime_error{"no trailing quote found"};
    else
      result.second = pos + 1; // discarding the trailing quote
  } else
    result = substring_if_no_spaces(str, pos, loc);
  return result;
}

} // namespace dmitigr::str

#endif  // DMITIGR_STR_SUBSTRINGS_HPP
