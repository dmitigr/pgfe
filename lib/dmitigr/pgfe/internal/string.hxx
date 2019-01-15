// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_INTERNAL_STRING_HXX
#define DMITIGR_PGFE_INTERNAL_STRING_HXX

#include "dmitigr/pgfe/internal/debug.hxx"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <locale>
#include <string>
#include <utility>

namespace dmitigr::pgfe::internal::string {

// -----------------------------------------------------------------------------
// C strings

/**
 * @internal
 *
 * @returns The pointer to the next non-space character, or pointer to the
 * terminating zero character.
 */
inline const char* next_non_space_pointer(const char* p) noexcept
{
  if (p)
    while (*p != '\0' && std::isspace(*p, std::locale{}))
      ++p;
  return p;
}

/**
 * @internal
 *
 * @returns The specified literal if `(literal != nullptr)`, or "" otherwise.
 */
inline const char* literal(const char* const literal) noexcept
{
  return literal ? literal : "";
}

/**
 * @internal
 *
 * @returns First non-null string literal of specified literals, or
 * `nullptr` if there is no such a literal.
 */
constexpr const char* coalesce(std::initializer_list<const char*> literals) noexcept
{
  for (const auto l : literals)
    if (l)
      return l;
  return nullptr;
}

// -----------------------------------------------------------------------------
// Text lines manipulations

/**
 * @internal
 *
 * @returns Line number by the given absolute position. (Line numbers starts at 1.)
 */
std::size_t line_number_by_position(const std::string& str, const std::size_t pos);

/**
 * @internal
 *
 * @returns Line and column numbers by the given absolute position. (Both numbers starts at 1.)
 */
std::pair<std::size_t, std::size_t> line_column_numbers_by_position(const std::string& str, const std::size_t pos);

// -----------------------------------------------------------------------------
// Predicates

/**
 * @internal
 *
 * @returns `true` if `c` is a valid space character.
 */
inline bool is_space_character(const char c)
{
  return std::isspace(c, std::locale{});
}

/**
 * @internal
 *
 * @returns `true` if `c` is a valid non-space character.
 */
inline bool is_non_space_character(const char c)
{
  return !std::isspace(c, std::locale{});
}

/**
 * @internal
 *
 * @returns `true` if `c` is a valid simple identifier character.
 */
inline bool is_simple_identifier_character(const char c)
{
  return std::isalnum(c, std::locale{}) || c == '_';
}

/**
 * @internal
 *
 * @returns `true` if `str` has at least one space character.
 */
inline bool has_space(const std::string& str)
{
  return std::any_of(cbegin(str), cend(str), is_space_character);
}

// -----------------------------------------------------------------------------
// Generators

/**
 * @internal
 *
 * @returns A random string of specified size from chars of palette.
 */
std::string random_string(const std::string& palette, std::string::size_type size);

/**
 * @internal
 *
 * @returns A random string of specified size from chars in the range [beg,end).
 *
 * @par Requires
 * `(beg < end)`
 */
std::string random_string(char beg, char end, std::string::size_type size);

// -----------------------------------------------------------------------------
// Transformators

/**
 * @internal
 *
 * @returns "i_n_p_u_t", where the "_" is the value of the `separator`.
 */
std::string sparsed_string(const std::string& input, const std::string& separator);

/**
 * @internal
 *
 * @par Effects
 * `(str.back() == c)`
 */
std::string& terminate_string(std::string& str, char c);

// -----------------------------------------------------------------------------
// Substrings

/**
 * @returns The position of the first non-space character of `str` in range [pos, str.size()).
 */
std::string::size_type position_of_non_space(const std::string& str, std::string::size_type pos);

/**
 * @returns The substring of `str` from position of `pos` until the position
 * of the character "c" that `pred(c) == false` as the first element, and the
 * position of the character followed "c" as the second element.
 */
template<typename Pred>
std::pair<std::string, std::string::size_type> substring_if(const std::string& str, Pred pred, std::string::size_type pos)
{
  DMITIGR_PGFE_INTERNAL_ASSERT(pos <= str.size());
  std::pair<std::string, std::string::size_type> result;
  const auto input_size = str.size();
  for (; pos < input_size; ++pos) {
    if (pred(str[pos]))
      result.first += str[pos];
    else
      break;
  }
  result.second = pos;
  return result;
}

/**
 * @returns The substring of `str` with the "simple identifier" from position of `pos`
 * as the first element, and the position of the next character in `str`.
 */
std::pair<std::string, std::string::size_type>
substring_if_simple_identifier(const std::string& str, std::string::size_type pos);

/**
 * @returns The substring of `str` with no spaces from position of `pos`
 * as the first element, and the position of the next character in `str`.
 */
std::pair<std::string, std::string::size_type>
substring_if_no_spaces(const std::string& str, std::string::size_type pos);

/**
 * @returns The unquoted substring of `str` if `str[pos] == '\''` or the substring
 * with no spaces from the position of `pos` as the first element, and the position
 * of the next character in `str`.
 */
std::pair<std::string, std::string::size_type> unquoted_substring(const std::string& str, std::string::size_type pos);

// -----------------------------------------------------------------------------
// Sequence converters

/**
 * @internal
 *
 * @returns The string with stringified elements of the sequence in range [b, e).
 */
template<class InputIterator, typename Function>
std::string to_string(const InputIterator b, const InputIterator e, const std::string& sep, Function to_str)
{
  std::string result;
  if (b != e) {
    auto i = b;
    for (; i != e; ++i) {
      result.append(to_str(*i));
      result.append(sep);
    }
    const std::string::size_type sep_size = sep.size();
    for (std::string::size_type i = 0; i < sep_size; ++i)
      result.pop_back();
  }
  return result;
}

/**
 * @internal
 *
 * @returns The string with stringified elements of the Container.
 */
template<class Container, typename Function>
std::string to_string(const Container& cont, const std::string& sep, Function to_str)
{
  return to_string(cbegin(cont), cend(cont), sep, to_str);
}

/**
 * @internal
 *
 * @returns The string with stringified elements of the Container.
 */
template<class Container>
std::string to_string(const Container& cont, const std::string& sep)
{
  return to_string(cont, sep, [](const std::string& e)->const auto& { return e; });
}

// -----------------------------------------------------------------------------
// Numeric converters

/**
 * @internal
 *
 * @returns The string object holding the character representation of the `value`
 * according to the given `base`.
 *
 * @par Requires
 * `(2 <= base && base <= 36)`
 */
template<typename Number>
std::enable_if_t<std::is_integral<Number>::value, std::string>
to_string(Number value, const Number base = 10)
{
  DMITIGR_PGFE_INTERNAL_ASSERT(2 <= base && base <= 36);
  static const char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                                'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
                                'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
                                'U', 'V', 'W', 'X', 'Y', 'Z'};
  static_assert(sizeof(digits) == 36, "");
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

} // namespace dmitigr::pgfe::internal::string

#endif  // DMITIGR_PGFE_INTERNAL_STRING_HXX
