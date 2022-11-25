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

#ifndef DMITIGR_STR_PREDICATE_HPP
#define DMITIGR_STR_PREDICATE_HPP

#include <algorithm>
#include <cctype>
#include <string_view>

namespace dmitigr::str {

// -----------------------------------------------------------------------------
// Predicates
// -----------------------------------------------------------------------------

/// @returns `true` if `c` is a valid space character.
template<typename Ch>
bool is_space(const Ch ch) noexcept
{
  return std::isspace(static_cast<unsigned char>(ch));
}

/// @returns `!is_space(ch)`.
template<typename Ch>
bool is_non_space(const Ch ch) noexcept
{
  return !is_space(ch);
}

/// @returns `true` if `c` is printable character.
template<typename Ch>
bool is_printable(const Ch ch) noexcept
{
  return std::isprint(static_cast<unsigned char>(ch));
}

/// @returns `true` if `c` is a zero character.
template<typename Ch>
bool is_zero(const Ch ch) noexcept
{
  return ch == '\0';
}

/// @returns `true` if `c` is a non zero character.
template<typename Ch>
bool is_non_zero(const Ch ch) noexcept
{
  return !is_zero(ch);
}

/// @returns `true` if `str` is a blank string.
inline bool is_blank(const std::string_view str) noexcept
{
  return std::all_of(cbegin(str), cend(str), is_space<char>);
}

/// @returns `true` if `str` has at least one space character.
inline bool has_space(const std::string_view str) noexcept
{
  return std::any_of(cbegin(str), cend(str), is_space<char>);
}

/// @returns `true` if `input` is starting with `pattern`.
inline bool is_begins_with(const std::string_view input,
  const std::string_view pattern) noexcept
{
  return (pattern.size() <= input.size()) &&
    std::equal(cbegin(pattern), cend(pattern), cbegin(input));
}

} // namespace dmitigr::str

#endif  // DMITIGR_STR_PREDICATE_HPP
