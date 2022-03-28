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

#ifndef DMITIGR_STR_SUBSTR_HPP
#define DMITIGR_STR_SUBSTR_HPP

#include "exceptions.hpp"
#include "version.hpp"

#include <algorithm>
#include <locale>
#include <string>
#include <string_view>
#include <utility>

namespace dmitigr::str {

// -----------------------------------------------------------------------------
// Substrings
// -----------------------------------------------------------------------------

/**
 * @returns The position of the first non-space character of `str` in the range
 * [pos, str.size()), or `std::string_view::npos` if there is no such a position.
 *
 * @par Requires
 * `(pos <= str.size())`.
 */
inline std::string_view::size_type
position_of_non_space(const std::string_view str,
  const std::string_view::size_type pos, const std::locale& loc = {})
{
  if (!(pos <= str.size()))
    throw Exception{"cannot get position of non space by using invalid offset"};

  const auto b = cbegin(str);
  const auto e = cend(str);
  const auto i = std::find_if(b + pos, e, [&loc](const auto ch)
  {
    return is_non_space_character(ch, loc);
  });
  return (i != e) ? static_cast<std::string_view::size_type>(i - b) :
    std::string_view::npos;
}

/**
 * @returns The substring of `str` from position of `pos` until the position
 * of the character `c` that `(pred(c) == false)` as the first element, and
 * the position of the character followed `c` as the second element.
 *
 * @par Requires
 * `(pos <= str.size())`.
 */
template<typename Pred>
std::pair<std::string, std::string::size_type>
substring_if(const std::string& str, const Pred& pred,
  std::string::size_type pos, const std::locale& loc = {})
{
  if (!(pos <= str.size()))
    throw Exception{"cannot get substring by using invalid offset"};

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
 *
 * @par Requires
 * `(pos <= str.size())`.
 */
inline std::pair<std::string, std::string::size_type>
substring_if_simple_identifier(const std::string& str,
  const std::string::size_type pos, const std::locale& loc = {})
{
  if (!(pos <= str.size()))
    throw Exception{"cannot get substring by using invalid offset"};

  return std::isalpha(str[pos], loc) ?
    substring_if(str, is_simple_identifier_character, pos) :
    std::make_pair(std::string{}, pos);
}

/**
 * @returns The substring of `str` without spaces that starts from the position
 * of `pos` in pair with the position of a character following that substring.
 *
 * @par Requires
 * `(pos <= str.size())`.
 */
inline std::pair<std::string, std::string::size_type>
substring_if_no_spaces(const std::string& str, const std::string::size_type pos,
  const std::locale& loc = {})
{
  return substring_if(str, is_non_space_character, pos, loc);
}

/**
 * @returns The unquoted substring of `str` if `(str[pos] == '\'')` or the
 * substring without spaces from the position of `pos` in pair with the position
 * of a character following that substring.
 *
 * @par Requires
 * `(pos <= str.size())`.
 */
inline std::pair<std::string, std::string::size_type>
unquoted_substring(const std::string& str, std::string::size_type pos,
  const std::locale& loc = {})
{
  if (!(pos <= str.size()))
    throw Exception{"cannot get unquoted substring by using invalid offset"};

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
    if ((pos == input_size && str.back() != quote_char) ||
      (pos < input_size && str[pos] != quote_char))
      throw Exception{"cannot get unquoted substring because no trailing quote "
        "found"};
    else
      result.second = pos + 1; // discarding the trailing quote
  } else
    result = substring_if_no_spaces(str, pos, loc);
  return result;
}

} // namespace dmitigr::str

#endif  // DMITIGR_STR_SUBSTR_HPP
