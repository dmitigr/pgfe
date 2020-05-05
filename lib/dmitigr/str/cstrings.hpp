// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or str.hpp

#ifndef DMITIGR_STR_CSTRINGS_HPP
#define DMITIGR_STR_CSTRINGS_HPP

#include <initializer_list>
#include <locale>

namespace dmitigr::str {

/**
 * @returns The pointer to a next non-space character, or pointer to the
 * terminating zero character.
 */
inline const char* next_non_space_pointer(const char* p, const std::locale& loc = {}) noexcept
{
  if (p) {
    while (*p != '\0' && std::isspace(*p, loc))
      ++p;
  }
  return p;
}

/**
 * @returns The specified literal if `(literal != nullptr)`, or "" otherwise.
 */
inline const char* literal(const char* const literal) noexcept
{
  return literal ? literal : "";
}

/**
 * @returns The first non-null string literal of specified literals, or
 * `nullptr` if all of the `literals` are nulls.
 */
inline const char* coalesce(std::initializer_list<const char*> literals) noexcept
{
  for (const auto l : literals) {
    if (l)
      return l;
  }
  return nullptr;
}

} // namespace dmitigr::str

#endif  // DMITIGR_STR_CSTRINGS_HPP
