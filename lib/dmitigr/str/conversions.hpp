// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or str.hpp

#ifndef DMITIGR_STR_CONVERSIONS_HPP
#define DMITIGR_STR_CONVERSIONS_HPP

#include <dmitigr/base/debug.hpp>

#include <algorithm>
#include <limits>
#include <string>
#include <type_traits>

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
  DMITIGR_REQUIRE(2 <= base && base <= 36, std::out_of_range);
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

/**
 * @returns The string with stringified elements of the `Container`.
 */
template<class Container, typename Function>
std::string to_string(const Container& cont, const std::string& sep, Function&& to_str)
{
  return to_string(cbegin(cont), cend(cont), sep, std::forward<Function>(to_str));
}

/**
 * @returns The string with stringified elements of the `Container`.
 */
template<class Container>
std::string to_string(const Container& cont, const std::string& sep)
{
  return to_string(cont, sep, [](const std::string& e)->const auto& { return e; });
}

} // namespace dmitigr::str

#endif  // DMITIGR_STR_CONVERSIONS_HPP
