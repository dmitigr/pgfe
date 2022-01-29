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

#ifndef DMITIGR_STR_NUMERIC_HPP
#define DMITIGR_STR_NUMERIC_HPP

#include "exceptions.hpp"
#include "version.hpp"

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
  static_assert(std::numeric_limits<Number>::min() <= 2 && std::numeric_limits<Number>::max() >= 36);

  if (!(2 <= base && base <= 36))
    throw Exception{"cannot convert number to text by using invalid base"};

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

} // namespace dmitigr::str

#endif  // DMITIGR_STR_NUMERIC_HPP
