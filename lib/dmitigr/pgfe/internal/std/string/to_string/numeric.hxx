// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_INTERNAL_STD_STRING_TO_STRING_NUMERIC_HXX
#define DMITIGR_PGFE_INTERNAL_STD_STRING_TO_STRING_NUMERIC_HXX

#include "dmitigr/pgfe/internal/debug.hxx"

#include <algorithm>
#include <string>
#include <type_traits>

namespace dmitigr::pgfe::internal {

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

} // namespace dmitigr::pgfe::internal

#endif  // DMITIGR_PGFE_INTERNAL_STD_STRING_TO_STRING_NUMERIC_HXX
