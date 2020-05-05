// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or str.hpp

#ifndef DMITIGR_STR_PREDICATES_HPP
#define DMITIGR_STR_PREDICATES_HPP

#include <algorithm>
#include <functional>
#include <locale>

namespace dmitigr::str {

/**
 * @returns `true` if `c` is a valid space character.
 */
inline bool is_space_character(const char c, const std::locale& loc = {})
{
  return std::isspace(c, loc);
}

/**
 * @returns `(is_space_character(c) == false)`.
 */
inline bool is_non_space_character(const char c, const std::locale& loc = {})
{
  return !is_space_character(c, loc);
}

/**
 * @returns `true` if `c` is a valid simple identifier character.
 */
inline bool is_simple_identifier_character(const char c, const std::locale& loc = {})
{
  return std::isalnum(c, loc) || c == '_';
}

/**
 * @returns `(is_simple_identifier_character(c) == false)`.
 */
inline bool is_non_simple_identifier_character(const char c, const std::locale& loc = {})
{
  return !is_simple_identifier_character(c, loc);
}

/**
 * @returns `true` if `str` has at least one space character.
 */
inline bool has_space(const std::string& str, const std::locale& loc = {})
{
  return std::any_of(cbegin(str), cend(str), std::bind(is_space_character, std::placeholders::_1, loc));
}

/**
 * @returns `true` if `input` is starting with `pattern`.
 */
inline bool is_begins_with(const std::string_view input, const std::string_view pattern)
{
  return (pattern.size() <= input.size()) && std::equal(cbegin(pattern), cend(pattern), cbegin(input));
}

} // namespace dmitigr::str

#endif  // DMITIGR_STR_PREDICATES_HPP
