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
