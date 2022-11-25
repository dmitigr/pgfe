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

#ifndef DMITIGR_STR_NUMERIC_HPP
#define DMITIGR_STR_NUMERIC_HPP

#include "exceptions.hpp"

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
  static_assert(std::numeric_limits<Number>::min() <= 2 &&
    std::numeric_limits<Number>::max() >= 36);

  if (!(2 <= base && base <= 36))
    throw Exception{"cannot convert number to text by using invalid base"};

  constexpr const char digits[] =
    {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
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
