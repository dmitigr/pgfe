// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or str.hpp

#ifndef DMITIGR_STR_TRANSFORMATORS_HPP
#define DMITIGR_STR_TRANSFORMATORS_HPP

#include <algorithm>
#include <locale>
#include <string>
#include <string_view>

namespace dmitigr::str {

/**
 * @returns The string with the specified `delimiter` between the characters.
 */
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
inline void terminate_string(std::string& str, const char c)
{
  if (str.empty() || str.back() != c)
    str += c;
}

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
inline std::string to_lowercase(const std::string_view str, const std::locale& loc = {})
{
  std::string result{str};
  lowercase(result, loc);
  return result;
}

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
inline std::string to_uppercase(const std::string_view str, const std::locale& loc = {})
{
  std::string result{str};
  uppercase(result, loc);
  return result;
}

/**
 * @returns `true` if all of characters of `str` are in uppercase, or
 * `false` otherwise.
 */
inline bool is_lowercased(const std::string_view str, const std::locale& loc = {})
{
  return std::all_of(cbegin(str), cend(str), [&loc](const char c) { return std::islower(c, loc); });
}

/**
 * @returns `true` if all of character of `str` are in lowercase, or
 * `false` otherwise.
 */
inline bool is_uppercased(const std::string_view str, const std::locale& loc = {})
{
  return std::all_of(cbegin(str), cend(str), [&loc](const char c) { return std::isupper(c, loc); });
}

} // namespace dmitigr::str

#endif  // DMITIGR_STR_TRANSFORMATORS_HPP
