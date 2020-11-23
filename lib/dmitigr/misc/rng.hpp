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

#ifndef DMITIGR_MISC_RNG_HPP
#define DMITIGR_MISC_RNG_HPP

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <string>

namespace dmitigr::rng {

// -----------------------------------------------------------------------------
// Number generators
// -----------------------------------------------------------------------------

/// Seeds the pseudo-random number generator.
inline void seed_by_now() noexcept
{
  const auto seed = std::chrono::duration_cast<std::chrono::seconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
  std::srand(static_cast<unsigned>(seed));
}

/**
 * @returns The random number.
 *
 * @remarks From TC++PL 3rd, 22.7.
 */
template<typename T>
constexpr T cpp_pl_3rd(const T maximum) noexcept
{
  const auto rand_num = static_cast<double>(std::rand());
  return static_cast<T>(maximum * (rand_num / RAND_MAX));
}

/// @overload
template<typename T>
constexpr T cpp_pl_3rd(const T minimum, const T maximum) noexcept
{
  assert(minimum < maximum);
  const auto range_length = maximum - minimum;
  return (cpp_pl_3rd(maximum) % range_length) + minimum;
}

// -----------------------------------------------------------------------------
// Strings generators
// -----------------------------------------------------------------------------

/**
 * @returns The random string.
 *
 * @param size The result size.
 * @param palette The palette of characters the result will consist of.
 */
inline std::string random_string(const std::string& palette,
  const std::string::size_type size)
{
  std::string result;
  result.resize(size);
  if (const auto pallete_size = palette.size()) {
    for (auto i = 0*size; i < size; ++i)
      result[i] = palette[rng::cpp_pl_3rd(pallete_size - 1)];
  }
  return result;
}

/**
 * @returns The random string.
 *
 * @param beg The start of source range.
 * @param end The past of end of source range.
 * @param size The result size.
 *
 * @par Requires
 * `(beg <= end)`.
 */
inline std::string random_string(const char beg, const char end,
  const std::string::size_type size)
{
  assert(beg <= end);
  std::string result;
  if (beg < end) {
    result.resize(size);
    const auto len = end - beg;
    for (auto i = 0*size; i < size; ++i)
      result[i] = static_cast<char>((rng::cpp_pl_3rd(end) % len) + beg);
  }
  return result;
}

} // namespace dmitigr::rng

#endif  // DMITIGR_MISC_RNG_HPP
