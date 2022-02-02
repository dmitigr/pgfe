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

#ifndef DMITIGR_STR_TRANSFORM_HPP
#define DMITIGR_STR_TRANSFORM_HPP

#include "version.hpp"
#include "../base/assert.hpp"

#include <algorithm>
#include <locale>
#include <string>
#include <string_view>
#include <vector>

namespace dmitigr::str {

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
    DMITIGR_ASSERT(offset <= pos);
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

#endif  // DMITIGR_STR_TRANSFORM_HPP
