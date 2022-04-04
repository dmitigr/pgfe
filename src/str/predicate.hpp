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

#include "version.hpp"

#include <algorithm>
#include <locale>
#include <string>
#include <string_view>

namespace dmitigr::str {

// -----------------------------------------------------------------------------
// Predicates
// -----------------------------------------------------------------------------

/// @returns `true` if `c` is a valid space character.
inline bool is_space_character(const char c,
  const std::locale& loc = {}) noexcept
{
  return std::isspace(c, loc);
}

/// @returns `(is_space_character(c) == false)`.
inline bool is_non_space_character(const char c,
  const std::locale& loc = {}) noexcept
{
  return !is_space_character(c, loc);
}

/// @returns `true` if `c` is a valid simple identifier character.
inline bool is_simple_identifier_character(const char c,
  const std::locale& loc = {}) noexcept
{
  return std::isalnum(c, loc) || c == '_';
}

/// @returns `(is_simple_identifier_character(c) == false)`.
inline bool is_non_simple_identifier_character(const char c,
  const std::locale& loc = {}) noexcept
{
  return !is_simple_identifier_character(c, loc);
}

/// @returns `true` if `str` has at least one space character.
inline bool has_space(const std::string& str,
  const std::locale& loc = {}) noexcept
{
  return std::any_of(cbegin(str), cend(str), [&loc](const auto ch)
  {
    return is_space_character(ch, loc);
  });
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
