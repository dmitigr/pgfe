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

#ifndef DMITIGR_STR_TRANSFORM_HPP
#define DMITIGR_STR_TRANSFORM_HPP

#include "../base/assert.hpp"
#include "basics.hpp"
#include "predicate.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>

namespace dmitigr::str {

// -----------------------------------------------------------------------------
// Transformators
// -----------------------------------------------------------------------------

/// @returns The string with the specified `delimiter` between the characters.
inline std::string
sparsed_string(const std::string_view input, const std::string_view delimiter)
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
 * `str.back() == c`.
 */
inline void terminate(std::string& str, const char c)
{
  if (str.empty() || str.back() != c)
    str += c;
}

/// Eliminates duplicate characters from string `str`.
inline void eliminate_duplicates(std::string& str)
{
  auto new_size = str.size();
  for (decltype(new_size) i{}; i < new_size; ++i) {
    const char ch = str[i];
    const auto b = begin(str) + i + 1;
    const auto e = begin(str) + new_size;
    const bool is_non_unique = find(b, e, ch) != e;
    if (is_non_unique) {
      remove_if(b, e, [ch, &new_size](const char c)
      {
        if (c == ch) {
          --new_size;
          return true;
        } else
          return false;
      });
    }
  }
  str.resize(new_size);
}

/// Trims `str` by dropping whitespaces at both sides of it.
inline std::string trimmed(std::string str, const Trim trim = Trim::all)
{
  if (str.empty())
    return str;

  const auto b = begin(str);
  const auto e = end(str);
  const auto tb = static_cast<bool>(trim & Trim::lhs) ?
    find_if(b, e, is_non_space<char>) : b;
  if (tb == e) {
    str.clear(); // the string consists of spaces, so just clear it out
    return str;
  }
  const auto te = static_cast<bool>(trim & Trim::rhs) ?
    find_if(rbegin(str), rend(str), is_non_space<char>).base() : e;

  const std::string::size_type new_size = te - tb;
  if (new_size != str.size()) {
    if (tb != b)
      move(tb, te, b);
    str.resize(new_size);
  }

  return str;
}

// -----------------------------------------------------------------------------
// lowercase

/**
 * @brief Replaces all of uppercase characters in `str` by the corresponding
 * lowercase characters.
 */
inline void lowercase(std::string& str)
{
  auto b = begin(str);
  auto e = end(str);
  transform(b, e, b, [](const unsigned char c){return tolower(c);});
}

/**
 * @returns The modified copy of the `str` with all of uppercase characters
 * replaced by the corresponding lowercase characters.
 */
inline std::string to_lowercase(std::string result)
{
  lowercase(result);
  return result;
}

/// @returns `true` if all of characters of `str` are in uppercase.
inline bool is_lowercased(const std::string_view str) noexcept
{
  return std::all_of(cbegin(str), cend(str), [](const unsigned char c)
  {
    return islower(c);
  });
}

// -----------------------------------------------------------------------------
// uppercase

/**
 * @brief Replaces all of lowercase characters in `str` by the corresponding
 * uppercase characters.
 */
inline void uppercase(std::string& str)
{
  auto b = begin(str);
  auto e = end(str);
  transform(b, e, b, [](const unsigned char c){return toupper(c);});
}

/**
 * @returns The modified copy of the `str` with all of lowercase characters
 * replaced by the corresponding uppercase characters.
 */
inline std::string to_uppercase(std::string result)
{
  uppercase(result);
  return result;
}

/// @returns `true` if all of character of `str` are in lowercase.
inline bool is_uppercased(const std::string_view str) noexcept
{
  return std::all_of(cbegin(str), cend(str), [](const unsigned char c)
  {
    return isupper(c);
  });
}

} // namespace dmitigr::str

#endif  // DMITIGR_STR_TRANSFORM_HPP
