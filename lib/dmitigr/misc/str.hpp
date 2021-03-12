// -*- C++ -*-
// Copyright (C) 2020 Dmitry Igrishin
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

#ifndef DMITIGR_MISC_STR_HPP
#define DMITIGR_MISC_STR_HPP

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <functional>
#include <limits>
#include <locale>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

namespace dmitigr::str {

// -----------------------------------------------------------------------------
// Numeric conversions
// -----------------------------------------------------------------------------

/**
 * @returns The string with the character representation
 * of the `value` according to the given `base`.
 *
 * @par Requires
 * `(2 <= base && base <= 36)`.
 */
template<typename Number>
std::enable_if_t<std::is_integral<Number>::value, std::string>
to_string(Number value, const Number base = 10)
{
  assert(2 <= base && base <= 36);
  static_assert(std::numeric_limits<Number>::min() <= 2 && std::numeric_limits<Number>::max() >= 36);
  static const char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                                'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
                                'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
                                'U', 'V', 'W', 'X', 'Y', 'Z'};
  static_assert(sizeof(digits) == 36);
  const bool negative = (value < 0);
  std::string result;
  if (negative)
    value = -value;
  while (value >= base) {
    const auto rem = value % base;
    value /= base;
    result += digits[rem];
  }
  result += digits[value];
  if (negative)
    result += '-';
  std::reverse(begin(result), end(result));
  return result;
}

// -----------------------------------------------------------------------------
// Sequence conversions
// -----------------------------------------------------------------------------

/**
 * @returns The string with stringified elements of the sequence in range `[b, e)`.
 */
template<class InputIterator, typename Function>
std::string to_string(const InputIterator b, const InputIterator e, const std::string& sep, Function&& to_str)
{
  std::string result;
  if (b != e) {
    auto i = b;
    for (; i != e; ++i) {
      result.append(to_str(*i));
      result.append(sep);
    }
    const auto sep_size = sep.size();
    for (std::string::size_type j = 0; j < sep_size; ++j)
      result.pop_back();
  }
  return result;
}

/// @returns The string with stringified elements of the `Container`.
template<class Container, typename Function>
std::string to_string(const Container& cont, const std::string& sep, Function&& to_str)
{
  return to_string(cbegin(cont), cend(cont), sep, std::forward<Function>(to_str));
}

/// @returns The string with stringified elements of the `Container`.
template<class Container>
std::string to_string(const Container& cont, const std::string& sep)
{
  return to_string(cont, sep, [](const std::string& e)->const auto& { return e; });
}

// -----------------------------------------------------------------------------
// C-strings
// -----------------------------------------------------------------------------

/**
 * @returns The pointer to a next non-space character, or pointer to the
 * terminating zero character.
 */
inline const char* next_non_space_pointer(const char* p, const std::locale& loc = {}) noexcept
{
  if (p) {
    while (*p != '\0' && std::isspace(*p, loc))
      ++p;
  }
  return p;
}

/// @returns The specified literal if `(literal != nullptr)`, or "" otherwise.
inline const char* literal(const char* const literal) noexcept
{
  return literal ? literal : "";
}

/**
 * @returns The first non-null string literal of specified literals, or
 * `nullptr` if all of the `literals` are nulls.
 */
inline const char* coalesce(std::initializer_list<const char*> literals) noexcept
{
  for (const auto l : literals) {
    if (l)
      return l;
  }
  return nullptr;
}

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
line_number_by_position(const std::string& str, const std::string::size_type pos) noexcept
{
  assert(pos < str.size());
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
line_column_numbers_by_position(const std::string& str, const std::string::size_type pos) noexcept
{
  assert(pos < str.size());
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

// -----------------------------------------------------------------------------
// Predicates
// -----------------------------------------------------------------------------

/// @returns `true` if `c` is a valid space character.
inline bool is_space_character(const char c, const std::locale& loc = {}) noexcept
{
  return std::isspace(c, loc);
}

/// @returns `(is_space_character(c) == false)`.
inline bool is_non_space_character(const char c, const std::locale& loc = {}) noexcept
{
  return !is_space_character(c, loc);
}

/// @returns `true` if `c` is a valid simple identifier character.
inline bool is_simple_identifier_character(const char c, const std::locale& loc = {}) noexcept
{
  return std::isalnum(c, loc) || c == '_';
}

/// @returns `(is_simple_identifier_character(c) == false)`.
inline bool is_non_simple_identifier_character(const char c, const std::locale& loc = {}) noexcept
{
  return !is_simple_identifier_character(c, loc);
}

/// @returns `true` if `str` has at least one space character.
inline bool has_space(const std::string& str, const std::locale& loc = {}) noexcept
{
  return std::any_of(cbegin(str), cend(str), std::bind(is_space_character, std::placeholders::_1, loc));
}

/// @returns `true` if `input` is starting with `pattern`.
inline bool is_begins_with(const std::string_view input, const std::string_view pattern) noexcept
{
  return (pattern.size() <= input.size()) && std::equal(cbegin(pattern), cend(pattern), cbegin(input));
}

// -----------------------------------------------------------------------------
// Substrings
// -----------------------------------------------------------------------------

/**
 * @returns The position of the first non-space character of `str` in the range
 * [pos, str.size()), or `std::string_view::npos` if there is no such a position.
 */
inline std::string_view::size_type
position_of_non_space(const std::string_view str, const std::string_view::size_type pos, const std::locale& loc = {}) noexcept
{
  assert(pos <= str.size());
  const auto b = cbegin(str);
  const auto e = cend(str);
  const auto i = std::find_if(b + pos, e, std::bind(is_non_space_character, std::placeholders::_1, loc));
  return (i != e) ? static_cast<std::string_view::size_type>(i - b) : std::string_view::npos;
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
  assert(pos <= str.size());
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
  assert(pos <= str.size());
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
  assert(pos <= str.size());
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

// -----------------------------------------------------------------------------
// Transformators
// -----------------------------------------------------------------------------

/// @returns The string with the specified `delimiter` between the characters.
inline std::string sparsed_string(const std::string_view input, const std::string& delimiter)
{
  std::string result;
  if (!input.empty()) {
    result.reserve(input.size() + (input.size() - 1) * delimiter.size());
    auto i = cbegin(input);
    auto const e = cend(input) - 1;
    for (; i != e; ++i) {
      result += *i;
      result += delimiter;
    }
    result += *i;
  }
  return result;
}

/**
 * @par Effects
 * `(str.back() == c)`.
 */
inline void terminate(std::string& str, const char c)
{
  if (str.empty() || str.back() != c)
    str += c;
}

// Trims `str` by dropping whitespaces at both sides of it.
inline void trim(std::string& str, const std::locale& loc = {})
{
  if (str.empty())
    return;

  const auto is_not_space = [&loc](const auto c) { return !std::isspace(c, loc); };
  const auto b = begin(str);
  const auto e = end(str);
  const auto tb = find_if(b, e, is_not_space);
  if (tb == e) {
    str.clear(); // the string consists of spaces, so just clear it out
    return;
  }

  const auto rb = rbegin(str);
  const auto re = rend(str);
  const auto te = find_if(rb, re, is_not_space).base();
  move(tb, te, b);
  str.resize(te - tb);
}

/**
 * @brief Splits the `input` string into the parts separated by the
 * specified `separators`.
 *
 * @returns The vector of splitted parts.
 */
template<class S = std::string>
inline std::vector<S> split(const std::string_view input,
  const std::string_view separators)
{
  std::vector<S> result;
  result.reserve(4);
  std::string_view::size_type pos{std::string_view::npos};
  std::string_view::size_type offset{};
  while (offset < input.size()) {
    pos = input.find_first_of(separators, offset);
    assert(offset <= pos);
    const auto part_size = std::min<std::string_view::size_type>(pos, input.size()) - offset;
    result.push_back(S{input.substr(offset, part_size)});
    offset += part_size + 1;
  }
  if (pos != std::string_view::npos) // input ends with a separator
    result.emplace_back();
  return result;
}

// -----------------------------------------------------------------------------
// lowercase

/**
 * @brief Replaces all of uppercase characters in `str` by the corresponding
 * lowercase characters.
 */
inline void lowercase(std::string& str, const std::locale& loc = {})
{
  auto b = begin(str);
  auto e = end(str);
  std::transform(b, e, b, [&loc](const char c) { return std::tolower(c, loc); });
}

/**
 * @returns The modified copy of the `str` with all of uppercase characters
 * replaced by the corresponding lowercase characters.
 */
inline std::string to_lowercase(std::string str, const std::locale& loc = {})
{
  lowercase(str, loc);
  return str;
}

/// @returns `true` if all of characters of `str` are in uppercase.
inline bool is_lowercased(const std::string_view str, const std::locale& loc = {}) noexcept
{
  return std::all_of(cbegin(str), cend(str), [&loc](const char c) { return std::islower(c, loc); });
}

// -----------------------------------------------------------------------------
// uppercase

/**
 * @brief Replaces all of lowercase characters in `str` by the corresponding
 * uppercase characters.
 */
inline void uppercase(std::string& str, const std::locale& loc = {})
{
  auto b = begin(str);
  auto e = end(str);
  std::transform(b, e, b, [&loc](const char c) { return std::toupper(c, loc); });
}

/**
 * @returns The modified copy of the `str` with all of lowercase characters
 * replaced by the corresponding uppercase characters.
 */
inline std::string to_uppercase(std::string str, const std::locale& loc = {})
{
  uppercase(str, loc);
  return str;
}

/// @returns `true` if all of character of `str` are in lowercase.
inline bool is_uppercased(const std::string_view str, const std::locale& loc = {}) noexcept
{
  return std::all_of(cbegin(str), cend(str), [&loc](const char c) { return std::isupper(c, loc); });
}

} // namespace dmitigr::str

#endif  // DMITIGR_MISC_STR_HPP
